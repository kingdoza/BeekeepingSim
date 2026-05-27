#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/PlacementOccupantComponent.h"

bool UPlacementSlotRetrievePartFocusActionComponent::CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const
{
	return CanRetrievePlacementOccupant(InteractingCharacter);
}

bool UPlacementSlotRetrievePartFocusActionComponent::HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	UItemInstance* AcquiredItemInstance = nullptr;
	AActor* SlotActor = nullptr;
	if (!TryRetrievePlacementOccupant(InteractingCharacter, AcquiredItemInstance, SlotActor))
	{
		return false;
	}

	IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
	return true;
}

bool UPlacementSlotRetrievePartFocusActionComponent::CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const
{
	const UPlacementOccupantComponent* Occupant = ResolvePlacementOccupant();
	if (!Occupant || !InteractingCharacter || !InteractingCharacter->GetBeekeeperHotbar())
	{
		return false;
	}

	AActor* SlotActor = Occupant->GetOwningPlacementSlotActor();
	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return false;
	}

	return Occupant->CanRetrievePlacementOccupant(InteractingCharacter);
}

bool UPlacementSlotRetrievePartFocusActionComponent::TryRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter, UItemInstance*& OutAcquiredItemInstance, AActor*& OutSlotActor)
{
	OutAcquiredItemInstance = nullptr;
	OutSlotActor = nullptr;

	UPlacementOccupantComponent* Occupant = ResolvePlacementOccupant();
	if (!Occupant || !InteractingCharacter)
	{
		return false;
	}

	UBeekeeperHotbarComponent* Hotbar = InteractingCharacter->GetBeekeeperHotbar();
	if (!Hotbar)
	{
		return false;
	}

	AActor* SlotActor = Occupant->GetOwningPlacementSlotActor();
	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return false;
	}

	if (!Occupant->CanRetrievePlacementOccupant(InteractingCharacter))
	{
		return false;
	}

	UItemDefinition* ItemDefinition = Occupant->GetReturnItemDefinition();
	if (!ItemDefinition)
	{
		return false;
	}

	const FHotbarItemAcquireResult AcquireResult = Hotbar->TryAcquireItem(ItemDefinition, 1);
	if (!AcquireResult.bSuccess || AcquireResult.AddedQuantity != 1)
	{
		return false;
	}

	OutAcquiredItemInstance = AcquireResult.LastModifiedItemInstance.Get();
	OutSlotActor = SlotActor;
	return true;
}

UPlacementOccupantComponent* UPlacementSlotRetrievePartFocusActionComponent::ResolvePlacementOccupant()
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UPlacementOccupantComponent>() : nullptr;
}

const UPlacementOccupantComponent* UPlacementSlotRetrievePartFocusActionComponent::ResolvePlacementOccupant() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UPlacementOccupantComponent>() : nullptr;
}
