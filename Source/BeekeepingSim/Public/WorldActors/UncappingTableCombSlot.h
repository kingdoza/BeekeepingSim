#pragma once

#include "CoreMinimal.h"
#include "WorldActors/ItemPlacementSlotActor.h"
#include "UncappingTableCombSlot.generated.h"

class ABeehiveCombActor;
class UCombUncappingPartFocusActionComponent;
class UItemInstance;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AUncappingTableCombSlot : public AItemPlacementSlotActor
{
	GENERATED_BODY()

public:
	AUncappingTableCombSlot();

	UFUNCTION(BlueprintPure, Category = "Uncapping Table|Comb Slot")
	ABeehiveCombActor* GetPlacedCombActor() const;

	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;
	virtual bool TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void ClearPlacedItem_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual bool CanAcceptOccupantActor(AActor* CandidateActor) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Uncapping Table|Part Focus")
	TObjectPtr<UCombUncappingPartFocusActionComponent> CombPartFocusAction;

private:
	void RequestOwningUncappingTableRefresh() const;
};
