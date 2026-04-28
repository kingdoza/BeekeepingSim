#include "Public/StorageSlotDragDropOperation.h"
#include "Public/ItemInstance.h"

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
	if (Delta == 0)
	{
		return;
	}

	SetMoveQuantityClamped(MoveQuantity + Delta);
}

void UStorageSlotDragDropOperation::SetMoveQuantityClamped(const int32 NewQuantity)
{
	const int32 MinQuantity = MaxMoveQuantity > 0 ? 1 : 0;
	MoveQuantity = FMath::Clamp(NewQuantity, MinQuantity, MaxMoveQuantity);
}
