# Generic Placement Occupant + Beehive Comb Slot 구현 프롬프트

## 전제

이번 작업은 기존 generic item placement 흐름을 확장해, 화분떡뿐 아니라 소비장 및 향후 preplaced world item도 같은 배치/회수 구조를 사용하게 만든다.

핵심 책임 분리:

- `AItemPlacementSlotActor`: 배치, 점유 판단, empty slot use-area, preplaced occupant claim, clear 담당
- `UPlacementOccupantComponent`: 점유된 actor의 반환 `ItemDefinition`, owning slot, 회수 가능 조건, clear 전 hook 담당
- `UPlacementSlotRetrievePartFocusActionComponent`: secondary PartFocus 입력에서 generic 회수 실행 담당
- `ABeehiveCombSlotActor`: `AItemPlacementSlotActor` 기반 소비장 전용 slot subclass
- `ABeehiveCombActor`: 기존 소비장 actor/상호작용은 유지하되 placement occupant로 회수 가능하게 확장

구현 중 C++ source는 수정하되, `Content/` asset은 수정/저장하지 않는다. Blueprint 설정이 필요한 내용은 `.md/USER_UNREAL.md`에 작성한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

특히 `.md/QNA_ARCHITECTURE.md`의 다음 섹션 답변을 따른다.

- `벌통 소비장 슬롯 배치/회수 설계 QnA`
- `Generic Placement Occupant/Retrieve 설계 QnA`

## 확정 설계 결정

### 소비장 회수 상태 정책

- 소비장 회수 시 꿀 양과 visible face는 보존한다.
- `TargetBeeCount`가 0이 아니면 회수 불가다.
- 여왕벌이 해당 소비장에 attach/점유되어 있으면 회수 불가다.
- 소비장 상태 보존은 `UItemInstance` 확장으로 처리한다. 구현 중 저장 위치/형태가 애매하면 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

### 소비장 ItemDefinition 정책

- `ABeehive::DefaultCombItemDefinition`은 추가하지 않는다.
- item placement 경로로 배치된 소비장은 source item instance에서 반환 `ItemDefinition`을 주입받는다.
- 이미 월드에 배치된 소비장은 `UPlacementOccupantComponent::AuthoredReturnItemDefinition` fallback을 사용할 수 있다.
- runtime 주입값과 authored fallback이 모두 없으면 회수 불가다.

### Generic occupant/retrieve 정책

- 배치 점유자 계약은 `UPlacementOccupantComponent`로 구현한다.
- preplaced actor 연결은 slot actor의 `InitialOccupantActor`로 처리한다.
- `AuthoredReturnItemDefinition`은 `EditAnywhere`로 노출한다.
- 회수 가능 조건은 `UPlacementOccupantComponent`의 `BlueprintNativeEvent`로 확장한다.
- 기존 `APlacedItemActor`는 새 component/action으로 migration하되 기존 getter는 deprecated wrapper로 유지한다.
- clear 시 기본 destroy는 유지하되, destroy 전에 `PreClearPlacementOccupant` hook을 호출한다.

## 목표

1. `UPlacementOccupantComponent`를 추가한다.
2. `UPlacementSlotRetrievePartFocusActionComponent`를 추가한다.
3. `AItemPlacementSlotActor`를 generic occupied actor + initial occupant claim 구조로 확장한다.
4. 기존 `APlacedItemActor`/`UPlacedItemRetrievePartFocusActionComponent`를 새 generic 구조로 migration한다.
5. `ABeehiveCombSlotActor`를 추가한다.
6. `ABeehive`의 소비장 slot 관리가 `ABeehiveCombSlotActor`를 통해 active comb를 다루도록 전환한다.
7. `ABeehiveCombActor`에 occupant/retrieve 및 소비장 회수 가능 조건을 연결한다.
8. 관련 architecture 문서와 Unreal 수동 설정 문서를 갱신한다.
9. 가능하면 UBT 빌드를 수행한다.

## 신규 타입 1: UPlacementOccupantComponent

위치:

- `Source/BeekeepingSim/Public/WorldActors/PlacementOccupantComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacementOccupantComponent.cpp`

