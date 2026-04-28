# 코드 개선 요청 프롬프트

아래 통합 리뷰 결과를 기준으로 Unreal C++ 코드를 수정하라.

## 대상 파일

- `Source/BeekeepingSim/Private/StorageBoxComponent.cpp`
- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Private/StorageBoxFocusActionComponent.cpp`
- 필요 시 `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`

## 중요 문제 1: partial move 후 hotbar stack 수량 변경이 broadcast되지 않는 경로

### 문제
`MovePartialStorageToHotbar()`에서 기존 hotbar stack에 merge만 발생하면 `TargetItem->SetStackCount()`만 호출되고 `OnHotbarChanged`가 broadcast되지 않는다. `MovePartialHotbarToStorage()`에서도 source hotbar stack이 0이 되지 않으면 `ReevaluateSlots()`만 호출되며, slot enabled 변화가 없으면 broadcast되지 않는다.

### 영향
Hotbar UI가 partial 이동 후 stack count를 즉시 갱신하지 못한다. Quick move와 drag/drop 모두에서 UI stale 상태가 발생할 수 있다.

### 수정 방향
stack count 변경 자체도 hotbar 상태 변경으로 간주해 명시적으로 hotbar changed를 broadcast해야 한다. public API를 추가하거나 기존 setter 경로를 사용하되, 중복 broadcast를 최소화한다.

### 수정 예시
파일 경로: `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`

```cpp
UFUNCTION(BlueprintCallable, Category = "Hotbar")
void NotifyHotbarItemsChanged();
```

파일 경로: `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`

```cpp
void UBeekeeperHotbarComponent::NotifyHotbarItemsChanged()
{
	ReevaluateSlotsInternal();
	BroadcastHotbarChanged();
}
```

파일 경로: `Source/BeekeepingSim/Private/StorageBoxComponent.cpp`

```cpp
if (Result.MovedQuantity > 0)
{
	SourceItem->SetStackCount(SourceItem->GetStackCount() - Result.MovedQuantity);
	if (SourceItem->GetStackCount() <= 0)
	{
		Slots[StorageIndex].ItemInstance = nullptr;
	}

	HotbarComponent->NotifyHotbarItemsChanged();
	BroadcastStorageChanged();
	Result.bSuccess = true;
}
```

```cpp
if (Result.MovedQuantity > 0)
{
	SourceItem->SetStackCount(SourceItem->GetStackCount() - Result.MovedQuantity);
	if (SourceItem->GetStackCount() <= 0)
	{
		HotbarComponent->SetSlotItem(HotbarIndex, nullptr);
	}
	else
	{
		HotbarComponent->NotifyHotbarItemsChanged();
	}

	BroadcastStorageChanged();
	Result.bSuccess = true;
}
```

## 중요 문제 2: engaged 중 empty hotbar slot이 disabled 처리됨

### 문제
`IsSlotAllowedByActiveRule()`이 `AllowedItemTags.IsEmpty()`를 먼저 검사해 false를 반환하고, empty slot 검사도 false를 반환한다. `.md/0_ARCHITECTURE.md`는 engaged 중에도 빈 슬롯은 허용한다고 정의한다.

### 영향
Storage UI에서 empty hotbar slot이 disabled처럼 보일 수 있고, storage -> hotbar drop target UX가 깨질 수 있다.

### 수정 방향
유효 index와 engaged 여부 확인 후, 빈 슬롯은 항상 true로 반환한다. 그 다음 아이템 슬롯에 대해서만 allowed tag 필터를 적용한다.

### 수정 코드
파일 경로: `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`

```cpp
if (!bIsEngagedFocusActive)
{
	return true;
}

if (!Slots[Index].ItemInstance)
{
	return true;
}

const FGameplayTagContainer& AllowedItemTags = ActiveFocusRule.AllowedItemTags;
if (AllowedItemTags.IsEmpty())
{
	return false;
}
```

## 중요 문제 3: storage UI cleanup이 active drag operation을 정리하지 않음

### 문제
`UItemSlotWidget`은 drag operation drop/cancel delegate에서 source visual과 active drag operation을 정리한다. 하지만 StorageFocus cancel/abort/endplay로 widget tree가 제거되는 경우 `UStorageBoxFocusActionComponent::CleanupInteractionUI()`는 active storage만 정리하고 active drag operation은 정리하지 않는다.

### 영향
드래그 중 ESC/cancel 또는 storage actor EndPlay가 발생하면 controller에 stale drag operation이 남을 수 있다. 이후 wheel 입력이 hotbar 순환 대신 삭제된 drag operation 수량 조절로 소비될 수 있다.

### 수정 방향
storage UI cleanup에서 `ClearActiveItemSlotDragOperation()`도 호출한다. 가능하면 active storage clear도 현재 storage와 같은 경우에만 수행하도록 방어한다.

### 수정 코드
파일 경로: `Source/BeekeepingSim/Private/StorageBoxFocusActionComponent.cpp`

```cpp
if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(PlayerController))
{
	BeekeeperController->ClearActiveItemSlotDragOperation();
	BeekeeperController->ClearActiveStorageComponent();
}
```

## 개선 제안

- `MoveHotbarItemToStorage()`의 temporary `UE_LOG(LogTemp, Warning, TEXT("MoveHotbarItemToStorage"));`는 제거한다.
- partial move 내부에서 여러 `SetSlotItem()`을 호출하는 경로는 여러 번 broadcast될 수 있으므로, batch update API가 필요하면 별도 helper로 정리한다.
- quick move가 merge 가능한 슬롯만 있고 전체 수량을 받을 빈 슬롯이 없을 때 부분 성공을 허용할지, 전체 이동만 성공으로 볼지 정책을 명확히 한다.

## 검증 항목

- Storage -> Hotbar partial merge 후 hotbar stack count UI가 즉시 갱신되는지 확인한다.
- Hotbar -> Storage partial move 후 source hotbar stack count UI가 즉시 갱신되는지 확인한다.
- Storage engaged 중 empty hotbar slot이 enabled visual/drop target으로 남는지 확인한다.
- 드래그 중 storage cancel/abort/endplay 후 wheel 입력이 hotbar 순환으로 정상 복귀하는지 확인한다.
- LMB double click quick move가 merge 가능 슬롯 우선, 없으면 빈 슬롯 순서로 동작하는지 확인한다.
- UBT 빌드를 다시 실행한다.
