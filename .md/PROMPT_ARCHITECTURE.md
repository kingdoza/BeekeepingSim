# 재설계 프롬프트: ItemSlot Partial Drag Preview API 중복 정리

## 목적

현재 `UItemSlotWidget` / `UStorageSlotDragDropOperation` 에 partial drag preview 관련 API가 중복된 상태다.  
기존 동작을 깨지 않으면서 API 표면을 단일화하고, Blueprint 계약을 명확히 하는 재설계를 수행하라.

핵심 목표:

1. 중복 preview API를 하나의 표준 API 세트로 통합
2. wheel 수량 변경 시 drag visual + source slot preview 동기 갱신 유지
3. 실제 아이템 데이터(`UItemInstance::StackCount`)는 drag 중 절대 변경하지 않음
4. drop/cancel/cleanup 경로에서 preview 상태 누수 제거

---

## 현재 중복 상태(전제)

이미 코드에는 아래 류의 기능이 존재한다.

- `DragPreviewOriginalStackCount`, `DragPreviewMoveQuantity`, `DragPreviewMode`
- `ShouldHideItemVisualForCurrentDrag()`
- `GetDragPreviewDisplayStackCount()`
- `RefreshDragPreviewFromOperation(...)`
- `UStorageSlotDragDropOperation::SetMoveQuantityClamped()` 내부 source slot refresh 호출

동시에 새로 제안된 API는 아래 네이밍을 사용한다.

- `PartialDragPreviewOriginalStackCount`
- `PartialDragPreviewMoveQuantity`
- `bPartialDragPreviewActive`
- `IsPartialDragPreviewActive()`
- `ShouldHideItemVisualForPartialDragPreview()`
- `GetPartialDragPreviewDisplayStackCount()`
- `Set/Update/Clear/RefreshPartialDragPreview...`

이번 작업은 **둘 중 하나를 버리는 리네임 작업이 아니라**, 안전한 호환 계층까지 포함한 정리 작업이다.

---

## 작업 범위

- `Source/BeekeepingSim/Public/ItemSlotWidget.h`
- `Source/BeekeepingSim/Private/ItemSlotWidget.cpp`
- `Source/BeekeepingSim/Public/StorageSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/StorageSlotDragDropOperation.cpp`
- 필요 시 호출부(`BeekeeperController`, `StorageBoxFocusActionComponent`) 최소 수정

---

## 재설계 원칙

1. **Single Source of Truth**
   - preview 판단은 `UItemSlotWidget` 한 곳에서만 수행한다.
   - `UStorageSlotDragDropOperation` 은 수량 변경 이벤트/전파만 담당한다.

2. **Backward Compatibility**
   - 기존 Blueprint가 사용 중인 함수가 있으면 즉시 삭제하지 말고 alias 래퍼를 둔다.
   - 최소 1회차는 deprecated 주석으로 호환 유지.

3. **No Data Mutation During Drag**
   - preview는 전부 UI 상태 필드로만 표현.
   - 실제 stack 변경은 drop 성공 경로의 move API에서만 수행.

4. **Deterministic Cleanup**
   - drop/cancel/widget cleanup/endplay 어디서 끝나도 preview 상태는 반드시 초기화.

---

## 구현 지시

### 1) 표준 Preview API 확정

`UItemSlotWidget`의 표준 API를 아래로 고정한다.

- `IsPartialDragPreviewActive()`
- `ShouldHideItemVisualForPartialDragPreview()`
- `GetPartialDragPreviewDisplayStackCount()`
- `SetPartialDragPreviewState(...)`
- `UpdatePartialDragPreviewMoveQuantity(...)`
- `ClearPartialDragPreviewState()`
- `RefreshPartialDragPreviewFromOperation(...)`

기존 `ShouldHideItemVisualForCurrentDrag`, `GetDragPreviewDisplayStackCount`, `RefreshDragPreviewFromOperation` 은:

- 내부에서 표준 API를 호출하는 래퍼로 유지하거나
- 사용처 전환 후 `UE_DEPRECATED` 처리

### 2) 상태 필드 단일화

`DragPreview*` 계열과 `PartialDragPreview*` 계열이 공존하지 않게 정리한다.

- 내부 저장 필드는 한 세트만 사용
- expose 네이밍은 표준 API 기준으로 통일
- `FullStack` 처리용 숨김 여부는 별도 bool(`bIsDragSource`) + drag mode로 판단

### 3) Operation 연동 축소

`UStorageSlotDragDropOperation` 은 아래만 담당:

- partial 모드에서만 수량 조절
- `MoveQuantity` 변경 시
  - drag visual 갱신
  - source slot에 preview refresh 콜백
  - quantity changed delegate broadcast

판단 로직(숨김/표시/남은 수량 계산)은 operation에 두지 않는다.

### 4) 연결 지점 정리

- RMB drag 시작:
  - `SetPartialDragPreviewState(OriginalStack, MoveQuantity)`
- wheel 변경:
  - `RefreshPartialDragPreviewFromOperation(Operation)` 또는 내부 콜백
- drag 종료(drop/cancel/cleanup):
  - `ClearPartialDragPreviewState()`

중복 호출은 허용하되, 함수 내부에서 no-op 방어로 flicker/불필요 refresh 최소화.

### 5) Blueprint 계약 확정

`WBP_ItemSlot`은 아래 순서만 사용하도록 계약 문서화:

1. `ShouldHideItemVisualForPartialDragPreview()`
2. `IsPartialDragPreviewActive()`
3. 일반 표시 로직

Blueprint가 legacy 함수를 쓰고 있다면 새 API로 전환하고, 전환 완료 시 legacy 호출 제거.

---

## 수용 기준 (Acceptance)

- 동일 기능을 하는 preview API가 중복으로 남지 않는다.
- RMB drag 중 `MoveQuantity` 변경 시
  - drag visual 수량 즉시 반영
  - source slot 남은 수량 즉시 반영
- `MoveQuantity >= OriginalStack` 이면 source slot visual 숨김 판단이 일관된다.
- drag 종료 후 preview active 상태가 항상 false로 복귀한다.
- drag 중 실제 `UItemInstance::StackCount` 변경이 없다.
- UBT 빌드 통과.

---

## 리뷰 체크리스트

1. API 중복 제거가 실제로 되었는가 (이름만 추가하고 로직 중복 유지 금지)
2. cleanup 경로(drop/cancel/endplay/storage cleanup) 누락이 없는가
3. 기존 Blueprint 깨짐 없이 마이그레이션 가능한가
4. partial 전용 로직이 full stack 경로를 오염시키지 않는가
