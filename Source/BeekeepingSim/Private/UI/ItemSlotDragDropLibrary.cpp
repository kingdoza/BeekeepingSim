#include "UI/ItemSlotDragDropLibrary.h"

#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/StorageBoxComponent.h"
#include "UI/ItemSlotDragDropOperation.h"

bool UItemSlotDragDropLibrary::HandleItemSlotDrop(
	UItemSlotDragDropOperation* Operation,
	const EItemSlotContainerType TargetType,
	const int32 TargetIndex,
	UBeekeeperHotbarComponent* TargetHotbarComponent,
	UStorageBoxComponent* TargetStorageComponent)
{
	if (!Operation ||
		Operation->SourceType == EItemSlotContainerType::None ||
		TargetType == EItemSlotContainerType::None ||
		Operation->SourceIndex == INDEX_NONE ||
		TargetIndex == INDEX_NONE)
	{
		return false;
	}

	const bool bUsePartialMove =
		Operation->DragMode == EItemSlotDragMode::PartialStack &&
		Operation->MoveQuantity > 0;

	if (Operation->SourceType == EItemSlotContainerType::Hotbar &&
		TargetType == EItemSlotContainerType::Hotbar)
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

	if (Operation->SourceType == EItemSlotContainerType::Hotbar &&
		TargetType == EItemSlotContainerType::Storage)
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

	if (Operation->SourceType == EItemSlotContainerType::Storage &&
		TargetType == EItemSlotContainerType::Hotbar)
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

	if (Operation->SourceType == EItemSlotContainerType::Storage &&
		TargetType == EItemSlotContainerType::Storage)
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
