#include "Public/StorageSlotDragDropOperation.h"
#include "Public/ItemInstance.h"
#include "Public/ItemSlotWidget.h"
#include "Public/ItemVisualWidget.h"

void UStorageSlotDragDropOperation::InitializeMoveQuantity()
{
	const int32 SourceStackCount = ItemInstance ? ItemInstance->GetStackCount() : 0;
	MaxMoveQuantity = FMath::Max(0, SourceStackCount);

	if (DragMode == EItemSlotDragMode::PartialStack)
	{
		MoveQuantity = FMath::Clamp(1, 0, MaxMoveQuantity);
		return;
	}

	MoveQuantity = MaxMoveQuantity;
}

void UStorageSlotDragDropOperation::AdjustMoveQuantity(const int32 Delta)
{
	if (DragMode != EItemSlotDragMode::PartialStack)
	{
		return;
	}

	if (Delta == 0)
	{
		return;
	}

	SetMoveQuantityClamped(MoveQuantity + Delta);
}

void UStorageSlotDragDropOperation::SetMoveQuantityClamped(const int32 NewQuantity)
{
	const int32 MinQuantity = MaxMoveQuantity > 0 ? 1 : 0;
	const int32 ClampedQuantity = FMath::Clamp(NewQuantity, MinQuantity, MaxMoveQuantity);
	if (MoveQuantity == ClampedQuantity)
	{
		return;
	}

	MoveQuantity = ClampedQuantity;

	if (DragVisualWidget)
	{
		DragVisualWidget->SetItemVisualData(ItemInstance, MoveQuantity);
	}

	if (SourceSlotWidget)
	{
		SourceSlotWidget->RefreshDragPreviewFromOperation(this);
	}

	OnMoveQuantityChanged.Broadcast(MoveQuantity);
}
