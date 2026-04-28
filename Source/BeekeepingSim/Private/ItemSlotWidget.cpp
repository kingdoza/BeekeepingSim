#include "Public/ItemSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Public/BeekeeperController.h"
#include "Public/BeekeeperHotbarComponent.h"
#include "Public/ItemDefinition.h"
#include "Public/ItemInstance.h"
#include "Public/ItemSlotDragDropLibrary.h"
#include "Public/ItemVisualWidget.h"
#include "Public/StorageBoxComponent.h"
#include "Public/StorageSlotDragDropOperation.h"

void UItemSlotWidget::InitializeSlotContext(
	const EStorageSlotContainerType InContainerType,
	const int32 InSlotIndex,
	UBeekeeperHotbarComponent* InHotbarComponent,
	UStorageBoxComponent* InStorageComponent)
{
	ContainerType = InContainerType;
	SlotIndex = InSlotIndex;
	HotbarComponent = InHotbarComponent;
	StorageComponent = InStorageComponent;
	RefreshFromData();
}

void UItemSlotWidget::RefreshFromData()
{
	ItemInstance = nullptr;
	bIsSelected = false;
	bIsActivated = false;

	if (ContainerType == EStorageSlotContainerType::Hotbar && HotbarComponent)
	{
		const TArray<FHotbarSlotData>& HotbarSlots = HotbarComponent->GetSlots();
		if (HotbarSlots.IsValidIndex(SlotIndex))
		{
			ItemInstance = Cast<UItemInstance>(HotbarSlots[SlotIndex].ItemInstance.Get());
			bIsSelected = HotbarComponent->GetSelectedIndex() == SlotIndex;
			bIsActivated = HotbarSlots[SlotIndex].bIsEnabled;
		}
	}
	else if (ContainerType == EStorageSlotContainerType::Storage && StorageComponent)
	{
		if (StorageComponent->IsIndexValid(SlotIndex))
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
	SetDragSourceVisualHidden(false);

	if (ABeekeeperController* BeekeeperController = ResolveBeekeeperController())
	{
		BeekeeperController->ClearActiveItemSlotDragOperation();
	}
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FKey EffectingButton = InMouseEvent.GetEffectingButton();
	if (EffectingButton != EKeys::LeftMouseButton && EffectingButton != EKeys::RightMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

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

	UStorageSlotDragDropOperation* DragOperation = Cast<UStorageSlotDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UStorageSlotDragDropOperation::StaticClass()));
	if (!DragOperation)
	{
		return;
	}

	const FKey EffectingButton = InMouseEvent.GetEffectingButton();
	DragOperation->DragMode = EffectingButton == EKeys::RightMouseButton
		? EItemSlotDragMode::PartialStack
		: EItemSlotDragMode::FullStack;

	DragOperation->SourceType = ContainerType;
	DragOperation->SourceIndex = SlotIndex;
	DragOperation->ItemInstance = ItemInstance;
	DragOperation->SourceHotbarComponent = ContainerType == EStorageSlotContainerType::Hotbar ? HotbarComponent : nullptr;
	DragOperation->SourceStorageComponent = ContainerType == EStorageSlotContainerType::Storage ? StorageComponent : nullptr;
	DragOperation->InitializeMoveQuantity();

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
	UStorageSlotDragDropOperation* DragOperation = Cast<UStorageSlotDragDropOperation>(InOperation);
	if (!DragOperation)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	const bool bHandled = UItemSlotDragDropLibrary::HandleItemSlotDrop(
		DragOperation,
		ContainerType,
		SlotIndex,
		HotbarComponent,
		StorageComponent);

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

	if (ContainerType == EStorageSlotContainerType::Hotbar)
	{
		if (!HotbarComponent)
		{
			return false;
		}

		ABeekeeperController* Controller = ResolveBeekeeperController();
		UStorageBoxComponent* ActiveStorage = Controller ? Controller->GetActiveStorageComponent() : nullptr;
		if (!ActiveStorage)
		{
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
			return false;
		}

		return ActiveStorage->MovePartialHotbarToStorage(HotbarComponent, SlotIndex, TargetStorageIndex, SourceQuantity).bSuccess;
	}

	if (ContainerType == EStorageSlotContainerType::Storage)
	{
		if (!StorageComponent || !HotbarComponent)
		{
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
			return false;
		}

		return StorageComponent->MovePartialStorageToHotbar(HotbarComponent, SlotIndex, TargetHotbarIndex, SourceQuantity).bSuccess;
	}

	return false;
}
