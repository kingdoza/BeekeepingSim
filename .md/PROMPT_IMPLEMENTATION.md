# 구현 프롬프트: Game Time Bucket Event System

## 목표

게임 시간 기준 `n분 단위 경계`마다 여러 Actor가 각자 필요한 동작을 수행할 수 있는 공용 시간 Bucket 이벤트 시스템을 구현한다.

이번 구현의 1차 사용처는 `ABeehive`의 벌떼 `SpawnAmount` 갱신이다.

예:

```text
플레이 시작 시간: 12:15
BucketMinutes: 30
BeginPlay 즉시 호출: 12:15 1회
다음 bucket 이벤트: 12:30
이후: 13:00, 13:30 ...
```

중요:

- “마지막 호출 후 n분 간격”이 아니다.
- “게임 시계의 00:00 기준 n분 경계가 바뀔 때” 이벤트를 보낸다.

## 확정 QnA

`.md/QNA_ARCHITECTURE.md`의 게임 시간 Bucket 이벤트 시스템 답변을 따른다.

확정 사항:

- 중앙 시스템은 `UGameTimeBucketSubsystem : UWorldSubsystem`
- `AEnvironmentTimeOfDayActor` 연결은 수동 지정 우선, 비어 있으면 자동 탐색 fallback
- Listener는 Actor가 직접 interface 구현
- Blueprint listener 구현 지원
- `BucketMinutes`는 임의 정수 `1~1440` 허용
- Bucket 경계는 하루 `00:00` 기준으로 n분마다 자르고, 마지막 bucket은 짧아질 수 있음
- BeginPlay 즉시 호출은 listener subscription별 옵션
- 시간 점프 처리는 subscription별 `LatestOnly` / `CatchUp` 선택
- 한 Actor가 여러 Bucket subscription을 가질 수 있음
- 24시 wrap payload는 `bWrappedDay` 포함
- 시간 정지 중 Tick 기반 진행은 멈추지만, 수동 시간 변경 broadcast는 처리
- Listener 자동 발견은 BeginPlay 1회 + `RegisterListener` / `UnregisterListener` API
- `ABeehive`는 직접 listener interface를 구현

## 구현 전 참고 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`

## 권장 파일 구성

생성:

- `Source/BeekeepingSim/Public/Environment/GameTimeBucketTypes.h`
- `Source/BeekeepingSim/Public/Environment/GameTimeBucketListener.h`
- `Source/BeekeepingSim/Public/Environment/GameTimeBucketSubsystem.h`
- `Source/BeekeepingSim/Private/Environment/GameTimeBucketSubsystem.cpp`

수정:

- `Source/BeekeepingSim/Public/Environment/EnvironmentTimeOfDayActor.h`
- `Source/BeekeepingSim/Private/Environment/EnvironmentTimeOfDayActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

문서 갱신:

- `.md/Architecture/EnvironmentSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/0_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

주의:

- `WorldActors`에서 `EnvironmentTimeOfDayActor` concrete class를 include/탐색하지 않는다.
- `ABeehive`는 generic listener interface만 include한다.
- `UGameTimeBucketSubsystem`은 Environment 영역에 있으므로 `AEnvironmentTimeOfDayActor`를 알고 bind해도 된다.

## Type 설계

### `EGameTimeBucketCatchUpPolicy`

```cpp
UENUM(BlueprintType)
enum class EGameTimeBucketCatchUpPolicy : uint8
{
    LatestOnly,
    CatchUp
};
```

의미:

- `LatestOnly`: 시간이 점프하면 현재 bucket 이벤트만 받는다.
- `CatchUp`: 마지막 처리 이후 지나간 bucket 이벤트를 순서대로 받는다.

`ABeehive`는 `LatestOnly`를 사용한다.

### `FGameTimeBucketSubscription`

```cpp
USTRUCT(BlueprintType)
struct FGameTimeBucketSubscription
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket", meta = (ClampMin = "1", ClampMax = "1440"))
    int32 BucketMinutes = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket")
    bool bApplyImmediatelyOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket")
    EGameTimeBucketCatchUpPolicy CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket")
    FName SubscriptionTag = NAME_None;
};
```

