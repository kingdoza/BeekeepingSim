# Pollen Patty Population Bonus 구현 프롬프트

## 목표

벌통(`ABeehive`)에 배치된 화분떡이 colony population 증가량을 가속하도록 구현한다.

화분떡 tier별 효과 수치는 일반 `UItemDefinition`이 아니라 `UPollenPattyItemDefinition : UItemDefinition` subclass에 둔다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

특히 `.md/QNA_ARCHITECTURE.md`의 `화분떡 인구 가속효과 설계 QnA` 27-32번 답변을 따른다.

## 전제

- 화분떡은 generic `APlacedItemActor`로 배치된다.
- `APlacedItemActor`는 `UPlacedItemRemainingComponent`를 가진다.
- `ABeehive`에는 이미 화분떡 고정 소모 로직이 있다.
  - `PollenPattyConsumptionAreaTags`
  - `PollenPattyConsumptionSide`
  - `FindPollenPattyConsumptionTargetSlot(...)`
  - `DoesSlotMatchPollenPattyConsumptionTags(...)`
  - `ApplyPollenPattyConsumptionUpdate()`
- 화분떡 소모 대상 선택은 벌통 local Y 기준 `Leftmost/Rightmost` 1개 선택 정책을 따른다.
- `Content/` asset은 직접 수정하거나 resave하지 않는다.

위 전제가 현재 Source와 맞지 않으면 구현하지 말고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

## 명시적 제외 범위

- `UItemDefinition` 본체에 인구 가속 float 추가 금지
- `ABeehive`에 전역 `PollenPattyEggLayingMultiplier` 추가 금지
- `ItemLifespanBonus` 또는 감소량(`Decrease`) 수정 금지
- 여러 화분떡 효과 중첩 금지
- 최고 tier/최대 multiplier 화분떡을 별도로 찾아 적용하는 정책 금지
- remaining ratio에 따라 bonus 크기를 조절하는 처리 금지
- `Content/` asset 직접 수정 금지

## 확정 정책

1. 화분떡 인구 가속효과는 colony population 증가 항에만 적용한다.
2. 기존 공식의 `ItemEggLayingBonus`가 화분떡 효과를 반영한다.
3. 감소 항 `ItemLifespanBonus`/`Decrease`에는 관여하지 않는다.
4. active 화분떡이 여러 개여도 bonus는 중첩하지 않는다.
5. bonus 대상은 최고 tier가 아니라 기존 화분떡 소모 대상 선택 정책과 동일하게 고른다.
6. `PollenPattyConsumptionSide` 기준 leftmost/rightmost active 화분떡 1개를 선택한다.
7. 선택된 화분떡의 `EggLayingMultiplier`만 적용한다.
8. remaining amount가 0보다 크면 full bonus를 적용한다.
9. remaining ratio는 bonus 크기에 관여하지 않는다.
10. `ColonyPopulation` bucket이 먼저 처리되고, 이후 `PollenPattyConsumption` bucket이 처리된다.
11. bonus 대상 식별은 `PollenPattyConsumptionAreaTags` + active `UPlacedItemRemainingComponent` 기준을 재사용한다.
12. 선택된 occupied actor의 item definition이 `UPollenPattyItemDefinition`이 아니면 bonus는 `1.0f`다.

## 주요 구현 대상

Inventory:

- `Source/BeekeepingSim/Public/Inventory/PollenPattyItemDefinition.h`
- `Source/BeekeepingSim/Private/Inventory/PollenPattyItemDefinition.cpp`는 필요할 때만 추가한다.

WorldActors:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

## `UPollenPattyItemDefinition`

`UItemDefinition` subclass를 추가한다.

