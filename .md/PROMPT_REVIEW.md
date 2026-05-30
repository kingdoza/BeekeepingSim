# 리뷰 프롬프트: Pollen Patty Fixed Consumption

## 리뷰 목적

이번 리뷰는 `ABeehive`의 화분떡 소모가 **bucket 기반 고정량 소비 정책**대로 구현되었는지 검증한다.

핵심 목표:
- `PollenPattyConsumption` bucket 구독/이벤트 분기가 정확한지
- 소모 대상 탐색이 direct child `AItemPlacementSlotActor` + slot configured tags 기준인지
- 선택 규칙(`Leftmost/Rightmost`, local Y, tie 유지)과 단일 대상 소비 정책이 지켜지는지
- 소모 실행이 `UPlacedItemRemainingComponent::ConsumeAmount(...)` 단일 경로인지

중요: 워크트리에 다른 변경이 있을 수 있으므로 **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md` (`화분떡 고정 소모 로직 설계 QnA` 22~26)
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

---

## 명시적 제외 범위 확인

아래가 코드에 추가되지 않았는지 확인:

- colony population/산란력/수명/온도/벌 수 연동 소모 공식
- 화분떡 효과가 colony/honey 생산에 직접 보정으로 들어가는 로직
- 여러 화분떡 분배 소모 또는 같은 bucket spillover
- `ABeehive` 런타임 탐색 코드에 `Item.UseArea.Beehive.PollenPatty` 문자열 하드코딩
- `Content/` asset 수정 의존 구현

---

## 핵심 검증 질문

1. `AItemPlacementSlotActor::GetSlotAreaTags()`가 추가되었고 source of truth가 `SlotMeshComponent->GetAreaTags()`인가?
2. `ABeehive`에 `EPollenPattyConsumptionSide`, 관련 UPROPERTY, `ApplyPollenPattyConsumptionUpdate()`가 추가되었는가?
3. `GetGameTimeBucketSubscriptions_Implementation`에 `SubscriptionTag="PollenPattyConsumption"`이 추가되었는가?
4. `OnGameTimeBucketEvent_Implementation`에 `PollenPattyConsumption` 분기가 추가되었는가?
5. 소모량이 `PollenPattyConsumptionAmountPerBucket` 고정값만 사용되고, 시간 길이/이벤트 횟수로 스케일되지 않는가?
6. 대상 탐색이 direct child `AItemPlacementSlotActor` 수집 경로를 사용하고 descriptor `AreaTags`를 사용하지 않는가?
7. 태그 매칭이 `PollenPattyConsumptionAreaTags.IsEmpty() -> false`, `SlotTags.HasAll(...)` 의미로 구현되었는가?
8. 후보 필터가 `occupied actor + active remaining + current amount > 0`을 만족하는가?
9. 선택 규칙이 `Leftmost=min local Y`, `Rightmost=max local Y`, tie는 선행 후보 유지(`<`/`>`만 사용)인가?
10. 실제 소모가 선택된 1개 target에만 `ConsumeAmount(...)` 호출되는가?

---

## 리뷰 범위 (우선 파일)

### 구현 코드
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

### 문서
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

---

## 상세 체크리스트

### 1) Slot Tag API
- `GetSlotAreaTags()` public API 존재
- `SlotMeshComponent` null일 때 deprecated `AreaTags` fallback 처리

### 2) Beehive API/설정
- `PollenPattyConsumptionBucketMinutes` 기본 `60`
- `bApplyPollenPattyConsumptionOnBeginPlayBucket` 기본 `false`
- `PollenPattyConsumptionAmountPerBucket` 기본 `1.0f`
- `PollenPattyConsumptionSide` 기본 `Leftmost`
- `PollenPattyConsumptionAreaTags` UPROPERTY 존재

### 3) Bucket Wiring
- 구독 추가:
  - `BucketMinutes = Clamp(..., 1, 1440)`
  - `CatchUpPolicy = LatestOnly`
  - `SubscriptionTag = "PollenPattyConsumption"`
- 이벤트 분기에서 `ApplyPollenPattyConsumptionUpdate()` 호출

### 4) 후보 수집/선택
- `TInlineComponentArray<UChildActorComponent*> ChildActorComponents(this)` 기반 순회
- slot actor cast, tag match, occupied actor, remaining 상태 검증
- local Y 계산이 `GetActorTransform().InverseTransformPosition(SlotActor->GetActorLocation())` 경로인지 확인
- side별 strict 비교로 tie 유지되는지 확인

### 5) 소비 실행
- `Amount <= 0` early return
- target/remaining null guard
- `ConsumeAmount(Amount)` 호출 외 별도 clear/destroy 미수행

---

## 코드 검색 기준

### 있어야 함
- `PollenPattyConsumption`
- `ApplyPollenPattyConsumptionUpdate(`
- `GetSlotAreaTags(`
- `ConsumeAmount(`

### 없어야 함 (WorldActors 소스 런타임 로직)
- `Item.UseArea.Beehive.PollenPatty`

권장 검색:

```powershell
rg "PollenPattyConsumption" Source/BeekeepingSim .md
rg "Item.UseArea.Beehive.PollenPatty" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "GetSlotAreaTags|GetOccupiedActor|ConsumeAmount" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

---

## 시나리오 검증 포인트

1. `PollenPattyConsumptionAreaTags`가 비어 있으면 소모가 발생하지 않는다.
2. 태그 매칭 slot이 1개면 bucket마다 고정량(기본 1.0)만 감소한다.
3. 후보가 여러 개면 기본 `Leftmost`에서 local Y 최소 slot 1개만 감소한다.
4. `Rightmost`로 바꾸면 local Y 최대 slot 1개만 감소한다.
5. 선택 target이 소진돼도 같은 bucket에서 다른 화분떡으로 spillover되지 않는다.
6. remaining inactive 또는 current amount <= 0 actor는 후보에서 제외된다.

---

## 빌드/검증

- 가능하면 UBT 빌드 결과를 확인:
  - `BeekeepingSimEditor Win64 Development`
- 빌드 불가 시 원인을 분리해 보고한다. (예: 에디터 락/환경 이슈)

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순서로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- Findings 이후:
  - 가정/불확실성
  - 테스트 공백
  - 문서 동기화 누락 여부