class:

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacementOccupantComponent : public UActorComponent
```

필수 property:

```cpp
UPROPERTY(Transient, BlueprintReadOnly, Category = "Placement Occupant")
TObjectPtr<UItemDefinition> RuntimeReturnItemDefinition = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Occupant")
TObjectPtr<UItemDefinition> AuthoredReturnItemDefinition = nullptr;

UPROPERTY(Transient, BlueprintReadOnly, Category = "Placement Occupant")
TObjectPtr<AActor> OwningPlacementSlotActor = nullptr;
```

필수 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
void InitializeFromPlacement(UItemInstance* SourceItemInstance, AActor* InOwningPlacementSlotActor);

UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
void SetOwningPlacementSlotActor(AActor* InOwningPlacementSlotActor);

UFUNCTION(BlueprintPure, Category = "Placement Occupant")
UItemDefinition* GetReturnItemDefinition() const;

UFUNCTION(BlueprintPure, Category = "Placement Occupant")
AActor* GetOwningPlacementSlotActor() const;

UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
bool CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const;

UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
void PreClearPlacementOccupant();
```

Blueprint hooks:

```cpp
UFUNCTION(BlueprintNativeEvent, Category = "Placement Occupant")
bool ReceiveCanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const;

UFUNCTION(BlueprintNativeEvent, Category = "Placement Occupant")
void ReceivePreClearPlacementOccupant();
```

정책:

- `InitializeFromPlacement`는 `SourceItemInstance->GetDefinition()`을 `RuntimeReturnItemDefinition`에 저장하고 owning slot을 저장한다.
- `GetReturnItemDefinition` 우선순위는 `RuntimeReturnItemDefinition`, 그 다음 `AuthoredReturnItemDefinition`이다.
- `CanRetrievePlacementOccupant` 기본 구현은 `GetReturnItemDefinition() != nullptr && OwningPlacementSlotActor != nullptr` 정도로 둔다.
- actor별 특수 회수 조건은 BP/C++ override로 구현한다.
- `PreClearPlacementOccupant` 기본 구현은 no-op이다.

## 신규 타입 2: UPlacementSlotRetrievePartFocusActionComponent

위치:

- `Source/BeekeepingSim/Public/WorldActors/PlacementSlotRetrievePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacementSlotRetrievePartFocusActionComponent.cpp`

class:

```cpp
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacementSlotRetrievePartFocusActionComponent : public UCursorPartFocusActionComponent
```

동작:

1. owner actor에서 `UPlacementOccupantComponent`를 찾는다.
2. occupant에서 `GetReturnItemDefinition()`을 조회한다.
3. occupant에서 `GetOwningPlacementSlotActor()`를 조회한다.
4. slot actor가 `IItemPlacementSlot`을 구현하는지 확인한다.
5. `CanRetrievePlacementOccupant(Character)`가 true인지 확인한다.
6. character hotbar에 `TryAcquireItem(ItemDefinition, 1)`을 호출한다.
7. `bSuccess && AddedQuantity == 1`일 때만 성공으로 본다.
8. 성공 시 `IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor)`를 호출한다.
9. 실패 시 actor와 slot 상태는 유지한다.

`CanHandleSecondaryPartFocusAction`과 `HandleSecondaryPartFocusAction`만 구현한다. LMB begin/cancel 흐름은 건드리지 않는다.

## AItemPlacementSlotActor 변경 요구

현재 `SlotMeshComponent`는 유지한다.

- 타입은 기존처럼 `UItemUseAreaMeshComponent`를 유지한다.
- 역할은 empty slot의 item-use-area hit/visual이다.
- occupied 상태에서는 hidden/no collision/inactive 상태가 된다.

추가/변경:

```cpp
UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Item Placement Slot")
TObjectPtr<AActor> InitialOccupantActor = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Placement Slot")
bool bAttachInitialOccupantToSlot = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Placement Slot")
bool bSnapInitialOccupantToAttachPoint = false;
```

현재 `PlacedActor` 저장 필드는 public/BP 호환성이 있으므로 즉시 rename하지 않는다.

- 내부 의미는 generic occupied actor로 확장한다.
- `GetPlacedActor()`는 deprecated 문구를 달 수 있으면 달고, 기존 API 호환을 위해 유지한다.
- 새 getter가 필요하면 `GetOccupiedActor()`를 추가한다.

