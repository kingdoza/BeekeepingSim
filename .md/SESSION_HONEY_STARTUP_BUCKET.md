# Honey Startup Bucket Investigation

## 목적

PIE 시작 시 소비장에 꿀이 이미 차 있는 것처럼 보이는 문제를 다음 세션에서 바로 이어서 다루기 위한 인계 문서다.

## 현재 결론

문제의 직접 원인은 `bApplyHoneyProductionOnBeginPlayBucket`가 켜져 있어서 BeginPlay 즉시 꿀 생산이 실행되는 것이 아니다.

진단 로그 기준으로는 시작 시점의 소비장 꿀은 `0`이고, 게임 시작 직후 첫 일반 `HoneyProduction` bucket 이벤트가 발생하면서 꿀이 즉시 `100`까지 찬다.

즉 현상은 다음에 가깝다.

```text
초기 상태 누수 아님
-> 시작 직후 시간 bucket 경계 도달
-> HoneyProductionEvent Initial=0
-> TotalHoneyIncrease가 매우 큼
-> 첫 생산 tick 한 번에 소비장이 full
```

## 확인된 로그 요약

사용자 제공 로그의 핵심 값:

```text
BeginPlay_Start ApplyHoneyOnBeginPlay=0 HoneyBucketMinutes=30 HoneyProductionCoefficient=10.000

BeginPlay_AfterApplyInitialCombSetup
Slot 0 Honey=0.000
Slot 1 Honey=0.000

BeginPlay_AfterRegisterListener
Slot 0 Honey=0.000
Slot 1 Honey=0.000

HoneyProductionEvent Hour=10.000 BucketMinutes=30 BucketIndex=20 StartMinute=600 EndMinute=630 Initial=0 CatchUp=0 Wrapped=0

ApplyHoneyProductionUpdate TotalHoneyIncrease=1000.000 ColonyBeeCount=100 HoneyProductionCoefficient=10.000

ApplyHoneyProductionUpdate_AfterDistribute
Slot 0 Honey=100.000
Slot 1 Honey=100.000
```

해석:

- `ApplyHoneyOnBeginPlay=0`: 벌통의 begin-play 즉시 꿀 생산 옵션은 꺼져 있음.
- `Initial=0`: bucket subsystem의 initial apply가 아니라 일반 bucket 경계 이벤트임.
- `Honey=0` 상태가 `ApplyHoneyProductionUpdate_AfterDistribute`에서 처음 `100`이 됨.
- `TotalHoneyIncrease=1000`이고 active comb가 2개뿐이라 각 소비장이 `MaxHoneyPerComb=100`으로 clamp됨.

## 관련 코드 위치

- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
  - `ABeehive::BeginPlay`
  - `ABeehive::GetGameTimeBucketSubscriptions_Implementation`
  - `ABeehive::OnGameTimeBucketEvent_Implementation`
  - `ABeehive::ApplyHoneyProductionUpdate`
  - `ABeehive::DistributeHoneyIncreaseToCombs`
- `Source/BeekeepingSim/Private/Environment/GameTimeBucketSubsystem.cpp`
  - `UGameTimeBucketSubsystem::ProcessSubscription`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
  - `ABeehiveCombActor::BeginPlay`
  - `ABeehiveCombActor::AddHoneyAmount`
  - `ABeehiveCombActor::SanitizeHoneyState`

## 현재 진단 로그 상태

원인 확인을 위해 아래 Source 파일에 임시 로그가 들어가 있다.

- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

로그 prefix:

```text
[HoneyDiagnostic]
```

다음 세션에서 원인 분석이 끝났다면 이 로그는 제거 대상이다.

## 설계 선택지

### 옵션 A: 생산량 튜닝

`HoneyProductionCoefficient`를 낮춰 첫 bucket 한 번에 full이 되지 않게 한다.

- 장점: 코드 변경이 거의 없거나 없음.
- 단점: 시작 직후 첫 bucket 이벤트 자체는 그대로 발생한다.
- 현재 값 `10.0`은 `ColonyBeeCount=100` 기준 `TotalHoneyIncrease=1000`을 만든다.

### 옵션 B: 시작 직후 첫 일반 bucket 이벤트 스킵

벌통이 listener 등록 후 첫 `HoneyProduction` 일반 이벤트를 1회 무시한다.

- 장점: 시작 직후 바로 꿀이 차는 체감 문제를 직접 해결한다.
- 단점: `bApplyHoneyProductionOnBeginPlayBucket=false`의 의미와 별개로 "첫 일반 이벤트도 스킵"이라는 새 정책이 생긴다.
- 주의: `Initial=0` 이벤트를 무조건 첫 회 스킵하면, 실제 플레이 시작 시점이 bucket 경계 직전일 때 생산이 한 주기 늦어진다.

### 옵션 C: bucket subsystem에 grace/window 정책 추가

listener 등록 직후 일정 시간 또는 같은 frame/초기 minute 안의 bucket transition을 무시하는 공통 정책을 추가한다.

- 장점: 다른 gameplay bucket에서도 같은 문제가 생길 때 재사용 가능.
- 단점: Environment bucket 시스템 공통 정책이 복잡해진다.
- 주의: WorldActors가 Environment 정책을 과하게 제어하지 않도록 경계를 검토해야 한다.

### 옵션 D: 시작 시간/preview hour 조정

`AGameTimeOfDayActor`/`ADynamicSky` 시작 시간이 bucket 경계 근처에서 시작하지 않게 설정한다.

- 장점: 코드 변경 없이 완화 가능.
- 단점: 시간 설정에 의존하는 우회에 가깝다.
- 현재 로그는 `Hour=10.000`, `BucketMinutes=30`, `BucketIndex=20`에서 이벤트가 발생했다.

## 권장 방향

우선 옵션 A와 D로 의도한 밸런스인지 확인한다.

게임 디자인상 시작 후 첫 30분 경계에서 꿀 생산이 발생하는 것이 맞고, 문제는 "한 번에 full"인 것이라면 옵션 A가 맞다.

게임 디자인상 플레이 시작 직후에는 첫 생산을 늦춰야 한다면 옵션 B를 별도 QNA로 확정한 뒤 구현한다. 이 경우 `bApplyHoneyProductionOnBeginPlayBucket=false`와 구분되는 새 bool 이름이 필요하다. 예:

```cpp
bool bSkipFirstHoneyProductionBucketAfterBeginPlay = false;
```

## 다음 세션 작업 체크리스트

1. `[HoneyDiagnostic]` 로그를 한 번 더 확인해 같은 패턴인지 재확인한다.
2. `HoneyProductionCoefficient=10.0`이 임시 테스트 값인지 의도 값인지 결정한다.
3. 시작 직후 첫 일반 bucket 이벤트를 허용할지 결정한다.
4. 선택한 정책에 따라 구현한다.
5. 원인 확인용 `[HoneyDiagnostic]` 로그를 제거한다.
6. UBT 빌드를 수행한다.
7. 변경이 설계 정책이면 `.md/0_ARCHITECTURE.md`와 `.md/Architecture/WorldActorsSystem.md`를 갱신한다.

## 현재 작업 트리 주의

문서 작성 시점에 `Content/*.uasset` 변경이 존재했다. 이 세션의 진단 로그 작업은 Source만 수정했으며 Content asset은 수정하지 않았다.
