# 구현 수정 프롬프트: Placed Item Durability Remaining 리뷰 Findings

## 우선순위

1. Low: `UItemInstance::InitializeFromDefinition` durability 인자 계약 정리
2. Low: `WorldActorsSystem.md` Scope/과거 update 문구 동기화

## 발견 문제

### 1. `InitializeFromDefinition(..., InDurability)`가 durability item에서 인자를 무시함

- 대상 파일:
  - `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
  - `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
  - `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`
- 원인:
  - `UItemInstance::InitializeFromDefinition(UItemDefinition*, int32, float)`는 public/BlueprintCallable API에 `InDurability` 인자가 있다.
  - 하지만 `Definition->bUsesDurability == true`이면 `InDurability`를 사용하지 않고 항상 `Definition->MaxDurability`로 초기화한다.
  - 현재 state-aware 생성 경로는 이후 `SetDurability(DurabilityOverride)`를 호출해 보정하므로 C++ hotbar/storage/회수 경로는 동작한다.
- 영향:
  - Blueprint 또는 향후 C++ 코드가 `InitializeFromDefinition(..., InDurability)`를 직접 사용하면 durability 잔량이 full durability로 덮일 수 있다.
  - API 이름/인자와 실제 계약이 어긋나 유지보수자가 상태 보존 경로를 잘못 사용할 위험이 있다.
- 수정 방향:
  - 선택지 A: `InitializeFromDefinition`에서 durability item도 `InDurability`를 clamp 적용하도록 수정한다.
  - 선택지 B: 현재 full durability 초기화 정책을 유지하되 인자 의미를 제거/문서화하고, explicit durability 초기화는 `SetDurability` 또는 별도 helper로만 수행한다고 명확히 한다.
  - 권장: 선택지 A. 기존 기본값 `InDurability=1.0f`가 full durability와 다르게 해석될 수 있으므로 호출부 영향까지 확인한 뒤 적용한다. 위험하면 선택지 B로 계약을 명확히 한다.
- 수정 예:

```cpp
void UItemInstance::InitializeFromDefinition(UItemDefinition* InDefinition, int32 InStackCount, float InDurability)
{
	Definition = InDefinition;
	InstanceId = FGuid::NewGuid();
	StackCount = 0;

	if (Definition && Definition->bUsesDurability)
	{
		Durability = FMath::Clamp(InDurability, 0.0f, FMath::Max(0.0f, Definition->MaxDurability));
	}
	else
	{
		Durability = InDurability;
	}

	SetStackCount(InStackCount);
	RebuildActions();
}
```

적용 시 `ItemStackMoveUtils::CreateItemInstance(...)`의 full durability 기본 생성 경로가 깨지지 않도록 `InitializeFromDefinition(Definition, StackCount, Definition->MaxDurability)` 또는 현행 `SetDurability` 보정 경로를 같이 정리한다.

### 2. WorldActors 문서 Scope가 신규/기존 placed item 파일과 불완전하게 동기화됨

- 대상 파일:
  - `.md/Architecture/WorldActorsSystem.md`
- 원인:
  - Scope 목록에는 `PlacedItemRemaining*` 파일은 추가됐지만 `PlacedItemActor.h/.cpp`가 빠져 있다.
  - 하단 과거 update 일부에는 `UPlacedItemRetrieveFocusActionComponent`, `UPlacedItemRetrievePartFocusActionComponent` 중심 표현이 남아 있어 현재 generic retrieve 경로와 혼재되어 보인다.
- 영향:
  - 다음 구현/리뷰에서 `APlacedItemActor`가 WorldActors 정본 범위에서 누락될 수 있다.
  - retrieve 실행 주체가 `UPlacementSlotRetrievePartFocusActionComponent`인지, deprecated wrapper인지 혼동될 수 있다.
