#include "WorldActors/PlacedItemRetrieveFocusActionComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemDefinition.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/PlacedItemActor.h"

bool UPlacedItemRetrieveFocusActionComponent::CanExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	const APlacedItemActor* PlacedItem = GetOwner<APlacedItemActor>();
	if (!PlacedItem || !InteractingCharacter)
	{
		return false;
	}

	return PlacedItem->GetItemDefinition() != nullptr && InteractingCharacter->GetBeekeeperHotbar() != nullptr;
}

bool UPlacedItemRetrieveFocusActionComponent::ExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	APlacedItemActor* PlacedItem = GetOwner<APlacedItemActor>();
	if (!PlacedItem || !InteractingCharacter)
	{
		return false;
	}

	UItemDefinition* ItemDefinition = PlacedItem->GetItemDefinition();
	UBeekeeperHotbarComponent* Hotbar = InteractingCharacter->GetBeekeeperHotbar();
	if (!ItemDefinition || !Hotbar)
	{
		return false;
	}

	const FHotbarItemAcquireResult AcquireResult = Hotbar->TryAcquireItem(ItemDefinition, 1);
	if (!AcquireResult.bSuccess || AcquireResult.AddedQuantity != 1)
	{
		return false;
	}

	if (AActor* SlotActor = PlacedItem->GetOwningPlacementSlotActor())
	{
		if (SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
		{
			IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
			return true;
		}
	}

	PlacedItem->Destroy();
	return true;
}

