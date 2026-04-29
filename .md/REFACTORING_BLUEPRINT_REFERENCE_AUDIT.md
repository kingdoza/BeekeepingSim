# Blueprint Reference Audit

## 상태

- 검사일: 2026-04-29
- 목적: 리팩토링 설계 중 C++ API 삭제/rename/이동 후보의 Blueprint 참조 위험 확인
- 소스 코드 수정: 없음
- 에셋 수정/저장: 없음

## 검사 방법

1. Unreal Editor commandlet + PythonScriptPlugin으로 Asset Registry를 조회했다.
   - 전체 `/Game` AssetData 수: 736
   - `/Script/BeekeepingSim` 의존 AssetData 수: 86
   - BeekeepingSim 네이티브 부모 AssetData 수: 22
   - 의존성 조회 오류: 0
   - 원본 결과: `.md/blueprint_native_dependency_audit.json`
2. `Content/**/*.uasset`, `Content/**/*.umap` 668개 파일을 대상으로 C++ 심볼명을 ASCII/UTF-16LE 바이트 검색했다.
   - 원본 결과: `.md/blueprint_symbol_scan.json`
3. 주요 Widget Blueprint는 생성 클래스 CDO를 별도 확인했다.
   - 원본 결과: `.md/blueprint_ref_diag2.json`

## 네이티브 부모 Blueprint

중복된 `_C` GeneratedClass 항목을 제외한 실제 Blueprint 패키지 기준:

- `/Game/Beehive/BP_Beehive` -> `/Script/BeekeepingSim.Beehive`
- `/Game/Beekeeper/BP_BeekeeperCharacter` -> `/Script/BeekeepingSim.BeekeeperCharacter`
- `/Game/Beekeeper/BP_BeekeeperController` -> `/Script/BeekeepingSim.BeekeeperController`
- `/Game/Items/BP/BP_Item_BeeBrush` -> `/Script/BeekeepingSim.ItemPresentationActor`
- `/Game/Items/BP/BP_Item_Knife` -> `/Script/BeekeepingSim.ItemPresentationActor`
- `/Game/Items/BP/BP_Item_Patty` -> `/Script/BeekeepingSim.ItemPresentationActor`
- `/Game/Items/BP/BP_Item_Sprayer` -> `/Script/BeekeepingSim.ItemPresentationActor`
- `/Game/Stuff/BP_StorageBox` -> `/Script/BeekeepingSim.StorageBox`
- `/Game/UI/WBP_ItemSlot` -> `/Script/BeekeepingSim.ItemSlotWidget`
- `/Game/UI/WBP_ItemVisual` -> `/Script/BeekeepingSim.ItemVisualWidget`
- `/Game/UI/WBP_StorageBox` -> `/Script/BeekeepingSim.StorageBoxWidget`

## QnA 영향 요약

### Q2. Blueprint 호환 API 삭제/유지 정책

Blueprint 직렬화 심볼 확인 결과:

- 참조 있음:
  - `InitializeSlotContext`: `Content/UI/WBP_Hotbar.uasset`, `Content/UI/WBP_StorageBox.uasset`
  - `ShouldHideItemVisualForCurrentDrag`: `Content/UI/WBP_ItemSlot.uasset`
  - `IsPartialDragPreviewActive`: `Content/UI/WBP_ItemSlot.uasset`
  - `GetPartialDragPreviewDisplayStackCount`: `Content/UI/WBP_ItemSlot.uasset`
  - `OnStorageWidgetInitialized`: `Content/UI/WBP_StorageBox.uasset`
- 참조 없음:
  - `GetDragPreviewDisplayStackCount`
  - `RefreshDragPreviewFromOperation`
  - `RefreshPartialDragPreviewFromOperation`
  - `InitializeStorageWidget`
  - `MoveHotbarItemToStorage`
  - `MoveStorageItemToHotbar`
  - `SwapStorageSlots`
  - `SwapHotbarAndStorage`

판단:

- `ShouldHideItemVisualForCurrentDrag`, `IsPartialDragPreviewActive`, `GetPartialDragPreviewDisplayStackCount`, `InitializeSlotContext`, `OnStorageWidgetInitialized`는 Blueprint 참조가 확인되므로 삭제/rename 금지로 보는 것이 안전하다.
- 참조가 없는 래퍼도 네이티브 부모 Widget Blueprint의 API 표면 일부이므로 즉시 삭제보다는 단계적 deprecate 또는 유지가 안전하다.