권장 header:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "PollenPattyItemDefinition.generated.h"

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UPollenPattyItemDefinition : public UItemDefinition
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pollen Patty", meta = (ClampMin = "1.0"))
    float EggLayingMultiplier = 1.2f;
};
```

정책:

- 일반 item asset은 계속 `UItemDefinition`을 사용한다.
- 화분떡 item asset만 `UPollenPattyItemDefinition`을 사용한다.
- 여러 tier 화분떡은 `UPollenPattyItemDefinition` asset을 여러 개 만들고 `EggLayingMultiplier`만 다르게 둔다.
- 별도 virtual function이나 interface는 이번 범위에 추가하지 않는다.
- `UItemDefinition::GetPrimaryAssetId()` override는 건드리지 않는다. subclass는 기존 `ItemId` 정책을 그대로 상속한다.

## `ABeehive::GetItemEggLayingBonus()`

현재 구현은 `1.0f`를 반환한다.

변경 후 흐름:

```cpp
float ABeehive::GetItemEggLayingBonus() const
{
    const UPollenPattyItemDefinition* PollenPattyDefinition = ResolveActivePollenPattyItemDefinitionForPopulationBonus();
    if (!PollenPattyDefinition)
    {
        return 1.0f;
    }

    return FMath::Max(1.0f, PollenPattyDefinition->EggLayingMultiplier);
}
```

helper 이름은 기존 style에 맞게 조정해도 된다.

## 대상 선택 helper

기존 `FindPollenPattyConsumptionTargetSlot(UPlacedItemRemainingComponent*& OutRemainingComponent) const`가 현재 Source에 있으면 재사용한다.

권장:

```cpp
const UPollenPattyItemDefinition* ABeehive::ResolveActivePollenPattyItemDefinitionForPopulationBonus() const
{
    UPlacedItemRemainingComponent* RemainingComponent = nullptr;
    const AItemPlacementSlotActor* TargetSlot = FindPollenPattyConsumptionTargetSlot(RemainingComponent);
    if (!TargetSlot || !RemainingComponent)
    {
        return nullptr;
    }

    AActor* OccupiedActor = TargetSlot->GetOccupiedActor();
    if (!OccupiedActor)
    {
        return nullptr;
    }

    const UItemDefinition* ItemDefinition = ResolvePlacedItemDefinition(OccupiedActor);
    return Cast<UPollenPattyItemDefinition>(ItemDefinition);
}
```

필요하면 private helper를 추가한다.

```cpp
const UItemDefinition* ResolvePlacedItemDefinitionForPopulationBonus(const AActor* OccupiedActor) const;
```

item definition resolve 우선순위:

1. `APlacedItemActor`이면 `GetItemDefinition()`
2. 아니면 `UPlacementOccupantComponent::GetReturnItemDefinition()`
3. 둘 다 없으면 `nullptr`

주의:

- 새 helper는 selection policy를 바꾸지 않는다.
- 최고 tier/최대 multiplier 탐색을 하지 않는다.
- 여러 후보가 있어도 기존 소모 대상 선택 결과 1개만 본다.
- `PollenPattyConsumptionAreaTags`가 비어 있으면 기존 helper가 대상 없음으로 처리하므로 bonus도 `1.0f`다.
- selected actor에 remaining이 있고 `CurrentAmount > 0`인 조건은 기존 helper 조건을 그대로 따른다.

## Include 지침

`Beehive.cpp`에 필요한 include를 추가한다.

예:

```cpp
#include "Inventory/PollenPattyItemDefinition.h"
#include "WorldActors/PlacedItemActor.h"
#include "WorldActors/PlacementOccupantComponent.h"
```

`Beehive.h`에는 `UPollenPattyItemDefinition`을 노출할 필요가 없으면 forward declaration만 사용하거나 아예 cpp-local helper로 둔다.

## 공식 검증

기존 공식:

```text
Increase = QueenBaseEggLayingPower * ItemEggLayingBonus * TemperatureScore * BeeIncreaseCoefficient
Decrease = ColonyBeeCount * BeeDecreaseCoefficient / ItemLifespanBonus / TemperatureScore
```

변경 후:

```text
ItemEggLayingBonus =
  selected active pollen patty가 UPollenPattyItemDefinition이면 Max(1.0, EggLayingMultiplier)
  아니면 1.0
