#include "UI/ItemSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Character/BeekeeperController.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "UI/ItemSlotDragDropLibrary.h"
#include "UI/ItemVisualWidget.h"
#include "Inventory/StorageBoxComponent.h"
#include "UI/ItemSlotDragDropOperation.h"

void UItemSlotWidget::InitializeSlotContext(
	const EItemSlotContainerType InContainerType,
	const int32 InSlotIndex)
{
	ContainerType = InContainerType;
	SlotIndex = InSlotIndex;
	RefreshFromData();
}

void UItemSlotWidget::RefreshFromData()
{
	ItemInstance = nullptr;
	bIsSelected = false;
	bIsActivated = false;

	if (ContainerType == EItemSlotContainerType::Hotbar)
	{
		UBeekeeperHotbarComponent* HotbarComponent = ResolveHotbarComponentForSlot();
		if (HotbarComponent)
		{
			const TArray<FHotbarSlotData>& HotbarSlots = HotbarComponent->GetSlots();
			if (HotbarSlots.IsValidIndex(SlotIndex))
			{
				ItemInstance = Cast<UItemInstance>(HotbarSlots[SlotIndex].ItemInstance.Get());
				bIsSelected = HotbarComponent->GetSelectedIndex() == SlotIndex;
				bIsActivated = HotbarSlots[SlotIndex].bIsEnabled;
			}
		}
	}
	else if (ContainerType == EItemSlotContainerType::Storage)
	{
		UStorageBoxComponent* StorageComponent = ResolveStorageComponentForSlot();
		if (StorageComponent && StorageComponent->IsIndexValid(SlotIndex))
		{
			ItemInstance = StorageComponent->GetItemAt(SlotIndex);
			bIsActivated = true;
		}
	}

	RefreshVisual();
}

void UItemSlotWidget::RefreshVisual()
{
	OnSlotVisualStateChanged();
}

void UItemSlotWidget::SetDragSourceVisualHidden(const bool bHidden)
{
	bIsDragSource = bHidden;
	RefreshVisual();
}

void UItemSlotWidget::ClearDragState()
{
	PendingDragButton = EKeys::Invalid;
	ClearPartialDragPreviewState();
	ActiveDragMode = EItemSlotDragMode::FullStack;
	SetDragSourceVisualHidden(false);
	RefreshFromData();

	if (ABeekeeperController* BeekeeperController = ResolveBeekeeperController())
	{
		BeekeeperController->ClearActiveItemSlotDragOperation();
	}
}

bool UItemSlotWidget::IsPartialDragPreviewActive() const
{
	return bPartialDragPreviewActive
		&& PartialDragPreviewOriginalStackCount > 0
		&& PartialDragPreviewMoveQuantity > 0
		&& !ShouldHideItemVisualForPartialDragPreview();
}

bool UItemSlotWidget::ShouldHideItemVisualForPartialDragPreview() const
{
	return bPartialDragPreviewActive
		&& PartialDragPreviewOriginalStackCount > 0
		&& PartialDragPreviewMoveQuantity >= PartialDragPreviewOriginalStackCount;
}

int32 UItemSlotWidget::GetPartialDragPreviewDisplayStackCount() const
{
	if (!bPartialDragPreviewActive)
	{
		return 0;
	}

	return FMath::Max(0, PartialDragPreviewOriginalStackCount - PartialDragPreviewMoveQuantity);
}

void UItemSlotWidget::SetPartialDragPreviewState(const int32 InOriginalStackCount, const int32 InMoveQuantity)
{
	const int32 NewOriginal = FMath::Max(0, InOriginalStackCount);
	const int32 NewMove = FMath::Clamp(InMoveQuantity, 0, NewOriginal);
	const bool bNewActive = NewOriginal > 0 && NewMove > 0;

	if (PartialDragPreviewOriginalStackCount == NewOriginal
		&& PartialDragPreviewMoveQuantity == NewMove
		&& bPartialDragPreviewActive == bNewActive)
	{
		return;
	}

	PartialDragPreviewOriginalStackCount = NewOriginal;
	PartialDragPreviewMoveQuantity = NewMove;
	bPartialDragPreviewActive = bNewActive;
	RefreshVisual();
}

void UItemSlotWidget::UpdatePartialDragPreviewMoveQuantity(const int32 InMoveQuantity)
{
	if (!bPartialDragPreviewActive)
	{
		return;
	}

	const int32 NewMove = FMath::Clamp(InMoveQuantity, 0, PartialDragPreviewOriginalStackCount);
	if (PartialDragPreviewMoveQuantity == NewMove)
	{
		return;
	}

	PartialDragPreviewMoveQuantity = NewMove;
	RefreshVisual();
}

void UItemSlotWidget::ClearPartialDragPreviewState()
{
	if (PartialDragPreviewOriginalStackCount == 0
		&& PartialDragPreviewMoveQuantity == 0
		&& !bPartialDragPreviewActive)
	{
		return;
	}

	PartialDragPreviewOriginalStackCount = 0;
	PartialDragPreviewMoveQuantity = 0;
	bPartialDragPreviewActive = false;
	RefreshVisual();
}

