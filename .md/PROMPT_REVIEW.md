# 리뷰 프롬프트: Pollen Patty Population Bonus

## 리뷰 목적

이번 리뷰는 벌통(`ABeehive`)의 화분떡 인구 가속효과가 정책대로 구현되었는지 검증한다.

핵심 목표:
- bonus가 colony population **증가 항(`ItemEggLayingBonus`)에만** 적용되는지
- bonus 대상 선택이 기존 화분떡 소모 대상 선택 정책(`PollenPattyConsumptionSide`, local Y, 단일 대상)과 동일한지
- 효과 수치 source가 `UPollenPattyItemDefinition::EggLayingMultiplier`인지
- 중첩/최고 tier 탐색/ratio 스케일 같은 금지 정책이 들어가지 않았는지

중요: 워크트리에 다른 변경이 있을 수 있으므로 **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md` (`화분떡 인구 가속효과 설계 QnA` 27~32)
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

---

## 명시적 제외 범위 확인

아래가 추가되지 않았는지 확인:

- `UItemDefinition` 본체에 인구 가속 float
- `ABeehive` 전역 multiplier 필드(`PollenPattyEggLayingMultiplier`류)
- `CalculateBeeDecreaseAmount`/`GetItemLifespanBonus` 의미 변경
- 여러 화분떡 중첩, 최고 tier 탐색, ratio 기반 bonus 스케일
- Content asset 직접 수정 의존 구현

---

## 핵심 검증 질문

1. `UPollenPattyItemDefinition : UItemDefinition`가 추가되고 `EggLayingMultiplier(ClampMin=1.0, default 1.2)`를 가지는가?
2. `ABeehive::GetItemEggLayingBonus()`가 selected active 화분떡 definition만 참조하는가?
3. bonus 대상 선택이 기존 `FindPollenPattyConsumptionTargetSlot(...)` 결과(단일 1개)를 재사용하는가?
4. occupied actor definition resolve 우선순위가 정책과 일치하는가?
   - `APlacedItemActor::GetItemDefinition()`
   - `UPlacementOccupantComponent::GetReturnItemDefinition()`
5. selected definition이 `UPollenPattyItemDefinition`이 아니면 `1.0f`를 반환하는가?
6. selected definition이 맞으면 `Max(1.0f, EggLayingMultiplier)`를 반환하는가?
7. `CalculateBeeIncreaseAmount()`만 변화하고 `CalculateBeeDecreaseAmount()`는 그대로인가?
8. bucket 동작 의미가 `ColonyPopulation` 후 `PollenPattyConsumption` 순서와 일치하는가?

---

## 리뷰 범위 (우선 파일)

### Inventory
- `Source/BeekeepingSim/Public/Inventory/PollenPattyItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/ItemDefinition.h`

### WorldActors
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

### 문서
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

---

## 상세 체크리스트

### 1) 타입/데이터 모델
- `UPollenPattyItemDefinition`가 `UItemDefinition` 호환 경로(`UItemDefinition*`)로 자연스럽게 사용되는지
- `UItemDefinition` 본체에 화분떡 전용 population 필드가 생기지 않았는지

### 2) Beehive bonus 계산
- `GetItemEggLayingBonus()`:
  - 대상 없음 -> `1.0f`
  - 대상 definition cast 실패 -> `1.0f`
  - cast 성공 -> `Max(1.0f, EggLayingMultiplier)`
- 대상 선택 helper가 기존 소모 대상 정책을 변경하지 않는지

### 3) 감소 항 불변성
- `CalculateBeeDecreaseAmount()` 계산식이 기존과 동일한지
- `GetItemLifespanBonus()`가 기존 의미(`1.0f`) 유지하는지

### 4) 정책 위반 여부
- 최고 tier/최대 multiplier를 전체 후보에서 탐색하는 코드가 없는지
- remaining ratio를 bonus 크기에 반영하는 코드가 없는지
- bonus 중첩(합/곱 누적) 경로가 없는지

---

## 코드 검색 기준

### 있어야 함
- `PollenPattyItemDefinition`
- `EggLayingMultiplier`
- `GetItemEggLayingBonus`

### 없어야 함
- `PollenPattyEggLayingMultiplier`
- `BeehivePopulationEffect`
- `PopulationBonus` (임의 전역 개념 추가)

권장 검색:

```powershell
rg "PollenPattyItemDefinition|EggLayingMultiplier|GetItemEggLayingBonus" Source/BeekeepingSim .md
rg "PollenPattyEggLayingMultiplier|BeehivePopulationEffect|PopulationBonus" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private
rg "CalculateBeeDecreaseAmount|GetItemLifespanBonus" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp
```

---

## 시나리오 검증 포인트

1. active 화분떡 없음 -> `GetItemEggLayingBonus()==1.0`
2. selected active 화분떡이 `UPollenPattyItemDefinition(1.2)` -> `1.2`
3. selected 화분떡 remaining 0 -> 대상 제외되어 `1.0`
4. 여러 화분떡 존재 -> selected 1개만 적용(중첩 없음)
5. 반대편에 더 높은 tier가 있어도 selected 화분떡 multiplier만 적용
6. selected actor definition이 일반 `UItemDefinition`이면 `1.0`
7. `CalculateBeeIncreaseAmount()`만 변하고 `CalculateBeeDecreaseAmount()`는 동일
8. 같은 경계에서 population update가 먼저 bonus 적용 후 consumption 처리

---

## 빌드/검증

- 가능하면 UBT 빌드 결과 확인:
  - `BeekeepingSimEditor Win64 Development`
- 빌드 불가 시 원인(에디터 락/환경 이슈) 분리 보고

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
