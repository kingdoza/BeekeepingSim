#include "WorldActors/PlacedItemRetrieveFocusActionComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"

bool UPlacedItemRetrieveFocusActionComponent::CanExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	const UPlacementSlotRetrievePartFocusActionComponent* RetrieveAction = GetOwner() ? GetOwner()->FindComponentByClass<UPlacementSlotRetrievePartFocusActionComponent>() : nullptr;
	if (!RetrieveAction)
	{
		return false;
	}

	return RetrieveAction->CanRetrievePlacementOccupant(InteractingCharacter);
}

bool UPlacedItemRetrieveFocusActionComponent::ExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	UPlacementSlotRetrievePartFocusActionComponent* RetrieveAction = GetOwner() ? GetOwner()->FindComponentByClass<UPlacementSlotRetrievePartFocusActionComponent>() : nullptr;
	if (!RetrieveAction)
	{
		return false;
	}

	UItemInstance* AcquiredItemInstance = nullptr;
	AActor* SlotActor = nullptr;
	if (!RetrieveAction->TryRetrievePlacementOccupant(InteractingCharacter, AcquiredItemInstance, SlotActor))
	{
		return false;
	}

	if (SlotActor && SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
		return true;
	}

	return false;
}
