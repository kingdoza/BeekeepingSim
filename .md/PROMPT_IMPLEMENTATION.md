# 구현 프롬프트: ItemSlot Context 정리 및 Drag Preview 보강

## 작업 범위

이 문서는 현재 발견된 ItemSlot 문제를 해결하기 위한 신규 구현 지시만 담는다.

기존에 완료된 구현 회차, 과거 drag/drop 라우터 구현, 기존 partial move 구현 상태 설명은 이 문서 범위에서 제외한다.

이번 작업의 목표:

1. `UItemSlotWidget` 이 `HotbarComponent`, `StorageComponent` 를 직접 보관하지 않도록 정리한다.
2. storage -> hotbar quick move 가 controller context 기반으로 동작하게 한다.
3. LMB drag 중 source slot item visual 이 사라지게 한다.
4. RMB drag 중 wheel 로 이동 수량을 조절하고, drag visual/source slot preview 를 즉시 갱신한다.

## 반영할 QNA 결정

관련 QNA 만 반영한다.

- 질문 5: RMB 부분 이동 중 mouse wheel 입력은 PlayerController 또는 HUD 계층에서 active drag operation 을 보관해 처리한다.
- 질문 11: RMB drag 중 source slot stack count 감소는 UI preview 로만 처리한다.
  - 실제 `UItemInstance::StackCount` 는 drag 중 변경하지 않는다.
  - 실제 데이터 변경은 drop 성공 시점에만 수행한다.
  - drop 실패/cancel/ESC/widget cleanup 시 rollback 이 필요 없도록 한다.
- 질문 12: `UItemSlotWidget` 은 container component 를 직접 보관하지 않는다.
  - `UItemSlotWidget` 은 `ContainerType`, `SlotIndex` 만 보관한다.
  - hotbar 는 controller/player context 에서 resolve 한다.
  - storage 는 controller active storage context 에서 resolve 한다.

## 현재 확인된 문제

### 1. LMB source slot visual 숨김

현재 C++에는 drag 시작 시 `SetDragSourceVisualHidden(true)` 흐름이 있다.

하지만 `RefreshVisual()` 은 `OnSlotVisualStateChanged()` 만 호출하므로 실제 아이콘/수량 숨김은 Blueprint 구현 또는 추가 getter/API 계약에 의존한다.

필요 작업:

- C++에서 source slot visual 상태를 Blueprint 가 안정적으로 읽을 수 있게 getter/API 를 제공한다.
- Blueprint 구현에서 이 상태를 사용해 내부 `ItemVisualWidget` 을 숨기거나 비운다.

### 2. storage -> hotbar quick move 실패

현재 구현은 storage slot quick move 에서 `StorageComponent` 와 `HotbarComponent` 직접 보관값에 의존한다.

개선 방향:

- `UItemSlotWidget` 은 `HotbarComponent`, `StorageComponent` 를 직접 보관하지 않는다.
- hotbar 는 controller/player context 에서 resolve 한다.
- storage 는 controller active storage context 에서 resolve 한다.
- storage slot 은 `InitializeSlotContext(Storage, Index)` 만으로 quick move 가 동작해야 한다.

### 3. RMB drag quantity preview 미완성

현재 `MoveQuantity`, `MaxMoveQuantity`, `AdjustMoveQuantity()` 류의 기반은 있으나 다음이 부족하다.

- wheel 로 `MoveQuantity` 변경 후 drag visual StackCount 즉시 갱신
- source slot 에 남은 수량 preview 즉시 반영
- `MoveQuantity == OriginalStackCount` 일 때 source slot visual 숨김

## 구현 지시

### 1. Controller hotbar/storage context provider 정리

`ABeekeeperController` 에 아래 API 가 없으면 추가한다.

```cpp
UFUNCTION(BlueprintPure, Category = "Hotbar")
UBeekeeperHotbarComponent* GetPlayerHotbarComponent() const;

UFUNCTION(BlueprintPure, Category = "Storage")
UStorageBoxComponent* GetActiveStorageComponent() const;
```

`GetActiveStorageComponent()` 는 이미 있으면 유지한다.

`GetPlayerHotbarComponent()` 동작:

- `GetPawn()` 을 `ABeekeeperCharacter` 로 cast 한다.
- 캐릭터에서 `UBeekeeperHotbarComponent` 를 얻어 반환한다.
- 필요하면 `ABeekeeperCharacter` 에 아래 getter 를 추가한다.

```cpp
UFUNCTION(BlueprintPure, Category = "Hotbar")
UBeekeeperHotbarComponent* GetBeekeeperHotbarComponent() const;
```

주의:

- `ActiveStorageComponent` 는 `UStorageBoxFocusActionComponent` 가 focus begin/cleanup 에서 최신화하는 기존 흐름을 사용한다.
- cleanup 시 active storage 가 현재 storage 와 같을 때만 clear 하는 방어는 유지한다.

### 2. `UItemSlotWidget` container component 직접 보관 제거

`UItemSlotWidget` 에서 제거한다.

