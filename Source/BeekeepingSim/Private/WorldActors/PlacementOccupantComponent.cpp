#include "WorldActors/PlacementOccupantComponent.h"

#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"

void UPlacementOccupantComponent::InitializeFromPlacement(UItemInstance* SourceItemInstance, AActor* InOwningPlacementSlotActor)
{
	RuntimeReturnItemDefinition = SourceItemInstance ? SourceItemInstance->GetDefinition() : nullptr;
	SetOwningPlacementSlotActor(InOwningPlacementSlotActor);
}

void UPlacementOccupantComponent::SetOwningPlacementSlotActor(AActor* InOwningPlacementSlotActor)
{
	OwningPlacementSlotActor = InOwningPlacementSlotActor;
}

UItemDefinition* UPlacementOccupantComponent::GetReturnItemDefinition() const
{
	return RuntimeReturnItemDefinition ? RuntimeReturnItemDefinition.Get() : AuthoredReturnItemDefinition.Get();
}

AActor* UPlacementOccupantComponent::GetOwningPlacementSlotActor() const
{
	return OwningPlacementSlotActor.Get();
}

bool UPlacementOccupantComponent::CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const
{
	if (!GetReturnItemDefinition() || !OwningPlacementSlotActor)
	{
		return false;
	}

	return ReceiveCanRetrievePlacementOccupant(InteractingCharacter);
}

void UPlacementOccupantComponent::PreClearPlacementOccupant()
{
	ReceivePreClearPlacementOccupant();
}

bool UPlacementOccupantComponent::ReceiveCanRetrievePlacementOccupant_Implementation(ABeekeeperCharacter* InteractingCharacter) const
{
	return true;
}

void UPlacementOccupantComponent::ReceivePreClearPlacementOccupant_Implementation()
{
}

