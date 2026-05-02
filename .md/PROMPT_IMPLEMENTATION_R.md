# 구현 프롬프트: Game Time Bucket Event System 리뷰 반영

## 배경

`UGameTimeBucketSubsystem : UWorldSubsystem` 기반 시간 bucket 이벤트 시스템과 `ABeehive` listener 연동 리뷰 결과, 기본 구조/빌드/Environment 경계는 통과했다. 다만 런타임 spawn listener 등록 누락 가능성과 catch-up wrap payload 의미가 요구사항과 어긋날 수 있다.

## 발견 문제 목록

1. `ABeehive`가 `BeginPlay`/`EndPlay`에서 `UGameTimeBucketSubsystem`에 self register/unregister하지 않는다.
   - 현재 subsystem은 `OnWorldBeginPlay()`에서 `RefreshListeners()`로 기존 월드 actor를 1회 스캔한다.
   - `ABeehive::BeginPlay()`는 swarm child class 보정과 parameter 적용만 수행한다.
   - 따라서 world begin 이후 동적으로 spawn된 beehive는 외부에서 `RegisterListener()`를 수동 호출하지 않으면 bucket event를 받지 못한다.
2. `CatchUp` dispatch의 `bWrappedDay`가 자정 이후 catch-up event 전부에서 true가 될 수 있다.
   - 현재 `NextBucketStart / 1440 > LastAbsoluteBucketStartMinute / 1440` 비교를 매 catch-up event에 적용한다.
   - 기대 의미가 "이번 event가 24시 wrap 경계를 대표한다"라면 00:00 bucket 또는 실제 wrap transition 1회에만 true가 되어야 한다.
3. `HourToMinuteOfDay()`가 요구식 `FloorToInt(NormalizeHour24(Hour24) * 60)`에 `+0.001f` epsilon을 더한다.
   - 약 0.06초 이른 bucket 전환이 가능하다.
   - 의도된 float 안정화라면 문서화하고, 엄격한 경계 계산이 목표라면 epsilon을 제거한다.

## 우선순위

- Medium: 동적 spawn `ABeehive` listener 등록 보장
- Medium: `CatchUp` `bWrappedDay` payload 의미 확정 및 구현 보정
- Low: bucket minute 계산식의 epsilon 제거 또는 의도 문서화

## 수정 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/Environment/GameTimeBucketSubsystem.cpp`
- 필요 시 `.md/Architecture/EnvironmentSystem.md`, `.md/Architecture/WorldActorsSystem.md`

## 수정 방향

### 1. Beehive self registration

- `ABeehive::BeginPlay()`에서 `GetWorld()->GetSubsystem<UGameTimeBucketSubsystem>()`를 통해 자기 자신을 `RegisterListener(this)` 한다.
- `ABeehive::EndPlay()`에서 `UnregisterListener(this)`를 호출한다.
- 이 의존은 concrete `AEnvironmentTimeOfDayActor`가 아니라 subsystem/interface 경로이므로 WorldActors 경계 정책을 유지한다.
- 중복 등록은 subsystem의 `RegisterListener()`가 기존 listener entries를 제거한 뒤 재등록하므로 안전하다.

### 2. CatchUp wrap payload

- `bWrappedDay` 의미를 명확히 정한다.
  - 옵션 A: catch-up event 중 자정 bucket(`MinuteOfDay == 0`)에만 true.
  - 옵션 B: latest event처럼 time source broadcast가 wrap된 호출에서만 true.
- 현재처럼 자정 이후 같은 catch-up batch의 모든 event가 true가 되는 동작은 피한다.

### 3. Minute 계산식 정합성

- 요구사항을 엄격히 따르려면 `HourToMinuteOfDay()`를 `FMath::FloorToInt(NormalizeHour24(Hour24) * 60.0f)`로 변경한다.
- float 오차 방어가 필요하면 bucket boundary test와 문서에 epsilon 의도를 남긴다.

## 검증 방법

```powershell
rg "EnvironmentTimeOfDayActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "PrimaryActorTick.bCanEverTick = true" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "RegisterListener|UnregisterListener|RefreshListeners|bWrappedDay|HourToMinuteOfDay" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
```

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe' BeekeepingSimEditor Win64 Development -Project='C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject' -WaitMutex -NoHotReloadFromIDE
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe' BeekeepingSim Win64 Development -Project='C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject' -WaitMutex -NoHotReloadFromIDE
```

수동 검증:

- level start 전에 배치된 `ABeehive`가 begin immediate option에 따라 1회 적용되는지 확인한다.
- PIE 중 runtime spawn된 `ABeehive`가 별도 수동 등록 없이 bucket event를 받는지 확인한다.
- `LatestOnly` subscription은 큰 time jump 후 현재 bucket 1회만 받는지 확인한다.
- `CatchUp` subscription은 자정 crossing 시 `bWrappedDay`가 의도한 event에만 true인지 확인한다.
- `ABeehive` tick이 계속 비활성인지 확인한다.

## 아키텍처 문서 반영 필요 여부

- `ABeehive` self registration을 도입하면 `.md/Architecture/WorldActorsSystem.md`에 "Beehive가 subsystem에 직접 register/unregister한다"를 추가한다.
- `bWrappedDay` 의미를 확정하면 `.md/Architecture/EnvironmentSystem.md`에 payload 의미를 한 줄로 명시한다.

---

