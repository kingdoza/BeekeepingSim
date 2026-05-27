#pragma once

#include "CoreMinimal.h"
#include "WorldActors/ItemPlacementSlotActor.h"
#include "BeehiveCombSlotActor.generated.h"

class ABeehive;
class ABeehiveCombActor;
class UItemInstance;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeehiveCombSlotActor : public AItemPlacementSlotActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Beehive|Comb Slot")
	ABeehiveCombActor* GetPlacedCombActor() const;

	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;
	virtual bool TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void ClearPlacedItem_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual bool CanAcceptOccupantActor(AActor* CandidateActor) const override;

private:
	void RequestOwningBeehiveRefresh() const;
	ABeehive* ResolveOwningBeehive() const;
};
