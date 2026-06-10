#include "WorldActors/HoneyContainerRetrievePartFocusActionComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/HoneyContainerActor.h"
#include "WorldActors/ItemPlacementSlot.h"

bool UHoneyContainerRetrievePartFocusActionComponent::HandleSecondaryPartFocusAction(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter)
{
	(void)ScopeComponent;

	AHoneyContainerActor* ContainerActor = Cast<AHoneyContainerActor>(GetOwner());
	if (!ContainerActor)
	{
		return false;
	}

	UItemInstance* AcquiredItemInstance = nullptr;
	AActor* SlotActor = nullptr;
	if (!TryRetrievePlacementOccupant(InteractingCharacter, AcquiredItemInstance, SlotActor))
	{
		return false;
	}

	ContainerActor->WriteHoneyContainerStateToItemInstance(AcquiredItemInstance);

	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return false;
	}

	IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
	return true;
}
