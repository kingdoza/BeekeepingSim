# 구현 프롬프트: HH:MM 게임 시간 표시 위젯

## 목표

현재 게임 시간 `Hour24`를 화면에 `HH:MM` 형식으로 표시하는 UI 위젯을 구현한다.

예:

```text
0.0    -> 00:00
6.5    -> 06:30
12.25  -> 12:15
23.99  -> 23:59
24.0   -> 00:00
```

이번 구현은 시간 표시 UI에 한정한다.

## 확정 정책

사용자가 모든 애매한 사항에 대해 권장안을 채택했다.

확정 사항:

- 시간 원천은 `AEnvironmentTimeOfDayActor`
- `UGameTimeBucketSubsystem`은 사용하지 않는다.
  - Bucket 시스템은 gameplay n분 경계 이벤트용이다.
  - 시계 UI는 실제 현재 시간을 표시해야 하므로 `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`를 직접 사용한다.
- `OnTimeOfDayChanged` 이벤트는 받을 수 있지만, TextBlock 갱신은 분이 바뀔 때만 수행한다.
- 표시 계산은 내림(`Floor`) 기준이다.
  - 실제 시간이 12:30이 되기 전에는 12:29로 표시한다.
- `24:00`은 `00:00`으로 표시한다.
- 위젯 생성 주체는 기존 UI 시스템이 있으면 그 시스템을 따르고, 없으면 `ABeekeeperController`가 담당한다.
- `AEnvironmentTimeOfDayActor` 참조는 위젯이 직접 월드 탐색하지 않는다.
  - `ABeekeeperController` 또는 UI 생성 주체가 찾아서 위젯에 주입한다.
- 표시 형식은 이번 범위에서 무조건 `HH:MM` 고정이다.
- 에디터 프리뷰 UI 갱신은 이번 범위에서 제외한다.
  - PIE/런타임 동작만 구현한다.
- 시간 진행이 정지되면 마지막 표시를 유지한다.
- `SetCurrentHour24()` 등 수동 시간 변경으로 `OnTimeOfDayChanged`가 broadcast되면 즉시 반영한다.
- 위젯 생성 직후 현재 시간으로 즉시 초기 표시한다.

## 구현 전 참고 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/Architecture/UISystem.md`가 있으면 읽는다.
- `.md/Architecture/FocusSystem.md`는 crosshair/UI 생성 정책 확인용으로 필요 시 참고한다.

## 작업 범위

생성 후보:

- `Source/BeekeepingSim/Public/UI/TimeOfDayClockWidget.h`
- `Source/BeekeepingSim/Private/UI/TimeOfDayClockWidget.cpp`

수정 후보:

- `Source/BeekeepingSim/Public/Character/BeekeeperController.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperController.cpp`
- `Source/BeekeepingSim/Public/Environment/EnvironmentTimeOfDayActor.h`
- `Source/BeekeepingSim/Private/Environment/EnvironmentTimeOfDayActor.cpp`

문서 갱신:

- `.md/Architecture/EnvironmentSystem.md`
- `.md/0_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

주의:

- `Content/` asset 자동 생성/수정은 하지 않는다.
- `WBP_TimeOfDayClock` 같은 Blueprint Widget asset 생성은 에디터 수동 작업으로 남긴다.

## `UTimeOfDayClockWidget` 설계

### Class

```cpp
UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UTimeOfDayClockWidget : public UUserWidget
```

### Public API

필수:

```cpp
UFUNCTION(BlueprintCallable, Category = "Time Of Day")
void SetHour24(float InHour24);

UFUNCTION(BlueprintPure, Category = "Time Of Day")
float GetCurrentHour24() const;

UFUNCTION(BlueprintPure, Category = "Time Of Day")
FText GetFormattedTimeText() const;
```

권장:

```cpp
UFUNCTION(BlueprintPure, Category = "Time Of Day")
static FText FormatHour24AsHHMM(float Hour24);
```

### Blueprint Event

TextBlock binding을 C++에서 강제하지 않고 Blueprint가 표시를 구성할 수 있도록 event를 제공한다.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Time Of Day")
void OnDisplayedTimeChanged(const FText& NewTimeText, int32 Hour, int32 Minute);
```

`SetHour24()`는 표시 분이 바뀐 경우에만 `OnDisplayedTimeChanged()`를 호출한다.

### Internal State

권장:

```cpp
UPROPERTY(Transient, BlueprintReadOnly, Category = "Time Of Day", meta = (AllowPrivateAccess = "true"))
float CurrentHour24 = 12.0f;

UPROPERTY(Transient, BlueprintReadOnly, Category = "Time Of Day", meta = (AllowPrivateAccess = "true"))
int32 LastDisplayedTotalMinutes = INDEX_NONE;
```

### Format 정책

시간 normalize:

```cpp
NormalizedHour = FMath::Fmod(Hour24, 24.0f);
if (NormalizedHour < 0.0f)
{
    NormalizedHour += 24.0f;
}
```

분 계산:

```cpp
TotalMinutes = FMath::FloorToInt(NormalizedHour * 60.0f + 0.001f) % 1440;
Hour = TotalMinutes / 60;
Minute = TotalMinutes % 60;
```

주의:

- 반올림하지 않는다.
- `+0.001f`는 float 오차 보정용이다.
- `24.0` 또는 `-0.0` 근처 값은 normalize 후 `00:00`으로 표시한다.

텍스트:

```cpp
FString::Printf(TEXT("%02d:%02d"), Hour, Minute)
```

## `ABeekeeperController` 연동 설계

기존 UI 생성 주체가 없다면 `ABeekeeperController`가 런타임에서 시간 위젯을 생성한다.

### Header 추가 후보

```cpp
class UTimeOfDayClockWidget;
class AEnvironmentTimeOfDayActor;
```

노출값:

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Time Of Day")
TSubclassOf<UTimeOfDayClockWidget> TimeOfDayClockWidgetClass;

UPROPERTY(Transient)
TObjectPtr<UTimeOfDayClockWidget> TimeOfDayClockWidget;

UPROPERTY(Transient)
TObjectPtr<AEnvironmentTimeOfDayActor> BoundTimeOfDayActor;
```

### BeginPlay

`ABeekeeperController`에 `BeginPlay()` override가 없다면 추가한다.

흐름:

```text
BeginPlay
1. local controller인지 확인
2. TimeOfDayClockWidgetClass가 있으면 CreateWidget
3. AddToViewport
4. AEnvironmentTimeOfDayActor를 월드에서 탐색
5. 찾으면 OnTimeOfDayChanged에 bind
6. GetCurrentHour24()로 위젯 초기 표시
```

local controller 체크:

```cpp
if (!IsLocalController())
{
    return;
}
```

### Environment 탐색

위젯이 직접 탐색하지 않고 controller가 담당한다.

권장:

```cpp
AEnvironmentTimeOfDayActor* FindTimeOfDayActor() const;
```

정책:

- 레벨에 1개 배치되어 있다는 전제
- 없으면 warning
- 여러 개면 첫 번째 사용 + warning

주의:

- `AEnvironmentTimeOfDayActor`를 UI 위젯이 include하지 않는다.
- include는 controller cpp에서 처리한다.

### Delegate Bind

`AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`는 dynamic multicast delegate다.

Controller에 handler를 둔다.

```cpp
UFUNCTION()
void HandleTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState);
```

handler:

```text
if TimeOfDayClockWidget:
    TimeOfDayClockWidget->SetHour24(Hour24)
```

`VisualState`는 이번 위젯에서는 사용하지 않는다.

### EndPlay

`EndPlay()`에서 delegate unbind한다.

```text
if BoundTimeOfDayActor:
    BoundTimeOfDayActor->OnTimeOfDayChanged.RemoveDynamic(...)
```

위젯 제거:

```text
if TimeOfDayClockWidget:
    TimeOfDayClockWidget->RemoveFromParent()
