# UI System

## Scope

- `Source/BeekeepingSim/Public/UI/StorageBoxWidget.h`
- `Source/BeekeepingSim/Private/UI/StorageBoxWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemVisualWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemVisualWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropLibrary.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotDragDropLibrary.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotDragDropOperation.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropTypes.h`

## Responsibilities

- hotbar/storage slot widget context와 표시 상태 유지
- full-stack/partial-stack drag payload 생성
- drop target에 따른 inventory mutation API 라우팅
- partial drag source preview와 drag visual 수량 갱신
- Blueprint Widget이 사용할 최소 C++ API 표면 제공

## Key Classes

- `UStorageBoxWidget`: storage UI root, storage/hotbar context 초기화 진입점
- `UItemVisualWidget`: item icon/name/count 표시 base widget
- `UItemSlotWidget`: slot context, mouse input, drag 시작/drop, quick move 진입점
- `UItemSlotDragDropOperation`: drag payload와 partial move 수량 상태
- `UItemSlotDragDropLibrary`: source/target container 조합별 drop routing
- `EItemSlotContainerType`: `None`, `Hotbar`, `Storage` container 구분
- `EItemSlotDragMode`: full stack / partial stack drag 구분
- `FItemSlotMoveResult`: partial move 결과

## Drag/Drop Flow

1. `UItemSlotWidget::NativeOnMouseButtonDown()`이 left/right drag 버튼을 기록한다.
2. `NativeOnDragDetected()`가 `UItemSlotDragDropOperation`을 생성한다.
3. Left drag는 full stack, right drag는 partial stack으로 설정한다.
4. Partial drag는 source slot preview state와 drag visual count를 함께 갱신한다.
5. Drop 시 `UItemSlotDragDropLibrary::HandleItemSlotDrop()`이 source/target container 조합을 판정한다.
6. 실제 상태 변경은 `UBeekeeperHotbarComponent` 또는 `UStorageBoxComponent`가 수행한다.
7. Drop/cancel 후 source slot은 drag state와 active controller operation을 정리한다.

## Blueprint/API Contracts

현재 Blueprint 참조가 확인된 API:

- `UItemSlotWidget::InitializeSlotContext`
- `UItemSlotWidget::ShouldHideItemVisualForCurrentDrag`
- `UItemSlotWidget::IsPartialDragPreviewActive`
- `UItemSlotWidget::GetPartialDragPreviewDisplayStackCount`
- `UStorageBoxWidget::OnStorageWidgetInitialized`

`ShouldHideItemVisualForCurrentDrag`는 legacy wrapper다. 새 Blueprint 로직은 가능하면 `ShouldHideItemVisualForPartialDragPreview`, `IsPartialDragPreviewActive`, `GetPartialDragPreviewDisplayStackCount` 조합을 우선 사용한다.

## C++-Only API

- `UStorageBoxWidget::InitializeStorageWidget`
- `UItemSlotWidget::RefreshPartialDragPreviewFromOperation`
- `UItemSlotWidget::RefreshDragPreviewFromOperation`

위 함수들은 현재 C++ 내부 연결을 위해 유지되며 BlueprintCallable이 아니다.

## Completed Refactoring Notes

- `UItemDragVisualWidget` 삭제
- `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation`
- `EStorageSlotContainerType` -> `EItemSlotContainerType`
- `StorageSlotDragDropTypes.h` -> `ItemSlotDragDropTypes.h`
- `UStorageBoxWidget`의 move/swap wrapper API 삭제
- `UItemSlotWidget::GetDragPreviewDisplayStackCount()` 삭제
- Core Redirect와 Blueprint compile/save 완료 상태로 기록됨

## Dependencies

- Inventory
- Character

## Design Notes

- UI는 domain mutation을 직접 구현하지 않는다. Widget은 context를 resolve하고 Inventory API에 의도를 전달한다.
- `UItemSlotDragDropOperation`의 UFUNCTION Category에 남아 있는 "Storage Drag Drop" 표기는 에디터 표시용 legacy naming이며 시스템 경계를 의미하지 않는다.
- Quick move target selection은 현재 UI에 남아 있는 예외적 편의 로직이다. 규칙이 복잡해지면 Inventory 쪽으로 이동한다.
- Drag visual은 별도 `UItemDragVisualWidget` 없이 `UItemVisualWidget` 계층으로 통일한다.

## Manual Review Points

- partial drag 수량 조절 중 source slot preview count와 drag visual count 일치 여부
- full stack drag cancel/drop 후 source visual 복구 여부
- storage UI 종료 시 active drag operation과 active storage context 정리 여부
- Blueprint에서 legacy wrapper를 제거하려면 먼저 `WBP_ItemSlot` 그래프를 새 API로 migration해야 한다.