정책:

- `BucketMinutes`는 `1~1440`으로 clamp한다.
- 같은 Actor가 여러 subscription을 반환할 수 있다.
- 같은 Actor 안에서 구독 목적을 구분하려면 `SubscriptionTag`를 사용한다.

### `FGameTimeBucketEvent`

```cpp
USTRUCT(BlueprintType)
struct FGameTimeBucketEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    float Hour24 = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    int32 BucketMinutes = 10;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    int32 BucketIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    int32 BucketStartMinute = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    int32 BucketEndMinute = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    bool bWrappedDay = false;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    bool bInitialApply = false;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    bool bCatchUp = false;

    UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
    FName SubscriptionTag = NAME_None;
};
```

`BucketStartMinute` / `BucketEndMinute`는 하루 안의 분 단위 값이다.

예:

```text
12:30 -> 750
13:00 -> 780
24:00 -> 1440
```

마지막 bucket은 짧아질 수 있으므로 `BucketEndMinute`는 `Min(BucketStartMinute + BucketMinutes, 1440)`로 계산한다.

## Listener Interface 설계

`UGameTimeBucketListener` / `IGameTimeBucketListener`를 만든다.

필수 함수:

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game Time Bucket")
void GetGameTimeBucketSubscriptions(TArray<FGameTimeBucketSubscription>& OutSubscriptions) const;

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game Time Bucket")
void OnGameTimeBucketEvent(const FGameTimeBucketEvent& Event);
```

중요:

- Actor가 직접 이 interface를 구현한다.
- Blueprint Actor도 구현 가능해야 한다.
- 한 Actor가 여러 subscription을 반환할 수 있어야 한다.
- 중앙 subsystem은 특정 Actor class를 특별 취급하지 않는다.

## Bucket 계산 정책

### Hour normalize

```cpp
NormalizeHour24(Hour) = Fmod(Hour, 24.0f), 음수면 +24.0f
```

### Total minute

```cpp
TotalMinutes = FloorToInt(NormalizeHour24(Hour24) * 60.0f + 0.001f)
TotalMinutes = Clamp/Modulo to 0~1439
```

`24:00`은 normalize 후 `0:00`으로 취급한다.

### Bucket index

```cpp
BucketIndex = TotalMinutes / BucketMinutes
```

### Bucket start/end

```cpp
BucketStartMinute = BucketIndex * BucketMinutes
BucketEndMinute = Min(BucketStartMinute + BucketMinutes, 1440)
```

### 임의 BucketMinutes 예시

`BucketMinutes = 25`:

```text
00:00, 00:25, 00:50, ...
23:20, 23:45, 00:00
```

마지막 bucket:

```text
23:45~00:00 = 15분
```

`BucketMinutes = 13`이면 마지막 bucket은 10분이다.

`BucketMinutes = 90`은 하루에 딱 나누어떨어진다.

## Subsystem 설계

### Class

```cpp
UCLASS()
class BEEKEEPINGSIM_API UGameTimeBucketSubsystem : public UWorldSubsystem
```

책임:

- `AEnvironmentTimeOfDayActor` 연결 및 delegate bind
- BeginPlay 시 listener 자동 탐색 1회
- listener 수동 등록/해제 API 제공
- subscription별 마지막 처리 bucket 상태 저장
- `LatestOnly` / `CatchUp` 정책에 따라 이벤트 발행

### Public API

권장:

```cpp
UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
void SetTimeOfDayActor(AEnvironmentTimeOfDayActor* InTimeOfDayActor);

UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
void RegisterListener(AActor* ListenerActor);

UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
void UnregisterListener(AActor* ListenerActor);

UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
void RefreshListeners();
```

`SetTimeOfDayActor`:

- 기존 time actor delegate bind가 있으면 unbind
- 새 actor에 bind
- 현재 시간으로 필요 상태 초기화

`RegisterListener`:

- Actor가 `IGameTimeBucketListener`를 구현하지 않으면 무시 또는 warning
- subscription 목록을 수집하고 내부 상태 생성
- 이미 등록된 Actor면 subscription을 refresh

`UnregisterListener`:

- Actor와 관련된 모든 subscription 상태 제거

`RefreshListeners`:

- 월드 Actor 중 `IGameTimeBucketListener` 구현 Actor를 1회 탐색
- 기존 수동 등록 listener는 유지하되 dead weak pointer는 정리

### Environment 연결 방식

질문 2 답변은 옵션 C다.

구현 정책:

1. `AEnvironmentTimeOfDayActor::BeginPlay()`에서 subsystem을 얻어 `SetTimeOfDayActor(this)`를 호출한다.
2. subsystem은 world begin play 또는 첫 사용 시 time actor가 없으면 `AEnvironmentTimeOfDayActor`를 자동 탐색한다.
3. 여러 `AEnvironmentTimeOfDayActor`가 발견되면 첫 번째를 사용하고 warning을 남긴다.

이 방식은 수동 지정 우선 + 자동 탐색 fallback을 만족한다.

### Time delegate 처리

`AEnvironmentTimeOfDayActor::OnTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)`에 bind한다.

`OnTimeOfDayChanged`는 현재 매 Tick broadcast될 수 있다.

Subsystem은 매 broadcast마다 listener에 바로 뿌리지 말고, subscription별 bucket index가 바뀐 경우에만 event를 발행한다.

## Subscription 상태 관리

Actor 하나가 여러 subscription을 가질 수 있으므로 상태 key는 Actor 단위가 아니라 subscription 단위여야 한다.

권장 내부 구조:

```cpp
struct FRegisteredGameTimeBucketSubscription
{
    TWeakObjectPtr<AActor> Listener;
    FGameTimeBucketSubscription Subscription;
    int32 LastBucketIndex = INDEX_NONE;
    int32 LastAbsoluteBucketStartMinute = INDEX_NONE;
    bool bHasApplied = false;
};
```

`LastAbsoluteBucketStartMinute`는 CatchUp 처리를 위해 필요하다.

현재 `AEnvironmentTimeOfDayActor`는 DayIndex를 제공하지 않으므로 subsystem이 내부에서 day wrap을 추적한다.

권장 내부 상태:

```cpp
int32 CurrentDayOffset = 0;
int32 LastObservedMinuteOfDay = INDEX_NONE;
```

매 시간 broadcast마다:

```text
MinuteOfDay = TotalMinutes(Hour24)
if LastObservedMinuteOfDay != INDEX_NONE && MinuteOfDay < LastObservedMinuteOfDay:
    bWrappedDay = true
    CurrentDayOffset++
else:
    bWrappedDay = false
AbsoluteMinute = CurrentDayOffset * 1440 + MinuteOfDay
```

주의:

- 수동으로 시간을 뒤로 크게 변경해도 `MinuteOfDay < LastObservedMinuteOfDay`로 wrap처럼 보일 수 있다.
- 질문 10 답변에 따라 수동 시간 변경도 처리한다.
- `CatchUp`은 가능한 범위에서 forward absolute minute 기준으로 처리한다.
- 과도한 catch-up 폭주 방지를 위해 `MaxCatchUpEventsPerSubscription` 같은 내부 상한을 둘 수 있다. 상한 초과 시 warning 후 latest event로 축약한다.

## Event 발행 정책

### BeginPlay immediate

listener subscription의 `bApplyImmediatelyOnBeginPlay`가 true이면 현재 `Hour24`로 즉시 event를 보낸다.

이 event:

```text
bInitialApply = true
bCatchUp = false
```

이후 subscription 상태의 `LastBucketIndex`, `LastAbsoluteBucketStartMinute`, `bHasApplied`를 현재 bucket 기준으로 저장한다.

### LatestOnly

현재 bucket이 마지막 처리 bucket과 다르면 현재 bucket event만 보낸다.

시간 점프 예:

```text
Last: 12:10
Now: 13:05
BucketMinutes: 10
LatestOnly event: 13:00 bucket 1회
```

### CatchUp

마지막 처리 bucket 이후 지나간 bucket들을 순서대로 보낸다.

시간 점프 예:

```text
Last: 12:10
Now: 13:05
BucketMinutes: 10
CatchUp events:
- 12:20
- 12:30
- 12:40
- 12:50
- 13:00
```

catch-up event:

```text
bCatchUp = true
bInitialApply = false
```

최종 event의 `Hour24`는 해당 bucket start minute 기준으로 계산한다.

단, `LatestOnly` event의 `Hour24`는 현재 broadcast의 실제 `Hour24`를 사용해도 된다.

## `ABeehive` 연동

`ABeehive`는 `IGameTimeBucketListener`를 직접 구현한다.

권장 exposed settings:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm Time", meta = (ClampMin = "1", ClampMax = "1440"))
int32 BeeSwarmBucketMinutes = 10;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm Time")
bool bApplyBeeSwarmOnBeginPlayBucket = true;
```

