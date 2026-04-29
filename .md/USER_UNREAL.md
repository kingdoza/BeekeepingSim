# Unreal Editor 사용자 작업 메모

## 목적

이 문서는 후속 리팩토링에서 변경/삭제될 수 있는 C++ API 때문에 사용자가 Unreal Editor에서 직접 확인하거나 처리해야 할 작업만 정리한다.

후속 구현 에이전트가 C++ 삭제, rename, Core Redirect, Blueprint API 정리를 진행할 때 이 문서를 기준으로 에디터 확인을 수행한다.

## 기본 원칙

- Compile 실패 상태의 Blueprint는 저장하지 않는다.
- C++ API 삭제/rename 후에는 관련 Blueprint를 열어 `Refresh All Nodes`, `Compile`, `Save` 순서로 확인한다.
- Core Redirect가 들어간 rename 작업 후에는 Editor를 재시작한 뒤 Blueprint를 다시 연다.
- 알 수 없는 노드, 깨진 enum pin, missing class/property 경고가 나오면 임의로 고치지 말고 구현 에이전트에게 그대로 전달한다.
- 관련 없는 Blueprint 그래프 구조나 에셋 설정은 수정하지 않는다.

## 공통 에디터 절차

1. 후속 구현 에이전트가 C++ 변경을 완료하고 빌드가 성공했다고 알려준 뒤 Editor를 연다.
2. 아래 대상 Blueprint를 하나씩 연다.
3. Blueprint 상단의 `Compile`을 누른다.
4. 오류가 없으면 `Save`한다.
5. 오류가 있으면 저장하지 말고 오류 메시지, 깨진 노드 이름, 해당 Blueprint 이름을 기록한다.
6. rename/Core Redirect 작업이 포함된 경우 Editor를 완전히 종료 후 재실행하고 다시 Compile/Save를 반복한다.

## 우선 확인 대상 Blueprint

- `Content/UI/WBP_ItemVisual`
- `Content/UI/WBP_ItemSlot`
- `Content/UI/WBP_Hotbar`
- `Content/UI/WBP_StorageBox`
- `Content/Beehive/BP_Beehive`
- `Content/Stuff/BP_StorageBox`
- `Content/Beekeeper/Lvl_BeekeeperTest`

`Lvl_BeekeeperTest`는 `FocusTargetComponent`가 배치 액터에 직렬화되어 있을 수 있으므로, Focus 관련 property 삭제 작업 후 한 번 열어 로드 경고가 없는지 확인한다.

## 1. `UItemDragVisualWidget` 삭제 전 확인

### 관련 API

- `UItemDragVisualWidget`
- `InitializeDragVisual`
- `OnDragVisualInitialized`

### 사용자가 할 일

1. `Content/UI/WBP_ItemVisual`을 연다.
2. `Class Settings`에서 Parent Class가 `ItemVisualWidget`인지 확인한다.
3. 그래프와 변수 목록에 `ItemDragVisualWidget`, `InitializeDragVisual`, `OnDragVisualInitialized` 관련 항목이 남아 있는지 확인한다.
4. `Compile`한다.
5. 오류가 없으면 `Save`한다.

### 문제 발생 시

- Parent Class가 `ItemDragVisualWidget`이거나 missing class로 표시되면 삭제를 진행하면 안 된다.
- `InitializeDragVisual` 또는 `OnDragVisualInitialized` 노드가 깨져 있으면 저장하지 말고 구현 에이전트에게 전달한다.

## 2. `UItemSlotWidget` API 변경/삭제 확인

### 삭제 또는 deprecate 후보

- `GetDragPreviewDisplayStackCount`
- `RefreshDragPreviewFromOperation`
- `RefreshPartialDragPreviewFromOperation`

### Blueprint 참조가 확인되었던 API

- `InitializeSlotContext`
- `ShouldHideItemVisualForCurrentDrag`
- `IsPartialDragPreviewActive`
- `GetPartialDragPreviewDisplayStackCount`

### 사용자가 할 일

1. `Content/UI/WBP_ItemSlot`을 연다.
2. `Refresh All Nodes`를 실행한다.
3. `Compile`한다.
4. `ShouldHideItemVisualForCurrentDrag`가 삭제 또는 대체되었다면, 구현 에이전트가 지정한 새 함수/로직으로 해당 노드를 교체한다.
5. `IsPartialDragPreviewActive`, `GetPartialDragPreviewDisplayStackCount` 노드가 정상인지 확인한다.
6. 오류가 없으면 `Save`한다.

### 문제 발생 시

- `ShouldHideItemVisualForCurrentDrag` 노드가 missing으로 표시되면 저장하지 말고 대체 API 이름을 구현 에이전트에게 확인한다.
- drag preview 표시가 이상하면 `WBP_ItemSlot`에서 full-stack drag 숨김 처리와 partial-stack count 표시가 분리되어 있는지 확인한다.

## 3. `UStorageBoxWidget` 래퍼 API 삭제 확인

### 삭제 또는 deprecate 후보

- `InitializeStorageWidget`
- `MoveHotbarItemToStorage`
- `MoveStorageItemToHotbar`
- `SwapStorageSlots`
- `SwapHotbarAndStorage`

### 유지 또는 별도 마이그레이션 대상

- `OnStorageWidgetInitialized`

### 사용자가 할 일

1. `Content/UI/WBP_StorageBox`를 연다.
2. 그래프에서 위 삭제 후보 함수 노드가 missing으로 표시되는지 확인한다.
3. `OnStorageWidgetInitialized` 이벤트가 정상인지 확인한다.
4. `Compile`한다.
5. 오류가 없으면 `Save`한다.