- `TObjectPtr<UBeekeeperHotbarComponent> HotbarComponent`
- `TObjectPtr<UStorageBoxComponent> StorageComponent`
- `InitializeSlotContext()` 의 `UBeekeeperHotbarComponent* InHotbarComponent` 매개변수
- `InitializeSlotContext()` 의 `UStorageBoxComponent* InStorageComponent` 매개변수
- HotbarComponent/StorageComponent 직접 getter/setter 또는 Blueprint 노출 핀
- `TryQuickMove()`, `RefreshFromData()`, `NativeOnDragDetected()`, `NativeOnDrop()` 내부의 component 직접 참조

변경 후 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Item Slot")
void InitializeSlotContext(EStorageSlotContainerType InContainerType, int32 InSlotIndex);
```

`UItemSlotWidget` 이 보관할 context:

```cpp
EStorageSlotContainerType ContainerType;
int32 SlotIndex;
```

필요한 helper:

```cpp
UBeekeeperHotbarComponent* ResolveHotbarComponentForSlot() const;
UStorageBoxComponent* ResolveStorageComponentForSlot() const;
```

`ResolveHotbarComponentForSlot()`:

- `GetOwningPlayer()` 를 `ABeekeeperController` 로 cast
- `GetPlayerHotbarComponent()` 반환
- 실패 시 null

`ResolveStorageComponentForSlot()`:

- `GetOwningPlayer()` 를 `ABeekeeperController` 로 cast
- `GetActiveStorageComponent()` 반환
- 실패 시 null

적용 지점:

- `RefreshFromData()`
  - hotbar slot: resolved hotbar 에서 `SlotIndex` 조회
  - storage slot: resolved active storage 에서 `SlotIndex` 조회
- `NativeOnDragDetected()`
  - hotbar source: operation `SourceHotbarComponent = ResolveHotbarComponentForSlot()`
  - storage source: operation `SourceStorageComponent = ResolveStorageComponentForSlot()`
- `NativeOnDrop()`
  - hotbar target: target hotbar = resolved hotbar
  - storage target: target storage = resolved active storage
- `TryQuickMove()`
  - hotbar -> storage: source hotbar = resolved hotbar, target storage = active storage
  - storage -> hotbar: source storage = active storage, target hotbar = resolved hotbar

실패 처리:

- resolved hotbar/storage 가 null 이면 false 반환
- crash 하지 않는다.
- quick move 실패 원인을 `UE_LOG` 로 구분할 수 있게 한다.

### 3. `UStorageSlotDragDropOperation` quantity change 알림

`MoveQuantity` 변경 시 drag visual 과 source slot preview 를 갱신할 수 있게 한다.

추가 권장:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemDragMoveQuantityChangedSignature, int32, NewMoveQuantity);

UPROPERTY(BlueprintAssignable, Category = "Storage Drag Drop")
FItemDragMoveQuantityChangedSignature OnMoveQuantityChanged;

UPROPERTY(Transient, BlueprintReadWrite, Category = "Storage Drag Drop")
TObjectPtr<UItemSlotWidget> SourceSlotWidget;
```

`SetMoveQuantityClamped()` 수정:

- 기존 값과 새 값이 같으면 불필요한 갱신을 하지 않는다.
- clamp 된 값이 변경되면 `MoveQuantity` 를 갱신한다.
- `DragVisualWidget` 이 유효하면:

```cpp
DragVisualWidget->SetItemVisualData(ItemInstance, MoveQuantity);
```

- `SourceSlotWidget` 이 유효하면 source preview 갱신을 호출한다.
- `OnMoveQuantityChanged.Broadcast(MoveQuantity)` 를 호출한다.

주의:

- `PartialStack` drag 에서만 wheel quantity 변경을 허용한다.
- `FullStack` drag 는 wheel 로 수량 변경하지 않는다.

### 4. `ABeekeeperController::AdjustActiveItemSlotDragQuantity()` 보강

수정 요구:

- active operation 이 없으면 false
- active operation 이 `PartialStack` 이 아니면 false
- wheel delta 가 0 이면 false
- wheel up/down 을 `+1/-1` 로 변환
- `MoveQuantity` 변경 전후가 같으면 false 또는 no-op
- 변경되면 drag visual StackCount 와 source slot preview 가 즉시 갱신되어야 한다.

### 5. `UItemSlotWidget` drag preview 상태 추가

source slot 에 남은 수량을 preview 하기 위한 상태를 추가한다.

권장 필드:

```cpp
UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot")
int32 DragPreviewMoveQuantity = 0;

UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot")
int32 DragPreviewOriginalStackCount = 0;

UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot")
EItemSlotDragMode DragPreviewMode = EItemSlotDragMode::FullStack;
```

권장 API:

```cpp
UFUNCTION(BlueprintPure, Category = "Item Slot")
bool ShouldHideItemVisualForCurrentDrag() const;

UFUNCTION(BlueprintPure, Category = "Item Slot")
int32 GetDragPreviewDisplayStackCount() const;

UFUNCTION(BlueprintCallable, Category = "Item Slot")
void RefreshDragPreviewFromOperation(UStorageSlotDragDropOperation* Operation);
```