필수 동작:

- `TryPlaceItem` 성공 후 spawned actor에 `UPlacementOccupantComponent`가 있으면 `InitializeFromPlacement(SourceItemInstance, this)` 호출
- 기존 `APlacedItemActor` 특수 cast 의존은 제거 또는 deprecated wrapper로 축소
- `ClearPlacedItem`은 occupied actor의 occupant component가 있으면 `PreClearPlacementOccupant()` 호출 후 destroy
- clear 후 `PlacedActor = nullptr`, visual refresh, PartFocus rebuild, ItemUseArea rebuild
- `IsPlacementOccupied`는 `PlacedActor` 유효성 기준으로 유지

preplaced claim:

- BeginPlay에서 `InitialOccupantActor`가 있고 아직 occupied가 아니면 claim한다.
- claim은 spawn이 아니라 기존 actor 등록이다.
- claim 대상은 `UPlacementOccupantComponent`를 가져야 한다. 없으면 claim 실패/로그.
- claim 성공 시 `PlacedActor = InitialOccupantActor`
- occupant component에 `SetOwningPlacementSlotActor(this)` 호출
- `bAttachInitialOccupantToSlot`이면 `AttachComponent`에 attach한다.
- `bSnapInitialOccupantToAttachPoint`이면 snap, 아니면 world transform 보존 attach를 사용한다.
- claim 후 visual/rebuild를 수행한다.

주의:

- OnConstruction에서 actor reference attach/destroy 같은 위험한 runtime mutation은 피한다. preplaced claim은 BeginPlay 중심으로 구현한다.
- Editor preview가 꼭 필요하면 별도 QnA로 묻는다.

## 기존 APlacedItemActor migration

`APlacedItemActor`에 다음 component를 기본 subobject로 추가한다.

- `UPlacementOccupantComponent`
- `UPlacementSlotRetrievePartFocusActionComponent`

기존 API는 유지하되 내부에서 component를 읽게 한다.

- `InitializePlacedItem(SourceItemInstance, SlotActor)`는 deprecated wrapper처럼 `PlacementOccupant->InitializeFromPlacement(...)` 호출
- `GetItemDefinition()`은 `PlacementOccupant->GetReturnItemDefinition()` 반환
- `GetOwningPlacementSlotActor()`는 `PlacementOccupant->GetOwningPlacementSlotActor()` 반환
- `GetPartFocusActionComponent()`는 새 generic retrieve action을 반환

기존 `UPlacedItemRetrievePartFocusActionComponent`:

- 즉시 삭제하지 않는다.
- 가능하면 `UPlacementSlotRetrievePartFocusActionComponent` subclass/deprecated wrapper로 축소한다.
- runtime 경로에서는 새 generic component를 사용한다.

기존 `UPlacedItemRetrieveFocusActionComponent`:

- global preview focus retrieve 경로는 현재 정본에서 사용하지 않는 legacy에 가깝다.
- 삭제/rename은 하지 말고, 필요 시 generic logic으로 내부만 정리한다.
- 삭제 필요성이 생기면 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

## 신규 타입 3: ABeehiveCombSlotActor

