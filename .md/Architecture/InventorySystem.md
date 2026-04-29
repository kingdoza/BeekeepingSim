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

- Hotbar/Storage 슬롯 상태 및 선택 상태 오너십
- 아이템 정의/인스턴스/액션 런타임 모델 제공
- partial/full 이동, swap, quick move 대상 선택 및 결과 계산
- 아이템 시각화 actor 구성 요소 제공

## Key Classes

- `UBeekeeperHotbarComponent`: 8슬롯 hotbar 상태 오너
- `UStorageBoxComponent`: storage 슬롯 상태 오너
- `UItemDefinition`: 정적 데이터
- `UItemInstance`: 런타임 데이터/내구도/액션 소유
- `UItemAction`: 액션 베이스
- `AItemPresentationActor`: held/on-cursor 시각화 actor
- `ItemStackMoveUtils`(private): 스택 계산/병합/생성 공통 helper

## Dependencies

- Focus
- UI
- Character

## Refactoring Notes

- 아래 public API 시그니처 유지:
  - `TryAcquireItem`
  - `MovePartialToSlot`
  - `MovePartialStorageToStorage`
  - `MovePartialStorageToHotbar`
  - `MovePartialHotbarToStorage`
  - `MoveHotbarItemToStorage`
  - `MoveStorageItemToHotbar`
  - `SwapHotbarAndStorage`
- helper는 계산/생성 공통화만 담당하고 delegate/재평가 호출 순서는 컴포넌트에 유지
- Hotbar 생성 outer=`UBeekeeperHotbarComponent`, Storage 생성 outer=`UStorageBoxComponent` 유지

## Manual Review Points

- partial 이동 후 `ReevaluateSlotsInternal()`/`BroadcastHotbarChanged()` 순서 유지
- storage 변경 후 `BroadcastStorageChanged()` 호출 시점 유지
- quick move 대상 슬롯 선택 규칙(merge 우선, 없으면 빈 슬롯) 유지
