# 코드 리뷰 요청 프롬프트 (Game Time Bucket Event System)

이번 변경은 `UGameTimeBucketSubsystem : UWorldSubsystem` 기반의 공용 시간 bucket 이벤트 시스템 추가와 `ABeehive` listener 연동 구현이다.

## 리뷰 목표

1. subsystem이 00:00 기준 n분 bucket 경계를 올바르게 계산하는지
2. listener interface가 C++/Blueprint 양쪽에서 구현 가능한지
3. subscription별 상태 관리가 actor 단위가 아니라 subscription 단위인지
4. `LatestOnly` / `CatchUp` 정책이 분리 동작하는지
5. `ABeehive`가 Environment concrete actor 의존 없이 bucket listener로만 갱신되는지
6. Beehive tick 추가 없이 동작하는지

## 필수 검증 포인트

### A. Subsystem 구조
- `UGameTimeBucketSubsystem`가 `UWorldSubsystem`인지
- `SetTimeOfDayActor`, `RegisterListener`, `UnregisterListener`, `RefreshListeners` API가 있는지
- `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged` bind/unbind 안전성

### B. Bucket 계산
- `BucketMinutes` clamp `1~1440`
- `MinuteOfDay = FloorToInt(NormalizeHour24(Hour24) * 60)`
- `BucketIndex = MinuteOfDay / BucketMinutes`
- `BucketStartMinute`, `BucketEndMinute`(마지막 짧은 bucket 허용)
- 24시 wrap payload(`bWrappedDay`) 처리

### C. Dispatch 정책
- BeginPlay immediate 옵션별 처리
- `LatestOnly`: 현재 bucket 1회
- `CatchUp`: 누락 bucket 순차 발행
- catch-up 폭주 보호 상한 동작

### D. Beehive 연동
- `ABeehive`가 `IGameTimeBucketListener` 구현하는지
- 기본 subscription:
  - `BucketMinutes = BeeSwarmBucketMinutes(기본 10)`
  - `CatchUpPolicy = LatestOnly`
  - `SubscriptionTag = "BeeSwarm"`
- `OnGameTimeBucketEvent`에서 태그 분기 후 `ApplyBeeSwarmHour24(Event.Hour24)` 호출하는지
- `ABeehive`가 `EnvironmentTimeOfDayActor`를 include/탐색하지 않는지

## 권장 확인 명령

- `rg "GameTimeBucket|BucketMinutes|CatchUp|LatestOnly|SubscriptionTag" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md`
- `rg "EnvironmentTimeOfDayActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors`
- `rg "PrimaryActorTick.bCanEverTick = true" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors`

## 출력 형식

1. Findings (High -> Medium -> Low)
2. Open Questions / Assumptions
3. Regression Risk Checklist
4. 최종 판정: Pass / Conditional Pass / Fail
## Review Checklist: Time Clock Widget

- Widget does not search `AEnvironmentTimeOfDayActor` directly.
- UI owner/controller resolves environment actor and injects `Hour24`.
- `HH:MM` formatting uses floor minute conversion and `24.0 -> 00:00`.
- Text update happens only when displayed minute changes.
- Clock path does not use `UGameTimeBucketSubsystem`.
## Review Checklist: Beehive Attraction Swarm

- `ABeehive` directly owns `AttractionSwarmNiagara` (no extra child actor).
- Attraction swarm does not subscribe to time bucket or hour-based auto updates.
- `User.SpawnAmount` is applied with `SetVariableInt`.
- `SpawnAmount` uses `RoundToInt(ColonyBeeCount * SpawnAmountScale)` and `MaxSpawnAmount` clamp.
- `ColonyBeeCount` changes immediately propagate to attraction spawn amount.
- Niagara user-parameter editing UI is hidden for `AttractionSwarmNiagara` via editor customization.
- Existing outgoing/ingoing spline swarm behavior remains intact.