- 수정 방향:
  - Scope에 아래 파일을 추가한다.
    - `Source/BeekeepingSim/Public/WorldActors/PlacedItemActor.h`
    - `Source/BeekeepingSim/Private/WorldActors/PlacedItemActor.cpp`
    - 필요 시 현재 유지되는 retrieve wrapper 파일들도 명시한다.
  - 2026-05-27/2026-05-28 update 문구에서 현재 신규 경로는 `UPlacementSlotRetrievePartFocusActionComponent`, 기존 `UPlacedItemRetrieve*`는 compatibility wrapper라고 분명히 구분한다.
  - 2026-05-31 update의 durability remaining 설명은 유지한다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg -n "InitializeFromDefinition\\(" Source/BeekeepingSim`
  - `rg -n "CreateItemInstance\\(" Source/BeekeepingSim/Private/Inventory`
  - `rg -n "PlacedItemActor\\.h|PlacedItemActor\\.cpp|PlacedItemRetrieve|PlacementSlotRetrieve" .md/Architecture/WorldActorsSystem.md`
- 수동 확인:
  - durability item 생성 시 기본 full durability가 필요한 경로가 유지되는지 확인
  - durability override 생성 시 source/remaining durability가 보존되는지 확인
  - `APlacedItemActor`와 retrieve component 문서 설명이 현재 코드 구조와 일치하는지 확인

## 아키텍처 문서 반영 필요 여부

- 필요.
- 대상:
  - `.md/Architecture/WorldActorsSystem.md`
- 이유:
  - Scope 목록과 현재 placed item retrieve 구조 설명이 실제 Source 구조와 완전히 일치해야 한다.

## 참고 리뷰 결과

- High: 없음
- Medium: 없음
- Low:
  - `UItemInstance::InitializeFromDefinition` durability 인자 계약 불일치
  - `WorldActorsSystem.md` Scope/update 문구 동기화 누락
- UBT/UHT:
  - `BeekeepingSimEditor Win64 Development` 성공
- 제외 범위 확인:
  - `ABeehive` 화분떡 자동 소모 tick/bucket 로직 없음
  - 화분떡 colony population/산란력/수명 보너스 효과 없음
  - hotbar/storage UI 잔량 bar/overlay/tooltip 없음
  - C++ 구현은 Content asset 직접 수정에 의존하지 않음

---

# 구현 수정 프롬프트: Pollen Patty Population Bonus 리뷰 Findings

## 우선순위

1. Low: `PopulationBonus` helper 명칭 정리

## 발견 문제

### 1. `PopulationBonus` 문자열이 private helper 이름에 남아 있음

- 대상 파일:
  - `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
  - `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 원인:
  - 리뷰 프롬프트의 금지 검색 기준은 `PopulationBonus`를 임의 전역 개념 추가로 보지 않기 위해 소스에 남기지 않는 것이다.
  - 현재 구현은 전역 multiplier 필드나 별도 bonus 시스템을 만들지는 않았지만, private helper 이름에 `ResolveActivePollenPattyItemDefinitionForPopulationBonus`, `ResolvePlacedItemDefinitionForPopulationBonus`가 남아 있다.
- 영향:
  - 런타임 동작 결함은 없다.
  - 다만 검색 기준상 `rg "PollenPattyEggLayingMultiplier|BeehivePopulationEffect|PopulationBonus" Source/...`가 실패하며, `ItemEggLayingBonus`라는 기존 공식 항에만 적용한다는 정책 표현과 이름이 어긋난다.
- 수정 방향:
  - helper 이름에서 `PopulationBonus`를 제거하고 기존 공식 항 이름과 맞춘다.
  - 권장 이름:
    - `ResolveActivePollenPattyItemDefinitionForEggLayingBonus`
    - `ResolvePlacedItemDefinitionForEggLayingBonus`
  - 또는 더 짧게:
    - `ResolveSelectedPollenPattyItemDefinition`
    - `ResolveOccupiedItemDefinition`
  - 함수 동작은 변경하지 않는다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg "PollenPattyItemDefinition|EggLayingMultiplier|GetItemEggLayingBonus" Source/BeekeepingSim .md`
  - `rg "PollenPattyEggLayingMultiplier|BeehivePopulationEffect|PopulationBonus" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private`
  - `rg "CalculateBeeDecreaseAmount|GetItemLifespanBonus" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 기대 결과:
  - `PopulationBonus` 검색은 Source에서 0건이어야 한다.
  - `GetItemEggLayingBonus()`는 selected active pollen patty definition만 보고 `Max(1.0f, EggLayingMultiplier)`를 반환해야 한다.
  - `CalculateBeeDecreaseAmount()`와 `GetItemLifespanBonus()` 의미는 그대로 유지되어야 한다.

## 아키텍처 문서 반영 필요 여부

- 불필요.
- 이유:
  - 문서에는 해당 private helper 이름이 정본 계약으로 노출되어 있지 않다.
  - 동작/정책 변경 없이 소스 내부 명칭만 정리하는 작업이다.