위치:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombSlotActor.cpp`

class:

```cpp
UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeehiveCombSlotActor : public AItemPlacementSlotActor
```

책임:

- 소비장 전용 placement slot
- `ABeehiveCombActor` 계열 actor만 배치 허용
- slot empty/occupied use-area 동작은 parent 재사용
- place/clear/initial claim 후 owning beehive에 comb layout/part focus/item-use-area 갱신 요청

필수 API:

```cpp
UFUNCTION(BlueprintPure, Category = "Beehive|Comb Slot")
ABeehiveCombActor* GetPlacedCombActor() const;
```

override:

- `TryPlaceItem_Implementation`
  - `PlacedActorClass`가 `ABeehiveCombActor` 계열인지 검증
  - 통과하면 parent `TryPlaceItem_Implementation` 호출
  - 성공 후 beehive 갱신 요청
- `ClearPlacedItem_Implementation`
  - parent clear 호출
  - beehive 갱신 요청

preplaced initial occupant:

- parent claim 후 해당 actor가 `ABeehiveCombActor`가 아니면 claim 실패 처리한다.
- parent에 hook이 필요하면 protected virtual `CanAcceptOccupantActor(AActor*)` 같은 확장점을 추가한다.

갱신 요청:

- parent의 host rebuild 경로를 활용한다.
- 소비장 slot이 `ABeehive`의 child actor로 붙는 구조면 `GetAttachParentActor()`를 `ABeehive`로 해석한다.
- `ABeehive::RebuildCursorPartFocusDescriptors()`
- `UCursorItemUseAreaScopeComponent::RebuildItemUseAreaDescriptors()`
- 필요하면 `ABeehive`에 public refresh API를 추가한다.

## ABeehive 변경 요구

현재 `ABeehive`는 `UChildActorComponent` 배열로 소비장 actor를 직접 생성한다. 이번 작업에서는 slot child actor를 생성하고, active comb는 slot의 placed comb로 조회한다.

추가/변경 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
TSubclassOf<ABeehiveCombSlotActor> CombSlotActorClass;
```

기존 `CombActorClass`는 즉시 삭제하지 않는다.

- 새로 배치되는 소비장 item의 `UItemPlacementUseAction::PlacedActorClass`가 comb actor class를 소유한다.
- 기존 초기 자동 생성 정책이 남아 있다면 migration/compatibility 용도로만 최소 유지한다.
- 삭제가 필요하면 Blueprint 영향 때문에 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

slot component 배열:

- 기존 `CombSlotComponents`는 child actor component 배열로 유지 가능하다.
- 각 child actor component의 class는 `ABeehiveCombSlotActor` 계열이어야 한다.
- 이름/배치/spacing 정책은 기존 `CombSlotSpacing`, `CombRackRoot`, `MaxCombCount` 흐름을 최대한 유지한다.

active comb 조회 변경:

- `FindManagedCombSlotIndex(const ABeehiveCombActor*)`
  - 각 slot component child actor를 `ABeehiveCombSlotActor`로 cast
  - `GetPlacedCombActor()`가 입력 comb와 같으면 index 반환
- `GetCombSlotComponentByIndex`
  - 기존처럼 slot child actor component 반환
- `GetCombSlotWorldTransformByIndex`
  - slot component transform 기준 유지
- `GetLiftedCombActor`
  - lift component index -> slot actor -> placed comb actor

기존 active comb 순회 로직 변경 대상:

- `RegisterCombPartsToScope`
- `RefreshCombSpawnAmounts`
- `ApplyColonyPopulationUpdate`
- `ApplyHoneyProductionUpdate`
- `ChooseQueenBeeCombSlotIndex`
- `ResolveQueenBeeAttachPoint`
- `DistributeHoneyIncreaseToCombs`
- `IsManagedActiveCombActor`

각 로직은 `ABeehiveCombSlotActor::GetPlacedCombActor()`가 null인 slot은 skip한다.

CurrentCombCount 정책:

- `CurrentCombCount`는 legacy/test compatibility로 유지할 수 있다.
- 새 구조에서 “점유된 소비장 수”와 다를 수 있으므로, 의미가 애매해지면 `.md/QNA_IMPLEMENTATION.md`에 질문한다.
- 권장은 `MaxCombCount`개의 slot은 항상 존재하고, 실제 active comb는 occupied slot 기준으로 계산하는 것이다.

queen/bee/honey 정책:

- queen 위치 후보는 placed comb가 있는 slot만 포함한다.
- lifted comb slot 제외 정책 유지.
- `TargetBeeCount`가 0이 아닌 소비장은 회수 불가이므로, 회수 성공 후 population/honey 갱신 대상에서 자연스럽게 제외된다.
- 여왕벌이 붙은 소비장 회수 차단을 위해 `ABeehive` 또는 comb occupant hook에서 queen attach 상태 확인 API가 필요할 수 있다.

## ABeehiveCombActor 변경 요구

추가 component:

- `UPlacementOccupantComponent`
- `UPlacementSlotRetrievePartFocusActionComponent` 또는 기존 comb PartFocus action에 secondary retrieve 위임

