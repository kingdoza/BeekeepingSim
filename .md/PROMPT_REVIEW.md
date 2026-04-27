# 코드 리뷰 요청 프롬프트

아래 변경사항을 Unreal C++ 관점에서 리뷰하라.

## 변경 파일

- `Source/BeekeepingSim/Public/ItemSlotDragDropLibrary.h` (신규)
- `Source/BeekeepingSim/Private/ItemSlotDragDropLibrary.cpp` (신규)
- `Source/BeekeepingSim/Public/StorageSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/StorageSlotDragDropOperation.cpp`
- `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Public/StorageBoxWidget.h`
- `Source/BeekeepingSim/Private/StorageBoxWidget.cpp`
- `.md/0_ARCHITECTURE.md`

## 변경 목적

- Hotbar UI 와 Storage UI 간 drag/drop 라우팅 의존성을 제거한다.
- `UStorageBoxWidget` 중심 라우팅을 제거하고, widget-중립 `UItemSlotDragDropLibrary` 라우팅으로 전환한다.
- `UStorageSlotDragDropOperation` 에 source component 참조를 추가해 slot widget 이 source/target component 기반으로 드롭을 처리하도록 한다.

## 핵심 로직

1. Drag operation 확장
- `UStorageSlotDragDropOperation`:
  - `SourceType`, `SourceIndex`
  - `SourceHotbarComponent`, `SourceStorageComponent`
  - 선택적 `ItemInstance`

2. 중립 라우터 추가
- `UItemSlotDragDropLibrary::HandleItemSlotDrop(...)`:
  - Hotbar -> Hotbar: 같은 hotbar component일 때 `SwapSlots()`
  - Hotbar -> Storage: `MoveHotbarItemToStorage()`
  - Storage -> Hotbar: `MoveStorageItemToHotbar()`
  - Storage -> Storage: 같은 storage component일 때 `SwapStorageSlots()`
  - cross-component 동일 컨테이너 이동(다른 hotbar/storage) 및 미지원 조합은 false

3. StorageBoxWidget 책임 축소
- 제거:
  - `HandleSlotDrop()`
  - `SwapHotbarSlots()`
- 유지:
  - `MoveHotbarItemToStorage()`
  - `MoveStorageItemToHotbar()`
  - `SwapStorageSlots()`
  - `SwapHotbarAndStorage()`

## 리뷰 집중 포인트

- `HandleItemSlotDrop()` 가 요구된 source/target 조합을 모두 처리하는지.
- 같은 컨테이너 타입에서 source/target component 동일성 검증이 정확한지.
- `UStorageSlotDragDropOperation` 의 component 참조가 null일 때 실패 경로가 안전한지.
- `UStorageBoxWidget` 에 라우팅 책임이 완전히 제거되었는지(잔존 호출 포함).
- hotbar/storage 변경 이벤트(`OnHotbarChanged`, `OnStorageChanged`)가 기존 component API 경로에서 유지되는지.
- Blueprint 연동 시 slot widget 이 `UStorageBoxWidget` 참조 없이도 라우팅 가능한 API 표면을 갖추었는지.