### 문제 발생 시

- `MoveHotbarItemToStorage`, `MoveStorageItemToHotbar`, `SwapStorageSlots`, `SwapHotbarAndStorage` 노드가 실제로 사용 중이면 구현 에이전트에게 전달한다.
- 해당 함수들은 Widget 래퍼였으므로, 대체 경로는 보통 drag/drop library 또는 Inventory 컴포넌트 API가 되어야 한다.

## 4. `bClearFocusOnConfirm` 삭제 또는 기능화 확인

### 관련 API

- `bClearFocusOnConfirm`
- `ShouldClearFocusOnConfirm`

### 사용자가 할 일

1. `Content/Beehive/BP_Beehive`를 연다.
2. `FocusTargetComponent`를 선택하고 Details 패널에 `Clear Focus On Confirm` 관련 값이 있는지 확인한다.
3. `Content/Stuff/BP_StorageBox`도 같은 방식으로 확인한다.
4. `Content/Beekeeper/Lvl_BeekeeperTest`를 열어 로드 경고가 없는지 확인한다.
5. 각 Blueprint를 `Compile`한다.
6. 오류가 없으면 `Save`한다.

### 문제 발생 시

- 삭제 후 missing property 경고가 나오면 저장하지 말고 구현 에이전트에게 전달한다.
- 값이 실제로 설정되어 있었거나 기획적으로 필요한 값이면 삭제 대신 Focus 확정 후 clear 동작 구현을 요청한다.

## 5. `EStorageSlotContainerType` rename 확인

### rename 후보

- `EStorageSlotContainerType` -> `EItemSlotContainerType`

### 영향 가능 Blueprint

- `Content/UI/WBP_Hotbar`
- `Content/UI/WBP_StorageBox`

### 사용자가 할 일

1. Core Redirect 적용 후 Editor를 재시작한다.
2. `WBP_Hotbar`를 연다.
3. enum pin 또는 변수 기본값이 `None`, `Hotbar`, `Storage` 중 올바른 값으로 유지되었는지 확인한다.
4. `Compile`하고 오류가 없으면 `Save`한다.
5. `WBP_StorageBox`도 같은 방식으로 확인한다.

### 문제 발생 시

- enum pin이 `Unknown`, `None`으로 잘못 바뀌었거나 연결이 끊어졌으면 저장하지 않는다.
- Core Redirect가 제대로 적용되지 않았을 가능성이 있으므로 구현 에이전트에게 전달한다.

## 6. `UStorageSlotDragDropOperation` rename 확인

### rename 후보

- `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation`
- `StorageSlotDragDropOperation.h/.cpp` -> `ItemSlotDragDropOperation.h/.cpp`
- `StorageSlotDragDropTypes.h` -> `ItemSlotDragDropTypes.h`

### 사용자가 할 일

1. rename과 Core Redirect 적용 후 Editor를 재시작한다.
2. `WBP_ItemSlot`을 연다.
3. Drag/Drop 관련 노드가 missing class로 표시되는지 확인한다.
4. `Compile`한다.
5. 오류가 없으면 `Save`한다.
6. `WBP_Hotbar`, `WBP_StorageBox`도 Compile/Save한다.

### 문제 발생 시

- drag/drop operation class가 missing으로 표시되면 저장하지 않는다.
- 드래그 시작, 드롭 처리, 부분 스택 수량 조절 노드가 깨졌는지 기록한다.

## 7. `FItemSlotMoveResult`, `EItemSlotDragMode` 변경 확인

### 관련 타입

- `FItemSlotMoveResult`
- `EItemSlotDragMode`

### 사용자가 할 일

1. 일반적으로 Blueprint 참조가 없어야 한다.
2. 그래도 UI Blueprint 전체를 Compile해서 누락 타입 오류가 없는지 확인한다.
3. 오류가 없으면 저장한다.

### 문제 발생 시

- 해당 타입 이름이 Blueprint 오류에 표시되면 구현 에이전트에게 전달한다.

## 8. 전체 UI Blueprint 최종 확인

API 삭제/rename 작업이 끝난 뒤 다음 순서로 한 번 더 확인한다.

1. `WBP_ItemVisual` Compile/Save
2. `WBP_ItemSlot` Compile/Save
3. `WBP_Hotbar` Compile/Save
4. `WBP_StorageBox` Compile/Save
5. Editor 재시작
6. 위 4개 Blueprint 다시 Compile
7. 오류가 없으면 Save All

## 9. 사용자 보고 양식

에디터 확인 후 구현 에이전트에게 아래 형식으로 전달한다.

```text
[Editor 확인 결과]

확인한 Blueprint:
- WBP_ItemVisual: 성공 / 실패
- WBP_ItemSlot: 성공 / 실패
- WBP_Hotbar: 성공 / 실패
- WBP_StorageBox: 성공 / 실패
- BP_Beehive: 성공 / 실패
- BP_StorageBox: 성공 / 실패
- Lvl_BeekeeperTest: 성공 / 실패

문제 발생 항목:
- Blueprint:
- 노드/속성/타입:
- 오류 메시지:
- 저장 여부:

추가 메모:
- 
```

## 저장하면 안 되는 경우

- Parent Class가 missing으로 표시된다.
- enum pin 값이 알 수 없는 값으로 바뀌었다.
- 삭제된 함수 노드가 missing으로 남아 있다.
- Compile 오류가 있다.
- 로드 시 missing property/class 경고가 나온다.
- 의도하지 않은 그래프 자동 변경이 발생했다.
