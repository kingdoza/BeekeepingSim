#include "Public/ItemSlotDragDropLibrary.h"

#include "Public/BeekeeperHotbarComponent.h"
#include "Public/StorageBoxComponent.h"
#include "Public/StorageSlotDragDropOperation.h"

bool UItemSlotDragDropLibrary::HandleItemSlotDrop(
	UStorageSlotDragDropOperation* Operation,
	const EStorageSlotContainerType TargetType,
	const int32 TargetIndex,
	UBeekeeperHotbarComponent* TargetHotbarComponent,
	UStorageBoxComponent* TargetStorageComponent)
{
	if (!Operation ||
		Operation->SourceType == EStorageSlotContainerType::None ||
		TargetType == EStorageSlotContainerType::None ||
		Operation->SourceIndex == INDEX_NONE ||
		TargetIndex == INDEX_NONE)
	{
		return false;
	}

	const bool bUsePartialMove =
		Operation->DragMode == EItemSlotDragMode::PartialStack &&
		Operation->MoveQuantity > 0;

	if (Operation->SourceType == EStorageSlotContainerType::Hotbar &&
		TargetType == EStorageSlotContainerType::Hotbar)
	{
		if (!Operation->SourceHotbarComponent || !TargetHotbarComponent ||
			Operation->SourceHotbarComponent != TargetHotbarComponent)
		{
			return false;
		}

		if (bUsePartialMove)
		{
			return TargetHotbarComponent->MovePartialToSlot(Operation->SourceIndex, TargetIndex, Operation->MoveQuantity).bSuccess;
		}

		return TargetHotbarComponent->SwapSlots(Operation->SourceIndex, TargetIndex);
	}

	if (Operation->SourceType == EStorageSlotContainerType::Hotbar &&
		TargetType == EStorageSlotContainerType::Storage)
	{
		if (!Operation->SourceHotbarComponent || !TargetStorageComponent)
		{
			return false;
		}

		if (bUsePartialMove)
		{
			return TargetStorageComponent->MovePartialHotbarToStorage(
				Operation->SourceHotbarComponent,
				Operation->SourceIndex,
				TargetIndex,
				Operation->MoveQuantity).bSuccess;
		}

		return TargetStorageComponent->MoveHotbarItemToStorage(
			Operation->SourceHotbarComponent,
			Operation->SourceIndex,
			TargetIndex);
	}

	if (Operation->SourceType == EStorageSlotContainerType::Storage &&
		TargetType == EStorageSlotContainerType::Hotbar)
	{
		if (!Operation->SourceStorageComponent || !TargetHotbarComponent)
		{
			return false;
		}

		if (bUsePartialMove)
		{
			return Operation->SourceStorageComponent->MovePartialStorageToHotbar(
				TargetHotbarComponent,
				Operation->SourceIndex,
				TargetIndex,
				Operation->MoveQuantity).bSuccess;
		}

		return Operation->SourceStorageComponent->MoveStorageItemToHotbar(
			TargetHotbarComponent,
			Operation->SourceIndex,
			TargetIndex);
	}

	if (Operation->SourceType == EStorageSlotContainerType::Storage &&
		TargetType == EStorageSlotContainerType::Storage)
	{
		if (!Operation->SourceStorageComponent || !TargetStorageComponent ||
			Operation->SourceStorageComponent != TargetStorageComponent)
		{
			return false;
		}

		if (bUsePartialMove)
		{
			return TargetStorageComponent->MovePartialStorageToStorage(
				Operation->SourceIndex,
				TargetIndex,
				Operation->MoveQuantity).bSuccess;
		}

		return TargetStorageComponent->SwapStorageSlots(Operation->SourceIndex, TargetIndex);
	}

	return false;
}