중요:

- LMB 소비장 lift/return은 기존 `UBeehiveCombPartFocusActionComponent` 경로를 유지한다.
- RMB/secondary retrieve는 generic retrieve action을 사용한다.
- 하나의 descriptor/action handler에서 LMB와 secondary를 모두 처리해야 하는 구조라면 `UBeehiveCombPartFocusActionComponent`가 secondary에서 `UPlacementSlotRetrievePartFocusActionComponent` helper를 호출하게 구성한다.
- 별도 retrieve action component를 붙이더라도 descriptor의 `ActionHandler`가 하나만 허용되는지 확인한다. descriptor가 action 1개만 받는다면 기존 comb action에 secondary retrieve 기능을 bridge한다.

소비장 회수 가능 조건:

- `UPlacementOccupantComponent::ReceiveCanRetrievePlacementOccupant`를 BP override하거나, C++ subclass/hook으로 구현한다.
- 조건:
  - `GetReturnItemDefinition()` 유효
  - owning slot 유효
  - `TargetBeeCount == 0`
  - 여왕벌이 이 comb에 붙어 있지 않음
- 꿀 양과 visible face는 회수 가능 조건에서 차단하지 않는다. 보존 대상이다.

상태 보존:

- 꿀 양과 visible face를 item instance에 보존해야 한다.
- 현재 `UItemInstance`에 generic custom state 저장 계약이 없으면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.
- 임시로 상태를 버리는 구현은 금지한다.

## Inventory / ItemInstance 검토 요구

소비장 회수 시 꿀 양과 visible face 보존이 확정되어 있다.

검토할 것:

- `UItemInstance`가 arbitrary runtime state를 저장할 수 있는지
- 없다면 최소 확장 방식 후보를 `.md/QNA_IMPLEMENTATION.md`에 질문할 것

질문 없이 임의로 `UItemInstance`에 대형 generic serialization system을 추가하지 않는다.

가능한 질문 예:

- 소비장 전용 item state struct를 둘지
- item instance에 `InstancedStruct`/tagged payload 류 generic state를 둘지
- 1차 구현에서 상태 보존 범위를 별도 component로 미룰지

단, 사용자가 이미 “꿀양과 visible face만 보존”이라고 답했으므로, 상태를 버리는 옵션은 제안하지 않는다.

## Focus / PartFocus 검토 요구

확인할 것:

- `FCursorPartFocusPartDescriptor`가 `ActionHandler` 하나만 지원하는지
- secondary input이 현재 hovered part의 `ActionHandler->HandleSecondaryPartFocusAction`으로만 가는지

제약:

- 소비장 LMB action을 깨면 안 된다.
- `UBeehiveCombPartFocusActionComponent`의 begin/cancel/abort 정책은 유지한다.

권장 구현:

- `UBeehiveCombPartFocusActionComponent`에 secondary retrieve bridge를 추가한다.
- 내부에서 owner comb의 `UPlacementOccupantComponent`를 찾고 generic retrieve helper/static function/component API를 호출한다.
- generic retrieve action과 code duplication을 피하기 위해 retrieve 실행 로직을 private helper 또는 component method로 공유한다.

## AItemPlacementSlotActor SlotMeshComponent 유지

`SlotMeshComponent`는 삭제하지 않는다.

역할:

- empty slot의 item-use-area hit/visual
- placement item의 `UseAreaTagQuery` 매칭 대상
- occupied 상태에서 hidden/collision off/inactive

`UPlacementOccupantComponent`는 `SlotMeshComponent`를 대체하지 않는다.

## Editor 작업 문서화

`.md/USER_UNREAL.md`에 추가:

1. `BP_Beehive` 또는 벌통 BP
   - comb slot child actor class를 `ABeehiveCombSlotActor` subclass로 설정
   - 기존 소비장 직접 child actor 방식에서 slot child actor 방식으로 전환
2. `BP_BeehiveCombSlot`
   - `SlotMeshComponent`의 mesh/material/transform 설정
   - `AreaTags`를 소비장 배치 item의 query와 맞춤
   - 필요 시 `InitialOccupantActor` 설정
   - preplaced 소비장은 attach/snap 옵션 확인
