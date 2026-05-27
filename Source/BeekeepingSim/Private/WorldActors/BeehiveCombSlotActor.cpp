#include "WorldActors/BeehiveCombSlotActor.h"

#include "Inventory/ItemInstance.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeehiveCombActor.h"

ABeehiveCombActor* ABeehiveCombSlotActor::GetPlacedCombActor() const
{
	return Cast<ABeehiveCombActor>(GetOccupiedActor());
}

void ABeehiveCombSlotActor::GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const
{
	// Comb slot retrieve is bridged through the comb part action's secondary input.
}

bool ABeehiveCombSlotActor::TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter)
{
	UClass* ResolvedClass = PlacedActorClass.Get();
	if (!ResolvedClass || !ResolvedClass->IsChildOf(ABeehiveCombActor::StaticClass()))
	{
		return false;
	}

	const bool bPlaced = Super::TryPlaceItem_Implementation(PlacedActorClass, SourceItemInstance, InteractingCharacter);
	if (!bPlaced)
	{
		return false;
	}

	if (ABeehiveCombActor* CombActor = GetPlacedCombActor())
	{
		CombActor->ApplyStateFromItemInstance(SourceItemInstance);
	}

	RequestOwningBeehiveRefresh();
	return true;
}

void ABeehiveCombSlotActor::ClearPlacedItem_Implementation()
{
	Super::ClearPlacedItem_Implementation();
	RequestOwningBeehiveRefresh();
}

void ABeehiveCombSlotActor::BeginPlay()
{
	Super::BeginPlay();
	RequestOwningBeehiveRefresh();
}

bool ABeehiveCombSlotActor::CanAcceptOccupantActor(AActor* CandidateActor) const
{
	return CandidateActor && CandidateActor->IsA<ABeehiveCombActor>();
}

void ABeehiveCombSlotActor::RequestOwningBeehiveRefresh() const
{
	if (ABeehive* Beehive = ResolveOwningBeehive())
	{
		Beehive->RefreshCombStateFromSlots();
	}
}

ABeehive* ABeehiveCombSlotActor::ResolveOwningBeehive() const
{
	return Cast<ABeehive>(GetAttachParentActor());
}
