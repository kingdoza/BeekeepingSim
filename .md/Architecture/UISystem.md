# UI System

## Scope

- `Source/BeekeepingSim/Public/UI/StorageBoxWidget.h`
- `Source/BeekeepingSim/Private/UI/StorageBoxWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemVisualWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemVisualWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemDragVisualWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemDragVisualWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropLibrary.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotDragDropLibrary.cpp`
- `Source/BeekeepingSim/Public/UI/StorageSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/UI/StorageSlotDragDropOperation.cpp`
- `Source/BeekeepingSim/Public/UI/StorageSlotDragDropTypes.h`

## Responsibilities

- storage/hotbar 슬롯 UI 컨텍스트 및 drag/drop 상태 유지
- full/partial stack drag 라우팅
- drag visual/slot visual 표시 갱신
- Blueprint 위젯 계층에서 사용할 C++ API 표면 제공

## Key Classes

- `UStorageBoxWidget`: UI 루트 및 이동/교환 wrapper API
- `UItemVisualWidget`: 공용 아이템 표시 base
- `UItemDragVisualWidget`: drag 전용 표시 base(유지 대상)
- `UItemSlotWidget`: 슬롯 입력/드래그 시작/quick move 진입점
- `UItemSlotDragDropLibrary`: drag/drop 중립 라우터
- `UStorageSlotDragDropOperation`: drag payload
- `EStorageSlotContainerType`, `EItemSlotDragMode`, `FItemSlotMoveResult`: 공용 타입

## Dependencies

- Inventory
- Focus
- Character

## Refactoring Notes

- Content 미참조 public compatibility API는 유지
  - `InitializeSlotContext`
  - `ShouldHideItemVisualForCurrentDrag`
  - `IsPartialDragPreviewActive`
  - `GetPartialDragPreviewDisplayStackCount`
  - `OnStorageWidgetInitialized`
- `UItemDragVisualWidget` 유지(잔존 심볼 이슈)
- rename 미적용:
  - `StorageSlotDragDropOperation`/`StorageSlotDragDropTypes`는 향후 후보로만 기록

## Manual Review Points

- partial drag 수량 조절(휠)과 미리보기 표시 일치 여부
- drag cancel/drop 완료 시 source visual 복구 및 active operation 해제
- Content 미참조 compatibility API 유지 상태 점검