```

`CalculateBeeDecreaseAmount()`는 수정하지 않는다.

## 문서 갱신

구현 후 아래 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - 화분떡 인구 가속효과가 `ItemEggLayingBonus`로 증가 항에만 적용된다고 기록
  - 효과 수치는 `UPollenPattyItemDefinition::EggLayingMultiplier`에서 읽는다고 기록
  - 여러 active 화분떡은 중첩하지 않고 기존 소모 대상 선택 정책과 동일한 1개만 적용한다고 기록
- `.md/Architecture/InventorySystem.md`
  - `UPollenPattyItemDefinition` 역할 추가
  - 일반 `UItemDefinition`에는 인구 가속 필드를 추가하지 않는다고 기록
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehive::GetItemEggLayingBonus()`가 선택된 active 화분떡 definition을 참조한다고 기록
  - `PollenPattyConsumptionAreaTags`/remaining 기준을 bonus 대상 식별에도 재사용한다고 기록
  - `ColonyPopulation`이 먼저 bonus를 적용하고 이후 `PollenPattyConsumption`이 소모된다고 기록
- `.md/USER_UNREAL.md`
  - 화분떡 item definition asset은 `UPollenPattyItemDefinition` class로 만들거나 reparent해야 한다고 기록
  - tier별 `EggLayingMultiplier` 설정 예시를 기록
  - 기존 화분떡 durability placed remaining 설정은 유지해야 한다고 기록

## 검색 검증

```powershell
rg "PollenPattyItemDefinition|EggLayingMultiplier|GetItemEggLayingBonus" Source/BeekeepingSim .md
rg "PollenPattyEggLayingMultiplier|BeehivePopulationEffect|PopulationBonus" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private
rg "CalculateBeeDecreaseAmount|GetItemLifespanBonus" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp
```

확인할 것:

- `UItemDefinition` 본체에 인구 가속 float가 추가되지 않았다.
- `ABeehive`에 전역 `PollenPattyEggLayingMultiplier`가 추가되지 않았다.
- `GetItemEggLayingBonus()`가 selected active pollen patty definition만 본다.
- `CalculateBeeDecreaseAmount()`와 `GetItemLifespanBonus()`는 기존 의미를 유지한다.
- source에서 `Content/` asset 수정이 없다.

## 시나리오 검증

가능하면 PIE 또는 디버그 호출로 확인한다.

1. active 화분떡이 없으면 `GetItemEggLayingBonus() == 1.0f`
2. selected active 화분떡이 `UPollenPattyItemDefinition(EggLayingMultiplier=1.2)`이면 `GetItemEggLayingBonus() == 1.2f`
3. selected active 화분떡 remaining amount가 0이면 대상에서 제외되고 bonus는 `1.0f`
4. 여러 화분떡이 있어도 `PollenPattyConsumptionSide` 기준 selected 1개만 적용된다.
5. selected 화분떡이 낮은 tier이고 반대쪽에 높은 tier가 있어도 selected 낮은 tier multiplier가 적용된다.
6. selected occupied actor의 item definition이 `UPollenPattyItemDefinition`이 아니면 bonus는 `1.0f`
7. `CalculateBeeIncreaseAmount()`만 변하고 `CalculateBeeDecreaseAmount()`는 변하지 않는다.
8. 같은 bucket 경계에서 population update 후 consumption이 수행되어, 소모 직전 active 화분떡이 해당 update에 bonus를 준다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `FindPollenPattyConsumptionTargetSlot(...)` 또는 동등한 기존 소모 대상 선택 helper가 없고, 선택 정책을 새로 정의해야 하는 경우
- occupied actor에서 item definition을 안정적으로 resolve할 수 없는 경우
- `UPollenPattyItemDefinition` subclass asset이 기존 item acquisition/stack/move 경로에서 `UItemDefinition*`로 호환되지 않는 근거가 발견되는 경우
- primary asset scan 설정 때문에 subclass data asset을 에디터에서 만들거나 로드하는 방식이 불명확한 경우
- QNA 27-32번 확정 답변과 현재 Source 구조가 충돌하는 경우
