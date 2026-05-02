# 구현 프롬프트: Game Time Bucket Event System / Time Clock Widget 리뷰 반영

## 배경

`UGameTimeBucketSubsystem : UWorldSubsystem` 기반 bucket 이벤트 시스템과 `ABeehive` listener 연동은 핵심 요구사항 대부분을 충족한다. 이전 리뷰에서 지적된 `ABeehive` self register/unregister, catch-up `bWrappedDay`, bucket minute epsilon 제거도 반영되어 있다.

다만 현재 제출 단위는 editor target 빌드가 실패하며, Time Clock Widget의 `HH:MM` minute 변환이 요구한 strict floor 규칙과 다르다.

## 발견 문제 목록

1. `LogBeekeepingBeeSwarm` 로그 카테고리가 public header 2곳에서 중복 선언되어 `BeekeepingSimEditor Win64 Development` 빌드가 실패한다.
   - `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
   - `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
   - `Source/BeekeepingSim/Private/WorldActors/BeeSplineSwarmActor.cpp`에서 `DEFINE_LOG_CATEGORY(LogBeekeepingBeeSwarm)`를 수행한다.
   - 한 translation unit에서 두 header가 함께 include되면 `FLogCategoryLogBeekeepingBeeSwarm` 재정의가 발생한다.
2. `UTimeOfDayClockWidget::Hour24ToTotalMinutes()`가 `FloorToInt(NormalizedHour * 60.0f + 0.001f)`를 사용한다.
   - Review checklist는 floor minute conversion을 요구한다.
   - 현재 구현은 minute boundary 약 0.06초 전부터 다음 분으로 표시될 수 있다.
3. `UGameTimeBucketSubsystem::EnsureTimeActorBound()`가 `TimeOfDayActor` 포인터 truthiness만 검사한다.
   - 일반 흐름에서는 문제 가능성이 낮지만, time actor destroy/replacement edge case에서는 `IsValid(TimeOfDayActor)` 검사가 더 안전하다.

## 우선순위

- High: 로그 카테고리 중복 선언 정리 및 editor/game target 빌드 복구
- Low: Time Clock Widget minute conversion strict floor로 수정
- Low: `EnsureTimeActorBound()` stale pointer edge case 보강

## 수정 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- 필요 시 신규 공용 로그 header 또는 기존 shared header
- `Source/BeekeepingSim/Private/UI/TimeOfDayClockWidget.cpp`
- `Source/BeekeepingSim/Private/Environment/GameTimeBucketSubsystem.cpp`

## 수정 방향

### 1. Log category 선언 단일화

- `DECLARE_LOG_CATEGORY_EXTERN(LogBeekeepingBeeSwarm, Log, All);`는 하나의 공용 header에만 둔다.
- 두 actor header가 같은 log category를 써야 한다면 `BeeSwarmTypes.h` 같은 shared header로 옮기고 양쪽에서 그 header를 include한다.
- `DEFINE_LOG_CATEGORY(LogBeekeepingBeeSwarm);`는 하나의 `.cpp`에만 유지한다.
- 또는 dual/spline actor별 로그 카테고리를 분리한다. 같은 이름을 두 header에 각각 선언하지 않는다.

### 2. Time Clock strict floor

- `UTimeOfDayClockWidget::Hour24ToTotalMinutes()`를 다음 규칙으로 맞춘다.

```cpp
const float NormalizedHour = NormalizeHour24(Hour24);
return FMath::FloorToInt(NormalizedHour * 60.0f) % 1440;
```

- `FormatHour24AsHHMM(24.0f)`가 `00:00`을 반환하는지 유지한다.
- `SetHour24()`가 displayed minute 변경 시에만 `OnDisplayedTimeChanged`를 호출하는 기존 최적화는 유지한다.

### 3. Time actor validity 보강

- `EnsureTimeActorBound()`의 early return은 `IsValid(TimeOfDayActor)` 기준으로 바꾼다.
- `SetTimeOfDayActor()` / `RegisterListener()` / `UnbindTimeActor()`도 필요하면 `IsValid(TimeOfDayActor)` 기준으로 정리한다.

## 검증 방법

```powershell
rg "DECLARE_LOG_CATEGORY_EXTERN\(LogBeekeepingBeeSwarm|DEFINE_LOG_CATEGORY\(LogBeekeepingBeeSwarm" Source/BeekeepingSim
rg "EnvironmentTimeOfDayActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "PrimaryActorTick.bCanEverTick = true" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "GameTimeBucketSubsystem" Source/BeekeepingSim/Public/UI Source/BeekeepingSim/Private/UI Source/BeekeepingSim/Public/Character Source/BeekeepingSim/Private/Character
```

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe' BeekeepingSimEditor Win64 Development -Project='C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject' -WaitMutex -NoHotReloadFromIDE
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe' BeekeepingSim Win64 Development -Project='C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject' -WaitMutex -NoHotReloadFromIDE
```

수동/간단 검증:

- `UTimeOfDayClockWidget::FormatHour24AsHHMM(24.0f)` -> `00:00`
- `UTimeOfDayClockWidget::FormatHour24AsHHMM(1.9999f)`가 strict floor 기준으로 표시되는지 확인
- `ABeehive`가 BeginPlay/EndPlay에서 bucket subsystem register/unregister를 유지하는지 확인
- `ABeehive` tick이 계속 비활성인지 확인

## 아키텍처 문서 반영 필요 여부

- 로그 카테고리 정리와 minute conversion 보정은 내부 구현 수정이므로 문서 반영 불필요.
- shared log header를 새로 추가하는 경우에도 시스템 책임 변화가 아니면 문서 갱신은 불필요.