```

## `AEnvironmentTimeOfDayActor` 변경 여부

가능하면 변경하지 않는다.

현재 필요한 API:

- `GetCurrentHour24()`
- `OnTimeOfDayChanged`

이미 존재하면 그대로 사용한다.

변경이 필요한 경우에만 최소 수정한다.

## Blueprint/UMG 수동 작업

구현 후 에디터에서 직접 수행한다.

1. `WBP_TimeOfDayClock` 생성
2. Parent Class를 `UTimeOfDayClockWidget`으로 지정
3. 위젯에 `TextBlock_Time` 추가
4. `OnDisplayedTimeChanged(NewTimeText, Hour, Minute)` 구현
5. `TextBlock_Time.SetText(NewTimeText)` 연결
6. `BP_BeekeeperController` 또는 사용하는 PlayerController BP에서 `TimeOfDayClockWidgetClass = WBP_TimeOfDayClock` 지정
7. 레벨에 `AEnvironmentTimeOfDayActor` 또는 해당 BP가 1개 배치되어 있는지 확인

UI 배치 권장:

- 화면 우상단 또는 상단 중앙
- 텍스트만 간결하게 표시
- `HH:MM` 외 추가 설명 텍스트는 이번 범위에서 넣지 않는다.

## 금지 작업

- UI 시계가 `UGameTimeBucketSubsystem`을 통해 갱신되는 구조
- 위젯 내부에서 `AEnvironmentTimeOfDayActor`를 직접 탐색하는 구조
- 매 프레임 Tick으로 TextBlock을 갱신하는 구조
- `ABeehive` 또는 WorldActors 시간 연동 로직 수정
- `AEnvironmentTimeOfDayActor`의 시간 진행 정책 변경
- `Content/` asset 자동 생성/수정
- `HH:MM` 외 AM/PM, 초 표시, 날짜 표시 추가
- 에디터 프리뷰 UI까지 지원하려는 확장 작업

## 문서 반영

### `.md/Architecture/EnvironmentSystem.md`

- `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`는 gameplay bucket뿐 아니라 UI 시계 표시에도 사용될 수 있음을 명시
- 시계 UI는 `UGameTimeBucketSubsystem`을 거치지 않는다고 명시

### `.md/0_ARCHITECTURE.md`

- UI 시간 표시 위젯 추가 요약

### `.md/USER_UNREAL.md`

에디터 작업 추가:

- `WBP_TimeOfDayClock` 생성
- parent를 `UTimeOfDayClockWidget`으로 설정
- `OnDisplayedTimeChanged`에서 TextBlock 갱신
- PlayerController BP에 widget class 지정
- 레벨에 `AEnvironmentTimeOfDayActor` 1개 배치

### `.md/PROMPT_REVIEW.md`

리뷰 체크 추가:

- 위젯이 직접 world actor 탐색을 하지 않는지
- controller/UI 생성 주체가 시간 Actor를 찾아 주입하는지
- `HH:MM` 포맷이 floor 기준인지
- `24.0 -> 00:00`인지
- Text 갱신이 분 변경 시에만 일어나는지
- Bucket subsystem을 사용하지 않는지

## 검증 명령

```powershell
rg "TimeOfDayClock|FormatHour24AsHHMM|OnDisplayedTimeChanged|HandleTimeOfDayChanged" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg "GameTimeBucketSubsystem" Source/BeekeepingSim/Public/UI Source/BeekeepingSim/Private/UI Source/BeekeepingSim/Public/Character Source/BeekeepingSim/Private/Character
```

빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 검증

1. `AEnvironmentTimeOfDayActor.CurrentHour24 = 12.25`로 시작한다.
2. PIE 시작 시 위젯이 즉시 `12:15`를 표시하는지 확인한다.
3. 시간이 진행되어 `12:16`이 되는 순간 표시가 `12:16`으로 바뀌는지 확인한다.
4. `12:15.9` 상태에서는 `12:15`로 표시되는지 확인한다.
5. `SetCurrentHour24(23.99)` 호출 시 `23:59`로 갱신되는지 확인한다.
6. `SetCurrentHour24(24.0)` 또는 `SetCurrentHour24(0.0)` 호출 시 `00:00`으로 표시되는지 확인한다.
7. 시간 진행을 정지하면 마지막 표시가 유지되는지 확인한다.
8. 시간 정지 중 `SetCurrentHour24()`를 수동 호출하면 즉시 갱신되는지 확인한다.

## 완료 기준

- `UTimeOfDayClockWidget`이 추가된다.
- `HH:MM` 포맷 함수가 C++에서 제공된다.
- 표시 계산은 floor 기준이다.
- `24:00`은 `00:00`으로 표시된다.
- 위젯 생성 직후 현재 시간이 즉시 표시된다.
- `OnTimeOfDayChanged`를 받아도 Text 갱신은 분 변경 시에만 일어난다.
- 위젯은 `AEnvironmentTimeOfDayActor`를 직접 탐색하지 않는다.
- `ABeekeeperController` 또는 기존 UI 생성 주체가 시간 Actor를 찾아 위젯에 전달한다.
- `UGameTimeBucketSubsystem`은 시계 UI에 사용하지 않는다.
- 관련 문서가 실제 구현과 일치한다.