`GetGameTimeBucketSubscriptions`:

```text
OutSubscriptions.Add({
  BucketMinutes = BeeSwarmBucketMinutes,
  bApplyImmediatelyOnBeginPlay = bApplyBeeSwarmOnBeginPlayBucket,
  CatchUpPolicy = LatestOnly,
  SubscriptionTag = "BeeSwarm"
})
```

`OnGameTimeBucketEvent`:

```text
if Event.SubscriptionTag == "BeeSwarm":
    ApplyBeeSwarmHour24(Event.Hour24)
```

주의:

- `ABeehive`가 `AEnvironmentTimeOfDayActor`를 include/탐색하지 않는다.
- `ABeehive` 자체 Tick을 추가하지 않는다.
- `ApplyBeeSwarmHour24()`는 기존처럼 spawn amount 계산과 Niagara parameter 적용만 담당한다.

## Blueprint Listener 예시 정책

Blueprint Actor가 listener interface를 구현하면:

- `GetGameTimeBucketSubscriptions`에서 배열을 반환
- `OnGameTimeBucketEvent`에서 `SubscriptionTag`로 분기

예:

```text
CropField
- 10분 / Tag=Moisture / CatchUp
- 60분 / Tag=Growth / CatchUp
- 1440분 / Tag=DailyReset / LatestOnly
```

중앙 subsystem은 Actor class를 알 필요가 없다.

## 금지 작업

- 각 Beehive가 `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`에 직접 bind하는 구조
- `ABeehive`에서 `AEnvironmentTimeOfDayActor` include/탐색
- `ABeehive` Tick 추가
- Bucket을 “마지막 호출 후 n분 간격”으로 처리
- `BucketMinutes`가 60의 약수/배수가 아닐 때 임의로 보정
- `BucketMinutes=25`, `13`, `110` 같은 값을 거부하지 말 것
- 중앙 subsystem이 `ABeehive`를 특별 취급하는 코드

## 문서 반영

### `.md/Architecture/EnvironmentSystem.md`

- `UGameTimeBucketSubsystem` 추가
- `AEnvironmentTimeOfDayActor`가 시간 원천이며 subsystem에 연결됨
- `OnTimeOfDayChanged`는 매 Tick broadcast될 수 있지만 Bucket subsystem이 n분 경계로 throttle/dispatch함
- Bucket 계산은 하루 `00:00` 기준, 마지막 bucket 짧음 허용
- `LatestOnly` / `CatchUp` 정책 설명

### `.md/Architecture/WorldActorsSystem.md`

- `ABeehive`가 `IGameTimeBucketListener` 구현
- `BeeSwarmBucketMinutes=10`
- BeginPlay 즉시 적용 후 10분 경계마다 `ApplyBeeSwarmHour24`
- Beehive는 Environment concrete actor에 의존하지 않음

### `.md/0_ARCHITECTURE.md`

- Environment time bucket event 시스템 요약
- WorldActors 시간 반응은 listener interface 기반임을 명시

### `.md/USER_UNREAL.md`

에디터 작업:

- 레벨에 `AEnvironmentTimeOfDayActor` 또는 BP 1개 배치
- `CurrentHour24`, `DayLengthSeconds`, `bTimeProgressionEnabled` 설정
- `BP_Beehive` 배치
- `BP_Beehive.BeeSwarmBucketMinutes` 기본 10 확인
- `BP_Beehive`의 벌떼 curve/settings/spline 설정
- 각 Beehive를 시간 delegate에 직접 bind하지 말 것

### `.md/PROMPT_REVIEW.md`

리뷰 체크:

- subsystem이 WorldSubsystem인지
- listener interface가 BP 구현 가능한지
- subscription별 상태가 Actor 단위가 아니라 subscription 단위인지
- 00:00 기준 bucket 경계인지
- 임의 `BucketMinutes` 마지막 짧은 bucket이 처리되는지
- `LatestOnly` / `CatchUp` 동작이 분리되는지
- Beehive가 Environment concrete actor에 의존하지 않는지
- Beehive Tick이 추가되지 않았는지

## 검증 명령

```powershell
rg "GameTimeBucket|BucketMinutes|CatchUp|LatestOnly|SubscriptionTag" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg "EnvironmentTimeOfDayActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "PrimaryActorTick.bCanEverTick = true" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 검증

### Bucket 경계

1. 시작 시간을 `12:15`로 설정한다.
2. `BeeSwarmBucketMinutes=30`으로 설정한다.
3. PIE 시작 시 `ABeehive::ApplyBeeSwarmHour24(12:15)`가 1회 호출되는지 확인한다.
4. 다음 호출이 `12:30` bucket에서 발생하는지 확인한다.
5. 이후 `13:00`, `13:30`에 호출되는지 확인한다.

### 임의 BucketMinutes

1. `BucketMinutes=25` listener를 만든다.
2. 경계가 `00:00, 00:25, 00:50 ... 23:45, 00:00`으로 처리되는지 확인한다.
3. `23:45~00:00` 마지막 bucket이 15분으로 처리되는지 확인한다.

### 복수 구독

1. 하나의 BP Actor가 10분/60분 subscription을 반환하게 만든다.
2. 두 subscription이 서로 독립적으로 이벤트를 받는지 확인한다.
3. `SubscriptionTag`로 분기 가능한지 확인한다.

### 시간 점프

1. 마지막 처리 시간이 `12:10`인 상태에서 `13:05`로 수동 변경한다.
2. `LatestOnly` subscription은 `13:00` bucket 1회만 받는지 확인한다.
3. `CatchUp` subscription은 `12:20, 12:30, 12:40, 12:50, 13:00`을 순서대로 받는지 확인한다.

### Beehive

1. `ABeehive`가 직접 `AEnvironmentTimeOfDayActor`를 참조하지 않는지 확인한다.
2. `ABeehive`가 `BeeSwarmBucketMinutes=10` subscription을 반환하는지 확인한다.
3. BeginPlay 즉시 `ApplyBeeSwarmHour24`가 호출되는지 확인한다.
4. 이후 10분 경계마다만 SpawnAmount가 갱신되는지 확인한다.

## 완료 기준

- `UGameTimeBucketSubsystem`이 구현된다.
- `IGameTimeBucketListener`가 BP 구현 가능하다.
- Actor 하나가 여러 시간 Bucket subscription을 가질 수 있다.
- `BucketMinutes`는 `1~1440` 임의 정수를 허용한다.
- 하루 `00:00` 기준으로 bucket 경계를 나눈다.
- 마지막 bucket이 짧아지는 경우가 정상 처리된다.
- BeginPlay 즉시 호출은 subscription별로 제어된다.
- `LatestOnly` / `CatchUp`이 subscription별로 동작한다.
- `ABeehive`는 listener로 10분 bucket을 구독한다.
- Beehive 벌떼 SpawnAmount는 BeginPlay 즉시 한 번, 이후 10분 경계마다 갱신된다.
- Beehive는 Environment concrete actor에 의존하지 않는다.
- 관련 문서가 실제 구현과 일치한다.