동작:

- LMB `FullStack` drag:
  - source slot item visual 은 완전히 숨긴다.
- RMB `PartialStack` drag:
  - `MoveQuantity < OriginalStackCount` 이면 source slot item visual 은 유지하고 stack count 는 `OriginalStackCount - MoveQuantity` 로 preview 한다.
  - `MoveQuantity >= OriginalStackCount` 이면 source slot item visual 을 완전히 숨긴다.
- drag 종료/drop/cancel:
  - preview 상태 초기화
  - 실제 데이터 기준으로 `RefreshFromData()` 또는 `RefreshVisual()` 호출

중요:

- drag 중 실제 `UItemInstance::StackCount` 를 변경하지 않는다.
- 실제 stack 변경은 drop 성공 시 기존 move API 에서만 수행한다.

### 6. `UItemSlotWidget::NativeOnDragDetected()` 보강

drag operation 생성 시:

- `DragOperation->SourceSlotWidget = this`
- source preview 초기화:

```cpp
DragPreviewOriginalStackCount = ItemInstance ? ItemInstance->GetStackCount() : 0;
DragPreviewMoveQuantity = DragOperation->MoveQuantity;
DragPreviewMode = DragOperation->DragMode;
```

- drag visual 생성 후:
  - LMB/full stack: drag visual 은 실제 stack count 표시 또는 override 없음
  - RMB/partial stack: drag visual 은 `MoveQuantity` override 로 표시
- source slot visual 상태 갱신:
  - LMB: 숨김
  - RMB: 남은 수량 preview 또는 전체 선택 시 숨김

### 7. `RefreshVisual()` / Blueprint 계약

`RefreshVisual()` 은 Blueprint 가 충분한 상태를 읽을 수 있게 해야 한다.

Blueprint 에서 읽을 값:

- `ItemInstance`
- `bIsSelected`
- `bIsActivated`
- `bIsDragSource`
- `ShouldHideItemVisualForCurrentDrag()`
- `GetDragPreviewDisplayStackCount()`

필요하면 getter 를 추가한다.

Blueprint 구현 규칙:

- `ShouldHideItemVisualForCurrentDrag() == true`
  - 내부 `WBP_ItemVisual` 을 숨기거나 `ClearItemVisualData()` 호출
- RMB partial drag 이고 숨김이 아니면
  - `WBP_ItemVisual.SetItemVisualData(ItemInstance, GetDragPreviewDisplayStackCount())`
- 일반 상태면
  - item 있음: `WBP_ItemVisual.SetItemVisualData(ItemInstance)`
  - item 없음: `WBP_ItemVisual.ClearItemVisualData()`

## Blueprint 수정 요구사항

### `WBP_ItemSlot`

기존 `InitializeSlotContext` 호출을 새 시그니처로 교체한다.

Hotbar slot:

```text
InitializeSlotContext(Hotbar, Index)
```

Storage slot:

```text
InitializeSlotContext(Storage, Index)
```

삭제할 것:

- `HotbarComponent` 변수
- `StorageComponent` 변수
- slot 단위 component 주입 로직

유지할 것:

- `ContainerType`
- `SlotIndex`
- 내부 `WBP_ItemVisual`
- `OnSlotVisualStateChanged` visual 갱신 구현

### `WBP_ItemVisual`

`OnItemVisualDataChanged` 에서 `GetDisplayStackCount()` 를 읽어 StackCount Text 를 갱신한다.

RMB drag 중에는 C++에서 `SetItemVisualData(ItemInstance, MoveQuantity)` 또는 source preview count 를 넘겨주므로, Blueprint 는 `GetDisplayStackCount()` 만 반영하면 된다.

## 검증 항목

- `UItemSlotWidget` 에 `HotbarComponent`, `StorageComponent` 멤버가 남아 있지 않다.
- `InitializeSlotContext()` 는 `ContainerType`, `SlotIndex` 만 받는다.
- storage slot 은 `InitializeSlotContext(Storage, Index)` 만으로 refresh/drop/quick move 가 동작한다.
- hotbar slot 은 `InitializeSlotContext(Hotbar, Index)` 만으로 refresh/drop/quick move 가 동작한다.
- LMB drag 시작 시 source slot item visual 이 보이지 않는다.
- LMB drag cancel/drop 후 source slot visual 이 실제 데이터 기준으로 복구된다.
- storage -> hotbar quick move 가 동작한다.
- RMB drag 시작 시 drag visual StackCount 가 1 로 표시된다.
- RMB drag 중 wheel up/down 으로 `MoveQuantity` 가 1..MaxMoveQuantity 범위에서 변경된다.
- `MoveQuantity` 변경 즉시 drag visual StackCount Text 가 바뀐다.
- RMB drag 중 source slot 은 `OriginalStackCount - MoveQuantity` 를 표시한다.
- RMB drag 수량이 전체 수량과 같아지면 source slot item visual 이 보이지 않는다.
- RMB drag cancel/drop 후 source slot visual 이 실제 데이터 기준으로 복구된다.