void UItemSlotWidget::RefreshPartialDragPreviewFromOperation(UItemSlotDragDropOperation* Operation)
{
	if (!Operation || Operation->DragMode != EItemSlotDragMode::PartialStack || !ItemInstance || Operation->SourceSlotWidget != this)
	{
		ClearPartialDragPreviewState();
		return;
	}

	if (!bPartialDragPreviewActive)
	{
		SetPartialDragPreviewState(ItemInstance->GetStackCount(), Operation->MoveQuantity);
		return;
	}

	UpdatePartialDragPreviewMoveQuantity(Operation->MoveQuantity);
}

bool UItemSlotWidget::ShouldHideItemVisualForCurrentDrag() const
{
	if (!bIsDragSource)
	{
		return false;
	}

	if (ActiveDragMode == EItemSlotDragMode::FullStack)
	{
		return true;
	}

	return ShouldHideItemVisualForPartialDragPreview();
}

void UItemSlotWidget::RefreshDragPreviewFromOperation(UItemSlotDragDropOperation* Operation)
{
	RefreshPartialDragPreviewFromOperation(Operation);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FKey EffectingButton = InMouseEvent.GetEffectingButton();
	if (EffectingButton != EKeys::LeftMouseButton && EffectingButton != EKeys::RightMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	PendingDragButton = EffectingButton;
	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EffectingButton).NativeReply;
}

FReply UItemSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
	}

	return TryQuickMove()
		? FReply::Handled()
		: Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UItemSlotWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemInstance || SlotIndex == INDEX_NONE)
	{
		return;
	}

	UItemSlotDragDropOperation* DragOperation = Cast<UItemSlotDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UItemSlotDragDropOperation::StaticClass()));
	if (!DragOperation)
	{
		return;
	}

	if (PendingDragButton != EKeys::LeftMouseButton && PendingDragButton != EKeys::RightMouseButton)
	{
		OutOperation = nullptr;
		return;
	}

	const bool bIsRightDrag = PendingDragButton == EKeys::RightMouseButton;
	DragOperation->DragMode = bIsRightDrag ? EItemSlotDragMode::PartialStack : EItemSlotDragMode::FullStack;

	DragOperation->SourceType = ContainerType;
	DragOperation->SourceIndex = SlotIndex;
	DragOperation->ItemInstance = ItemInstance;
	DragOperation->SourceHotbarComponent = ContainerType == EItemSlotContainerType::Hotbar ? ResolveHotbarComponentForSlot() : nullptr;
	DragOperation->SourceStorageComponent = ContainerType == EItemSlotContainerType::Storage ? ResolveStorageComponentForSlot() : nullptr;
	DragOperation->InitializeMoveQuantity();
	DragOperation->SourceSlotWidget = this;

	if (DragVisualWidgetClass)
	{
		if (UItemVisualWidget* DragVisualWidget = CreateWidget<UItemVisualWidget>(GetOwningPlayer(), DragVisualWidgetClass))
		{
			const int32 QuantityOverride =
				DragOperation->DragMode == EItemSlotDragMode::PartialStack ? DragOperation->MoveQuantity : INDEX_NONE;
			DragVisualWidget->SetItemVisualData(ItemInstance, QuantityOverride);
			DragOperation->DragVisualWidget = DragVisualWidget;
			DragOperation->DefaultDragVisual = DragVisualWidget;
		}
	}

	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Offset = FVector2D::ZeroVector;
	DragOperation->OnDrop.AddDynamic(this, &UItemSlotWidget::HandleDragOperationDropped);
	DragOperation->OnDragCancelled.AddDynamic(this, &UItemSlotWidget::HandleDragOperationCancelled);

	ActiveDragMode = DragOperation->DragMode;
	if (DragOperation->DragMode == EItemSlotDragMode::PartialStack)
	{
		SetPartialDragPreviewState(ItemInstance ? ItemInstance->GetStackCount() : 0, DragOperation->MoveQuantity);
	}
	else
	{
		ClearPartialDragPreviewState();
	}

	SetDragSourceVisualHidden(true);
	if (ABeekeeperController* BeekeeperController = ResolveBeekeeperController())
	{
		BeekeeperController->SetActiveItemSlotDragOperation(DragOperation);
	}

	OutOperation = DragOperation;
}

bool UItemSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UItemSlotDragDropOperation* DragOperation = Cast<UItemSlotDragDropOperation>(InOperation);
	if (!DragOperation)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	const bool bHandled = UItemSlotDragDropLibrary::HandleItemSlotDrop(
		DragOperation,
		ContainerType,
		SlotIndex,
		ResolveHotbarComponentForSlot(),
		ResolveStorageComponentForSlot());

	if (bHandled)
	{
		RefreshFromData();
	}

	return bHandled;
}

void UItemSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	ClearDragState();
}

void UItemSlotWidget::HandleDragOperationDropped(UDragDropOperation* InOperation)
{
	ClearDragState();
}

void UItemSlotWidget::HandleDragOperationCancelled(UDragDropOperation* InOperation)
{
	ClearDragState();
}

ABeekeeperController* UItemSlotWidget::ResolveBeekeeperController() const
{
	return Cast<ABeekeeperController>(GetOwningPlayer());
}

bool UItemSlotWidget::TryQuickMove()
{
	if (!ItemInstance || SlotIndex == INDEX_NONE)
	{
		return false;
	}

	const int32 SourceQuantity = ItemInstance->GetStackCount();
	if (SourceQuantity <= 0)
	{
		return false;
	}

	const UItemDefinition* SourceDefinition = ItemInstance->GetDefinition();
	if (!SourceDefinition)
	{
		return false;
	}

	if (ContainerType == EItemSlotContainerType::Hotbar)
	{
		UBeekeeperHotbarComponent* HotbarComponent = ResolveHotbarComponentForSlot();
		if (!HotbarComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("TryQuickMove failed: hotbar context is null for hotbar source slot."));
			return false;
		}

		UStorageBoxComponent* ActiveStorage = ResolveStorageComponentForSlot();
		if (!ActiveStorage)
		{
			UE_LOG(LogTemp, Warning, TEXT("TryQuickMove failed: active storage context is null for hotbar source slot."));
			return false;
		}

		const TArray<FStorageBoxSlotData>& StorageSlots = ActiveStorage->GetSlots();
		int32 TargetStorageIndex = INDEX_NONE;

		for (int32 Index = 0; Index < StorageSlots.Num(); ++Index)
		{
			UItemInstance* StorageItem = StorageSlots[Index].ItemInstance.Get();
			if (!StorageItem || StorageItem->GetDefinition() != SourceDefinition)
			{
				continue;
			}

			const int32 MaxStack = FMath::Max(1, SourceDefinition->MaxStack);
			if (StorageItem->GetStackCount() < MaxStack)
			{
				TargetStorageIndex = Index;
				break;
			}
		}

		if (TargetStorageIndex == INDEX_NONE)
		{
			for (int32 Index = 0; Index < StorageSlots.Num(); ++Index)
			{
				if (!StorageSlots[Index].ItemInstance)
				{
					TargetStorageIndex = Index;
					break;
				}
			}
		}

		if (TargetStorageIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("TryQuickMove failed: no valid storage target slot found."));
			return false;
		}

		return ActiveStorage->MovePartialHotbarToStorage(HotbarComponent, SlotIndex, TargetStorageIndex, SourceQuantity).bSuccess;
	}

	if (ContainerType == EItemSlotContainerType::Storage)
	{
		UStorageBoxComponent* StorageComponent = ResolveStorageComponentForSlot();
		UBeekeeperHotbarComponent* HotbarComponent = ResolveHotbarComponentForSlot();
		if (!StorageComponent || !HotbarComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("TryQuickMove failed: storage/hotbar context is null for storage source slot."));
			return false;
		}

		const TArray<FHotbarSlotData>& HotbarSlots = HotbarComponent->GetSlots();
		int32 TargetHotbarIndex = INDEX_NONE;

		for (int32 Index = 0; Index < HotbarSlots.Num(); ++Index)
		{
			UItemInstance* HotbarItem = Cast<UItemInstance>(HotbarSlots[Index].ItemInstance.Get());
			if (!HotbarItem || HotbarItem->GetDefinition() != SourceDefinition)
			{
				continue;
			}

			const int32 MaxStack = FMath::Max(1, SourceDefinition->MaxStack);
			if (HotbarItem->GetStackCount() < MaxStack)
			{
				TargetHotbarIndex = Index;
				break;
			}
		}

		if (TargetHotbarIndex == INDEX_NONE)
		{
			for (int32 Index = 0; Index < HotbarSlots.Num(); ++Index)
			{
				if (!HotbarSlots[Index].ItemInstance)
				{
					TargetHotbarIndex = Index;
					break;
				}
			}
		}

		if (TargetHotbarIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("TryQuickMove failed: no valid hotbar target slot found."));
			return false;
		}

		return StorageComponent->MovePartialStorageToHotbar(HotbarComponent, SlotIndex, TargetHotbarIndex, SourceQuantity).bSuccess;
	}

	return false;
}

UBeekeeperHotbarComponent* UItemSlotWidget::ResolveHotbarComponentForSlot() const
{
	const ABeekeeperController* Controller = ResolveBeekeeperController();
	return Controller ? Controller->GetPlayerHotbarComponent() : nullptr;
}

UStorageBoxComponent* UItemSlotWidget::ResolveStorageComponentForSlot() const
{
	const ABeekeeperController* Controller = ResolveBeekeeperController();
	return Controller ? Controller->GetActiveStorageComponent() : nullptr;
}
