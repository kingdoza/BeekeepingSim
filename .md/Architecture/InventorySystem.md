# Inventory System

## Scope

- `Source/BeekeepingSim/Public/Inventory/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Public/Inventory/StorageBoxComponent.h`
- `Source/BeekeepingSim/Private/Inventory/StorageBoxComponent.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemDefinition.h`
- `Source/BeekeepingSim/Private/Inventory/ItemDefinition.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemAction.h`
- `Source/BeekeepingSim/Private/Inventory/ItemAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/HoldItemUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/HoldItemUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/ActiveUseDurabilityItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/DisinfectantUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/DisinfectantUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/SmokerUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/SmokerUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/BeeBrushUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/BeeBrushUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/CombUncappingUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/CombUncappingUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/PollenPattyUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/PollenPattyUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/PollenPattyItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/ItemPlacementUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/ItemPlacementUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemActionContext.h`
- `Source/BeekeepingSim/Public/Inventory/ItemActionTypes.h`
- `Source/BeekeepingSim/Public/Inventory/HotbarItemInterface.h`
- `Source/BeekeepingSim/Public/Inventory/HotbarPresentationTypes.h`
- `Source/BeekeepingSim/Public/Inventory/ItemPresentationActor.h`
- `Source/BeekeepingSim/Private/Inventory/ItemPresentationActor.cpp`
- `Source/BeekeepingSim/Public/Inventory/VfxItemPresentationActor.h`
- `Source/BeekeepingSim/Private/Inventory/VfxItemPresentationActor.cpp`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`

## Responsibilities

- hotbar/storage 슬롯 상태 오너십
- item definition/instance/action 런타임 모델 제공
- item stack 병합, 분할 이동, swap, state-aware acquire 결과 계산
- focus item rule을 hotbar enabled state로 반영
- held/on-cursor presentation actor의 기반 class 제공
- hold-use action의 use-area tag query와 지속 효과 lifecycle 제공

## Key Classes

- `UBeekeeperHotbarComponent`: 8슬롯 player hotbar 상태 오너
- `UStorageBoxComponent`: storage 슬롯 상태 오너
- `UItemDefinition`: 정적 아이템 데이터 asset
- `UItemInstance`: 런타임 아이템 상태와 action 소유 객체
- `UItemAction`: item definition action spec에서 생성되는 런타임 행동 베이스
- `UHoldItemUseAction`: use-area tag query + LMB use session lifecycle + 효과 적용 경계 베이스
- `UActiveUseDurabilityItemDefinition`: active-use durability drain spec(`DurabilityDrainPerSecond`, `DrainPolicy`, `bRemoveItemWhenDepleted`)를 소유하는 `UItemDefinition` subclass
- `UDisinfectantUseAction`: continuous hold-use 동안 벌통 위생성 증가 효과 action
- `USmokerUseAction`: continuous hold-use 동안 벌통 공격성 감소 효과 action (item 소비 없음)
- `UBeeBrushUseAction`: lifted comb use-area에서 visible face bee target 감소와 queen relocation 요청을 수행하는 action
- `UCombUncappingUseAction`: 작업대 소비장 capping use-area에서 context hit point 기반 원형 brush로 현재 visible face의 wax capping mask를 제거하는 hold-use action. 작업대 comb PartFocus 잡기 상태에서는 begin/apply를 차단한다.
- `UItemPlacementUseAction`: slot interface 기반 generic placed-actor 배치 action
- `UPollenPattyUseAction`: `UItemPlacementUseAction` 기반 wrapper(화분떡 태그/이벤트 유지)
- `UPollenPattyItemDefinition`: 화분떡 tier별 인구 가속효과(`EggLayingMultiplier`)를 소유하는 `UItemDefinition` subclass
- `FItemActionSpec`: item definition에 저장되는 action class/tag 데이터
- `FItemActionContext`: action 실행 시 Character, FocusEngaged host, item-use-area target/hit context를 전달하는 DTO
- `FItemActionExecutionResult`: action 실행 성공, 소비 여부, stack delta, durability delta, 메시지를 담는 결과 DTO
- `FItemAcquireSpec`: definition, quantity, durability override를 포함하는 state-aware acquire 요청 DTO
- `FHotbarItemAcquireResult`: acquire 성공 여부, 추가 수량, 마지막 변경 slot/item instance를 전달하는 결과 DTO
- `AItemPresentationActor`: first-person held/on-cursor 표시 actor 베이스
- `AVfxItemPresentationActor`: `AItemPresentationActor` 기반 held-item VFX presentation actor
- `ItemStackMoveUtils`: private stack 계산/생성 helper

## Held Presentation Active Lifecycle

- `UDisinfectantUseAction` 같은 hold-use action은 held item visualizer에 active start/end만 알린다.
- `UBeekeeperHeldItemVisualizerComponent`는 현재 `AItemPresentationActor`에 다음 base API를 호출한다:
  - `BeginItemUseActive()`
  - `EndItemUseActive(bool bCanceled)`
- `AItemPresentationActor`는 generic active state/event만 소유한다:
  - `bIsItemUseActive`
  - `ReceiveItemUseActiveStarted`
  - `ReceiveItemUseActiveEnded`
- `AVfxItemPresentationActor`는 reusable held-item VFX presentation subclass다.
  - `UseVfxComponent` Niagara component를 소유한다.
  - active start에서 optional reset + `Activate(true)`를 수행한다.
  - active end에서 `Deactivate()` 또는 `DeactivateImmediate()`를 수행한다.
- VFX asset/transform/renderer parameter는 C++ 별도 property가 아니라 Niagara component details/BP에서 authoring한다.

## Item Model

- `UItemDefinition`은 표시명, 설명, 아이콘, `WorldMesh`, `HeldPresentationActorClass`, gameplay tag container, max stack, durability 설정, action spec 배열을 가진다.
- `UItemInstance`는 definition, stack count, durability, instance id, action instance 배열을 가진다.
- `UItemInstance`는 `IHotbarItemInterface`를 구현해 focus item rule 평가에 필요한 tag를 제공한다.
- `InitializeFromDefinition()`은 definition을 저장하고 stack/durability를 clamp한 뒤 `RebuildActions()`로 action instance를 재생성한다.
- Action 객체의 outer는 `UItemInstance`다. Definition의 `FItemActionSpec::ActionClass`가 abstract이면 생성하지 않는다.
- `UItemAction::InitializeAction()`은 owning item instance와 action spec을 받아 action tag를 초기화한다.
- `UItemInstance::FindActionByTag()` / `HasActionByTag()` / `ExecuteActionByTag()`는 action tag 기반 실행 경로다.
- `UItemInstance::FindHoldItemUseAction()`은 선택 아이템의 대표 `UHoldItemUseAction` 1개를 조회한다. 현재 구현은 action 배열에서 처음 발견한 hold action을 반환한다.

## Item Action Lifecycle

- 일반 action:
  - `CanExecute(Context)`로 실행 가능 여부를 판정한다.
  - `Execute(Context)`는 `FItemActionExecutionResult`를 반환한다.
  - 기본 구현은 실패/무효 결과를 반환하므로 실제 효과는 subclass가 구현한다.
- Hold-use action:
  - `GetUseAreaTagQuery()`가 사용할 수 있는 item-use-area tag query를 제공한다.
  - `CanBeginUse(Context)`와 `BeginUse(Context)`가 LMB use session 시작 여부를 결정한다.
  - `TickUse(Context, DeltaTime)`는 use session이 진행 중일 때 매 Tick 호출될 수 있다.
  - `CanApplyUseEffect(Context)`와 `ApplyUseEffect(Context, DeltaTime)`는 유효 item-use area target이 있을 때의 실질 효과 적용 경계다.
  - `ResolveActiveUseDurabilityDelta(Context, EffectResult, DeltaTime, bIsOverValidUseArea)`는 active-use durability drain delta를 계산한다.
  - `EndUse(Context, bWasCanceled)`는 release/cancel/deactivate 경로에서 session 종료를 받는다.
- `UHoldItemUseAction`는 C++ virtual API를 유지하면서 `BlueprintNativeEvent` hook(`ReceiveCanBeginUse`, `ReceiveBeginUse`, `ReceiveTickUse`, `ReceiveEndUse`, `ReceiveCanApplyUseEffect`, `ReceiveApplyUseEffect`)을 함께 제공한다.
- `FItemActionExecutionResult::bConsumedItem`/`StackDelta`와 `DurabilityDelta`는 action 결과 DTO다. 실제 mutation 적용 주체는 호출 경로가 명시해야 하며 action base가 자동으로 inventory를 mutation하지 않는다.
- Focus item-use-area 경로에서는 `UCursorItemUseAreaScopeComponent`가 `ApplyUseEffect` 결과를 해석하고, stack mutation은 `UBeekeeperHotbarComponent::ApplySelectedItemStackDelta`, durability mutation은 `ApplySelectedItemDurabilityDelta` authority 경로를 사용한다.
- placement action은 `IItemPlacementSlot::TryPlaceItem`을 통해 월드 슬롯 배치를 요청한다.
- placement 성공 후 stack delta 적용이 실패하면 scope가 slot interface(`ClearPlacedItem`)로 rollback한다.

## Slot Mutation Model

- Hotbar item outer는 `UBeekeeperHotbarComponent`다.
- Storage item outer는 `UStorageBoxComponent`다.
- Stack 이동 helper는 아래만 담당한다.
  - max stack 계산
  - 동일 definition 판정
  - available stack space 계산
  - merge 수량 계산/적용
  - 새 `UItemInstance` 생성
  - `FItemSlotMoveResult::RemainingQuantity` 계산
- delegate broadcast, focus rule 재평가, slot enabled 갱신은 각 컴포넌트가 담당한다.

## Public Mutation API

- `UBeekeeperHotbarComponent`
  - `TryAcquireItem`
  - `TryAcquireItemBySpec`
  - `ApplySelectedItemStackDelta`
  - `ApplySelectedItemDurabilityDelta`
  - `SwapSlots`
  - `MovePartialToSlot`
- `UStorageBoxComponent`
  - `SwapStorageSlots`
  - `MoveHotbarItemToStorage`
  - `MoveStorageItemToHotbar`
  - `SwapHotbarAndStorage`
  - `MovePartialStorageToStorage`
  - `MovePartialStorageToHotbar`
  - `MovePartialHotbarToStorage`

이 API는 UI widget wrapper가 아니라 실제 상태 변경 경로다.

## Public Query API

- `UBeekeeperHotbarComponent`
  - `PreviewAcquireItemBySpec`

`PreviewAcquireItemBySpec(const FItemAcquireSpec&)`는 `TryAcquireItemBySpec`와 같은 stack/empty-slot/durability compatibility 규칙을 사용하지만, slot mutation, item instance 생성, delegate broadcast를 수행하지 않는 dry-run query다. Focus prompt availability와 실제 acquire 실행은 같은 acquire 계산 규칙을 공유해야 한다.

## Dependencies

- Focus
- UI
- Character

## Completed Refactoring Notes

- `ItemStackMoveUtils`로 stack 계산 중복을 통합했다.
- `UStorageBoxWidget`의 이동/스왑 wrapper API는 삭제했다.
- Drag/drop container enum은 `EItemSlotContainerType`으로 rename되었다.
- `EItemSlotDragMode`, `FItemSlotMoveResult`는 유지했다.

## Design Notes

- Inventory system은 슬롯 상태 변경의 최종 authority다.
- UI는 drop/quick move 의도를 전달하고, 실제 mutation은 Hotbar/Storage 컴포넌트가 수행한다.
- 현재 quick move 대상 슬롯 선택은 `UItemSlotWidget::TryQuickMove()`에 남아 있다. 규칙이 늘어나면 `Inventory` private helper 또는 service로 이동하는 것이 다음 개선 후보다.
- `FItemSlotMoveResult`는 partial move 결과를 UI/Blueprint가 해석할 수 있는 공용 구조체다.
- FocusEngaged item-use-area 설계에서 실질 아이템사용효과의 owner는 item action이다.
- Focus의 `UCursorItemUseAreaScopeComponent`는 선택 item의 `FindHoldItemUseAction()` 결과를 cache하고 LMB Press/Hold/Release를 hold-use lifecycle로 라우팅한다.
- Hold-use item action은 use session 중 `TickUse(Context, DeltaTime)`와, 유효 area target 위에서 `ApplyUseEffect(Context, DeltaTime)` 형태의 지속 효과 호출을 지원한다.
- `USmokerUseAction`은 `Item.UseArea.Beehive.Smoker` tag query와 함께 hold-use 시 `ABeehive::DecreaseAggression`을 호출한다. durability drain 적용 여부는 source item definition이 `UActiveUseDurabilityItemDefinition`인지에 따라 결정된다.
- Item action은 사용 가능한 area tag query를 제공하고, Focus 쪽 item-use-area scope는 이를 `FItemUseAreaDescriptor::AreaTags`와 매칭한다.
- `FItemActionContext`는 `FocusEngagedHostActor`, `ItemUseAreaId`, `ItemUseAreaTags`, `ItemUseAreaHitComponent`, `ItemUseEffectTargetObject`, item-use-area impact point/normal을 포함해 효과 target context를 전달한다.
- 실제 효과 적용 빈도, 내구도 감소, 작업 진행도 누적 같은 rate limit은 item action 내부에서 관리한다.
- Instant click action, item stack 소비 정책, Blueprint override event hook은 아직 Inventory 정본에서 확정된 현재 계약이 아니다. 설계 확정 없이 Public API를 rename/delete하지 않는다.
- Focus prompt entry의 획득/회수 availability는 hotbar mutation API를 호출하지 않고 `PreviewAcquireItemBySpec` dry-run 결과를 사용한다.
- pickup, placed item retrieve, beehive comb retrieve 같은 경로는 prompt disabled 상태와 실제 실행 성공 조건이 어긋나지 않도록 dry-run/execute 양쪽에서 같은 `FItemAcquireSpec`을 구성해야 한다.

## Manual Review Points

- partial move 후 `BroadcastHotbarChanged()`와 `ReevaluateSlotsInternal()` 순서
- storage 변경 후 `BroadcastStorageChanged()` 호출 시점
- full move와 partial move의 stack merge/swap 동작 차이
- hotbar focus rule 적용 중 선택 슬롯 clear 정책
- `UItemInstance::RebuildActions()` 후 action outer와 transient lifetime이 item instance 기준으로 유지되는지 확인
- action result의 item 소비/stack delta를 실제 호출 경로가 해석하는지 확인
- hold-use action을 Blueprint에서 구현해야 하는 요구가 생기면 현재 BlueprintCallable virtual 계약으로 충분한지 먼저 확인

## Update 2026-05-27

- 배치 아이템 회수 성공 판정 규칙:
  - `UBeekeeperHotbarComponent::TryAcquireItemBySpec(FItemAcquireSpec)` 호출
  - `bSuccess == true` 이고 `AddedQuantity == 1`일 때만 회수 성공으로 본다.
- 회수 실패(공간 부족 포함) 시 월드 배치 actor와 slot 점유 상태는 유지한다.

## Update 2026-05-27 (PartFocus Retrieve)

- placed item 회수 실행 주체는 global focus action이 아니라 PartFocus secondary action component다.
- inventory mutation 규칙은 동일하게 hotbar authority API(`TryAcquireItemBySpec`)를 사용한다.

## Update 2026-05-27 (Hotbar Middle Click Toggle)

- `UBeekeeperHotbarComponent`가 runtime selection memory(`LastSelectedIndex`)를 소유한다. 기본값은 `0`이다.
- middle click 토글 API:
  - `ToggleSelectionFromLastSelectedSlot()`
  - 슬롯 선택 중이면 선택 해제 전 현재 index를 기억한다.
  - 미선택 상태면 `LastSelectedIndex` -> `0` -> 첫 enabled 슬롯 순서로 fallback 재선택한다.
- focus rule로 선택이 강제 해제될 때 마지막 선택 슬롯을 기억한다.
- 선택 아이템 사용/소모로 stack이 0이 되면 슬롯 item만 비우고 선택 index는 유지한다.
- FocusEngaged에서 hotbar slot input block 정책이 활성화된 경우 middle click 토글도 차단한다.

## Update 2026-05-27 (BeeBrush Use Action)

- `UBeeBrushUseAction`을 추가했다. (`UHoldItemUseAction` 기반)
- use-area query tag: `Item.UseArea.Beehive.BeeBrush`
- 효과 적용 정책:
  - target: `Context.ItemUseEffectTargetObject`의 `ABeehiveCombActor`
  - 현재 visible face의 target만 감소 (`ReduceVisibleFaceTargetBeeCountByAmount`)
  - visible face에 여왕벌이 붙어 있으면 `Context.FocusEngagedHostActor`의 `ABeehive::TryBrushQueenBeeFromCombVisibleFace`를 통해 다른 소비장으로 재부착을 시도
  - `ColonyBeeCount`는 변경하지 않음
  - item stack/durability 소비 없음 (`bConsumedItem=false` 유지)

## Update 2026-05-27 (ItemUseArea EffectTarget Policy)

- `Context.ItemUseEffectTargetObject`는 actor-level provider override가 아니라 `UItemUseAreaMeshComponent::EffectTargetPolicy`에서 결정된다.
  - `ComponentOwner`
  - `HostActor`
  - `ExplicitObject`
- item action은 context target을 cast해 도메인 mutation을 수행한다.
  - BeeBrush: `ABeehiveCombActor`
  - placement item use: `AItemPlacementSlotActor` (`IItemPlacementSlot`)

## Update 2026-05-28 (Beehive Comb Retrieve State Contract)

- `UItemInstance`에 소비장 회수 상태를 위한 최소 전용 상태를 추가했다.
  - `FBeehiveCombItemState` (`bHasState`, `HoneyAmount`, `HoneyRipeness`, `bIsFrontFaceVisible`)
  - `SetBeehiveCombState`, `SetBeehiveCombStateWithRipeness`, `ClearBeehiveCombState`, `HasBeehiveCombState`, `GetBeehiveCombState`
- `UBeekeeperHotbarComponent::TryAcquireItem` 결과에 마지막 변경 슬롯/인스턴스 정보를 추가했다.
  - `LastModifiedSlotIndex`
  - `LastModifiedItemInstance`
- 소비장 회수 경로는 hotbar acquire 성공 후(`AddedQuantity == 1`) 반환된 item instance에 comb state를 기록한다.
- invariant:
  - comb 상태 보존이 필요한 반환 item definition(`DA_HoneyComb` 등)은 `MaxStack == 1`이어야 한다. (`MaxStack > 1`은 회수 차단 대상)
- 상태 보존 범위:
  - 보존: 꿀 양(`CurrentHoney`), 꿀 숙성도(`CurrentHoneyRipeness`), visible face(front/back)
  - 회수 가능 조건: `TotalTargetBeeCount == 0` 및 queen 미부착(조건 판정은 WorldActors occupant hook에서 수행)

## Update 2026-05-31 (Placed Item Durability Remaining)

- `UItemDefinition`에 `FPlacedItemRemainingSpec`을 추가했다.
  - `bUseDurabilityAsPlacedRemaining`
  - `bClearOwningSlotWhenDepleted`
  - `VisualComponentClass` (`TSubclassOf<UActorComponent>`)
- 배치 잔량 수치는 durability를 재사용한다.
  - max: `UItemDefinition::MaxDurability`
  - current: `UItemInstance::Durability`
- hotbar acquire에 state-aware API를 추가했다.
  - `FItemAcquireSpec`
  - `TryAcquireItemBySpec(const FItemAcquireSpec&)`
  - 기존 `TryAcquireItem(UItemDefinition*, int32)`는 wrapper로 유지한다.
- durability 아이템 stack merge 규칙:
  - same definition + same durability(`0.0001f` tolerance)만 병합 가능
  - durability가 다르면 병합하지 않는다.
  - split/create 경로에서 새 instance는 source durability를 복사한다.
- 1차 범위 제외:
  - hotbar/storage UI 잔량 bar/overlay/tooltip 표시는 제외한다.

## Update 2026-05-31 (Pollen Patty Population Bonus)

- 화분떡 인구 가속효과 수치는 `UItemDefinition` 본체가 아니라 `UPollenPattyItemDefinition`에 둔다.
  - 필드: `EggLayingMultiplier` (권장 기본값 `1.2`)
- 일반 아이템 asset은 기존 `UItemDefinition`을 계속 사용한다.
- 화분떡 tier는 `UPollenPattyItemDefinition` asset을 여러 개 두고 multiplier만 다르게 설정한다.

## Update 2026-06-01 (Acquire Preview For Prompt Availability)

- focus prompt disabled 상태 판정을 위해 hotbar acquire dry-run query를 추가하는 설계를 확정했다.
- `PreviewAcquireItemBySpec`는 `TryAcquireItemBySpec`와 동일한 수용 가능성 규칙을 사용하되 hotbar 상태를 변경하지 않는다.
- pickup과 회수 prompt entry는 공간 부족 또는 stack compatibility 실패 시 entry를 제거하지 않고 `bEnabled=false`로 표시할 수 있다.

## Update 2026-06-02 (Active-Use Durability Drain)

- 신규 item definition subclass:
  - `UActiveUseDurabilityItemDefinition`
  - 필드: `DurabilityDrainPerSecond`, `DrainPolicy`, `bRemoveItemWhenDepleted`
  - `DrainPolicy`:
    - `WhenUseEffectSucceeded`: active use-area 위에서 `ApplyUseEffect`가 `bSucceeded=true`를 반환한 Tick에만 drain
    - `WhileOverValidUseArea`: active use-area 위에서 `CanApplyUseEffect`가 true이면 effect success와 무관하게 drain
    - `WhileUseSessionActive`: `BeginUse` 성공 후 LMB active use session 동안 use-area hover/target과 무관하게 drain
- action 결과 DTO 확장:
  - `FItemActionExecutionResult::DurabilityDelta`
  - 음수=소모, 양수=회복(현재 구현은 소모만 사용)
- hold-use base 정책:
  - `UHoldItemUseAction::ResolveActiveUseDurabilityDelta(...)`
  - active-use durability definition이면 `bUsesDurability=true`, `MaxDurability>0`, `MaxStack==1`, `DurabilityDrainPerSecond>0`, `CurrentDurability>0` 조건에서만 delta 반환
  - `DrainPolicy`가 durability 감소 조건을 결정하며 기본값은 `WhenUseEffectSucceeded`
  - default `ReceiveCanBeginUse`/`ReceiveCanApplyUseEffect`는 durability 0 또는 invalid config를 사용 불가로 판정
- hotbar authority API 추가:
  - `FHotbarItemDurabilityMutationResult`
  - `ApplySelectedItemDurabilityDelta(float DurabilityDelta, bool bRemoveWhenDepleted)`
  - durability 0 도달 + `bRemoveWhenDepleted=true`면 selected slot item 제거
  - stack count는 변경하지 않음
- 적용 범위:
  - 훈연기/소독약은 DataAsset이 `UActiveUseDurabilityItemDefinition` 기반으로 전환된 경우에만 active-use drain 적용
  - 벌솔은 기존 durability 소모 없음 정책 유지

## Update 2026-06-02 (Beehive Comb Honey Ripeness State)

- `FBeehiveCombItemState`에 `HoneyRipeness` 절대값을 추가했다.
- 기존 `SetBeehiveCombState(float HoneyAmount, bool bIsFrontFaceVisible)` 시그니처는 유지한다.
  - 기존 setter는 `HoneyRipeness=0.0f`로 state를 저장한다.
- 신규 `SetBeehiveCombStateWithRipeness(float HoneyAmount, float HoneyRipeness, bool bIsFrontFaceVisible)`를 추가했다.
- 소비장 회수/재배치 state 보존 범위는 `HoneyAmount`, `HoneyRipeness`, visible face다.
- 저장되는 `HoneyRipeness`는 material ratio가 아니라 `ABeehiveCombActor::CurrentHoneyRipeness` 절대값이다.

## Update 2026-06-08 (Comb Uncapping Use Action + Capping State)

- `UCombUncappingUseAction`을 추가했다. (`UHoldItemUseAction` 기반)
- use-area query tag: `Item.UseArea.UncappingTable.HoneyComb`
- 효과 적용 정책:
  - target: `Context.ItemUseEffectTargetObject`의 `ABeehiveCombActor`
  - hit: `Context.bHasItemUseAreaHit`와 `Context.ItemUseAreaImpactPoint`
  - 현재 visible face의 front/back capping use-area mesh만 유효하다.
  - `Context.FocusEngagedHostActor`가 `AUncappingTable`이고 comb slot이 PartFocus 잡기 상태가 아닐 때만 begin/apply가 유효하다.
  - `MinStampInterval`과 `MinStampDistanceCm`를 모두 만족할 때 brush stamp를 허용한다.
  - 실제 mask pixel이 하나 이상 `>0`에서 `0`으로 바뀐 tick에만 `FItemActionExecutionResult::bSucceeded=true`다.
  - 이미 제거된 영역을 문지른 no-op stamp는 `bSucceeded=false`다.
- 이번 범위에서 `UCombUncappingUseAction`은 `DurabilityDelta`를 설정하지 않는다. active-use durability drain은 밀도 도구 DataAsset에서 별도로 사용하지 않는 정책이다.
- `FBeehiveCombItemState`에 face별 capping mask 저장 필드를 추가했다.
  - `CappingMaskWidth`
  - `CappingMaskHeight`
  - `FrontWaxCappingMask`
  - `BackWaxCappingMask`
- 기존 `SetBeehiveCombState(...)`와 `SetBeehiveCombStateWithRipeness(...)`는 유지하며, mask가 비어 있는 state는 소비장 actor가 full mask fallback으로 처리한다.
- 새 `SetBeehiveCombStateWithCapping(...)`는 honey amount, ripeness, visible face와 capping mask를 함께 저장한다.
