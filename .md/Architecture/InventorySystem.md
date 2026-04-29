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
- `Source/BeekeepingSim/Public/Inventory/ItemActionContext.h`
- `Source/BeekeepingSim/Public/Inventory/ItemActionTypes.h`
- `Source/BeekeepingSim/Public/Inventory/HotbarItemInterface.h`
- `Source/BeekeepingSim/Public/Inventory/HotbarPresentationTypes.h`
- `Source/BeekeepingSim/Public/Inventory/ItemPresentationActor.h`
- `Source/BeekeepingSim/Private/Inventory/ItemPresentationActor.cpp`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`

## Responsibilities

- hotbar/storage 슬롯 상태 오너십
- item definition/instance/action 런타임 모델 제공
- item stack 병합, 분할 이동, swap, acquire 결과 계산
- focus item rule을 hotbar enabled state로 반영
- held/on-cursor presentation actor의 기반 class 제공

## Key Classes

- `UBeekeeperHotbarComponent`: 8슬롯 player hotbar 상태 오너
- `UStorageBoxComponent`: storage 슬롯 상태 오너
- `UItemDefinition`: 정적 아이템 데이터 asset
- `UItemInstance`: 런타임 아이템 상태와 action 소유 객체
- `UItemAction`: 아이템 행동 베이스
- `AItemPresentationActor`: first-person held/on-cursor 표시 actor 베이스
- `ItemStackMoveUtils`: private stack 계산/생성 helper

## Item Model

- `UItemDefinition`은 표시명, 설명, 아이콘, `WorldMesh`, `HeldPresentationActorClass`, gameplay tag, max stack, durability 설정, action spec을 가진다.
- `UItemInstance`는 definition, stack count, durability, instance id, action instance를 가진다.
- `UItemInstance`는 `IHotbarItemInterface`를 구현해 focus item rule 평가에 필요한 tag를 제공한다.
- action 객체의 outer는 `UItemInstance`다.

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

## Manual Review Points

- partial move 후 `BroadcastHotbarChanged()`와 `ReevaluateSlotsInternal()` 순서
- storage 변경 후 `BroadcastStorageChanged()` 호출 시점
- full move와 partial move의 stack merge/swap 동작 차이
- hotbar focus rule 적용 중 선택 슬롯 clear 정책