3. `BP_BeehiveComb`
   - `UPlacementOccupantComponent` 존재 확인
   - `AuthoredReturnItemDefinition` 설정 가능
   - generic retrieve/secondary 경로 확인
   - target bee count/queen attached 회수 차단 조건 구현 확인
4. 소비장 item definition
   - `UItemPlacementUseAction::PlacedActorClass`를 소비장 actor BP로 설정
   - use-area tag query를 comb slot tag와 맞춤
5. 기존 placed item BP
   - `UPlacementOccupantComponent`와 generic retrieve action이 붙었는지 확인
   - 기존 retrieve BP 노드가 있다면 deprecated 경고 확인

## 문서 갱신

구현 후 갱신:

- `.md/0_ARCHITECTURE.md`
  - generic placement occupant/retrieve 흐름
  - 소비장 slot 구조 요약
- `.md/Architecture/WorldActorsSystem.md`
  - `UPlacementOccupantComponent`
  - `UPlacementSlotRetrievePartFocusActionComponent`
  - `ABeehiveCombSlotActor`
  - `AItemPlacementSlotActor` initial occupant claim
  - `ABeehive` comb slot 관리 변경
- `.md/Architecture/FocusSystem.md`
  - PartFocus secondary retrieve가 generic action/helper로 처리되는 경로
- `.md/Architecture/InventorySystem.md`
  - 소비장 회수 state 보존 계약. 구현이 QnA로 보류되면 보류 사실 명시
- `.md/USER_UNREAL.md`
  - BP/editor migration 절차

## 검증 기준

### 검색

있어야 함:

- `UPlacementOccupantComponent`
- `UPlacementSlotRetrievePartFocusActionComponent`
- `ABeehiveCombSlotActor`
- `InitialOccupantActor`
- `AuthoredReturnItemDefinition`
- `RuntimeReturnItemDefinition`
- `PreClearPlacementOccupant`
- `CanRetrievePlacementOccupant`

runtime 경로에서 줄어들어야 함:

- `APlacedItemActor` hard cast 기반 retrieve 로직
- placed item 전용 retrieve component에만 회수 규칙이 묶인 구조

### 동작 검증

1. 기존 화분떡 배치
   - empty slot에 배치 가능
   - occupied slot에는 다시 배치 불가
   - secondary 회수 시 hotbar 공간 있으면 회수
   - 공간 없으면 actor/slot 유지
2. preplaced item
   - slot의 `InitialOccupantActor`로 claim
   - authored fallback item definition으로 회수
   - 회수 성공 시 slot clear
3. 소비장 배치
   - empty comb slot에 소비장 item 배치
   - 배치된 소비장은 기존 LMB lift/return 가능
   - BeeBrush use area 기존 동작 유지
4. 소비장 회수
   - `TargetBeeCount == 0`이고 여왕벌 attach 없음이면 secondary 회수 가능
   - `TargetBeeCount > 0`이면 회수 실패, actor/slot 유지
   - 여왕벌이 붙어 있으면 회수 실패, actor/slot 유지
   - 꿀 양과 visible face는 회수 후 item state로 보존
5. 벌통 runtime
   - honey production은 placed comb만 대상으로 수행
   - colony population은 placed comb만 대상으로 수행
   - queen location 후보는 placed comb만 포함
   - lifted comb 제외 정책 유지

### 빌드

가능하면 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 소비장 꿀 양/visible face를 `UItemInstance`에 보존할 안정적 계약이 없는 경우
- `FCursorPartFocusPartDescriptor`가 기존 comb LMB action과 generic retrieve secondary action을 동시에 수용하기 어려운 경우
- `CurrentCombCount` 의미가 새 slot occupancy 구조와 충돌해 public API 의미 변경이 필요한 경우
- 기존 `APlacedItemActor` BP 참조 때문에 retrieve component migration이 asset break를 유발할 가능성이 큰 경우
- preplaced initial occupant claim을 construction에서 처리해야 한다는 요구가 생기는 경우
- `CombActorClass` 삭제/rename이 필요해지는 경우
- UCLASS/UFUNCTION/UPROPERTY rename 또는 Core Redirect가 필요해지는 경우