### Q3. `UItemDragVisualWidget` 처리

확인 결과:

- `UItemDragVisualWidget`을 네이티브 부모로 쓰는 Blueprint는 확인되지 않았다.
- `InitializeDragVisual`, `OnDragVisualInitialized` 심볼 참조도 확인되지 않았다.
- 단, `ItemDragVisualWidget` 심볼 문자열이 `Content/UI/WBP_ItemVisual.uasset` 안에 남아 있다.
- Asset Registry상 `WBP_ItemVisual`의 현재 네이티브 부모는 `/Script/BeekeepingSim.ItemVisualWidget`이다.

판단:

- 활성 부모 참조는 아닌 것으로 보이나, 패키지 내부에 직렬화된 잔존 심볼이 있으므로 즉시 삭제는 여전히 위험하다.
- 삭제 후보로 두려면 구현 전에 Editor에서 `WBP_ItemVisual` 열기/컴파일/저장 후 심볼 제거 여부를 다시 확인하거나 Core Redirect/수동 복구 계획이 필요하다.

### Q4. `UFocusTargetComponent::bClearFocusOnConfirm`

확인 결과:

- `FocusTargetComponent` 참조는 다음 에셋에서 확인된다.
  - `Content/Beehive/BP_Beehive.uasset`
  - `Content/Stuff/BP_StorageBox.uasset`
  - `Content/__ExternalActors__/Beekeeper/Lvl_BeekeeperTest/7/MB/E8JF54C51FGECC78SRLKT4.uasset`
  - `Content/__ExternalActors__/Beekeeper/Lvl_BeekeeperTest/2/ST/QSPTDUSKEIWMNDDA0XYW5Y.uasset`
- `bClearFocusOnConfirm`, `ClearFocusOnConfirm`, `ShouldClearFocusOnConfirm` 심볼 참조는 확인되지 않았다.

판단:

- 현재 Content 기준으로 해당 속성/함수의 Blueprint 참조는 없다.
- 컴포넌트 클래스 자체는 Blueprint/레벨 배치 참조가 있으므로 클래스 rename/delete는 금지해야 한다.

### Q6. 클래스명/파일명 rename 허용 범위

확인 결과:

- `EStorageSlotContainerType` 참조 있음:
  - `Content/UI/WBP_Hotbar.uasset`
  - `Content/UI/WBP_StorageBox.uasset`
- 참조 없음:
  - `StorageSlotDragDropOperation`
  - `EItemSlotDragMode`
  - `FItemSlotMoveResult`
  - `SetMoveQuantityClamped`
  - `AdjustMoveQuantity`
  - `InitializeMoveQuantity`
  - `OnMoveQuantityChanged`
  - `MoveQuantity`
  - `MaxMoveQuantity`

판단:

- `EStorageSlotContainerType` rename은 Blueprint 파손 위험이 확실하므로 Core Redirect와 Blueprint 재컴파일 계획 없이는 금지해야 한다.
- `StorageSlotDragDropOperation`, `EItemSlotDragMode`, `FItemSlotMoveResult`는 Content 심볼 참조가 확인되지 않아 Blueprint 측 rename 위험은 낮지만, C++ public API와 UHT 이름 변경 리스크는 별도로 남는다.

## 결론

- Blueprint 참조 여부를 이유로 반드시 유지해야 하는 항목:
  - `UItemSlotWidget`
  - `UItemVisualWidget`
  - `UStorageBoxWidget`
  - `EStorageSlotContainerType`
  - `InitializeSlotContext`
  - `ShouldHideItemVisualForCurrentDrag`
  - `IsPartialDragPreviewActive`
  - `GetPartialDragPreviewDisplayStackCount`
  - `OnStorageWidgetInitialized`
- Blueprint 참조가 확인되지 않아 삭제/rename 검토가 가능한 항목:
  - `bClearFocusOnConfirm`
  - `ShouldClearFocusOnConfirm`
  - `MoveHotbarItemToStorage`
  - `MoveStorageItemToHotbar`
  - `SwapStorageSlots`
  - `SwapHotbarAndStorage`
  - `StorageSlotDragDropOperation`
  - `EItemSlotDragMode`
  - `FItemSlotMoveResult`
- 삭제/rename 전에 추가 수동 확인이 필요한 항목:
  - `UItemDragVisualWidget`: 활성 부모 참조는 없지만 `WBP_ItemVisual.uasset`에 잔존 심볼이 있다.
