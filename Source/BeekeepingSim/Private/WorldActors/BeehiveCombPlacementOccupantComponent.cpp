#include "WorldActors/BeehiveCombPlacementOccupantComponent.h"

#include "Inventory/ItemDefinition.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeehiveCombActor.h"

bool UBeehiveCombPlacementOccupantComponent::ReceiveCanRetrievePlacementOccupant_Implementation(ABeekeeperCharacter* InteractingCharacter) const
{
	if (!Super::ReceiveCanRetrievePlacementOccupant_Implementation(InteractingCharacter))
	{
		return false;
	}

	const ABeehiveCombActor* CombActor = GetOwner<ABeehiveCombActor>();
	if (!CombActor)
	{
		return false;
	}

	const UItemDefinition* ReturnDefinition = GetReturnItemDefinition();
	if (!ReturnDefinition || ReturnDefinition->MaxStack != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s retrieve is blocked: beehive comb return item must have MaxStack == 1."), *GetNameSafe(CombActor));
		return false;
	}

	if (CombActor->GetTargetBeeCount() != 0)
	{
		return false;
	}

	const AActor* SlotActor = GetOwningPlacementSlotActor();
	const ABeehive* Beehive = SlotActor ? Cast<ABeehive>(SlotActor->GetAttachParentActor()) : nullptr;
	if (!Beehive)
	{
		return true;
	}

	return !Beehive->IsQueenBeeAttachedToComb(CombActor);
}
