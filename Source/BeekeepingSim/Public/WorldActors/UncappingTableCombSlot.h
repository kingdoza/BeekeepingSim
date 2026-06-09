#pragma once

#include "CoreMinimal.h"
#include "WorldActors/ItemPlacementSlotActor.h"
#include "UncappingTableCombSlot.generated.h"

class ABeehiveCombActor;
class ABeekeeperCharacter;
class UCombUncappingPartFocusActionComponent;
class UCursorPartFocusActionComponent;
class UCursorPartFocusScopeComponent;
class UItemInstance;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AUncappingTableCombSlot : public AItemPlacementSlotActor
{
	GENERATED_BODY()

public:
	AUncappingTableCombSlot();

	UFUNCTION(BlueprintPure, Category = "Uncapping Table|Comb Slot")
	ABeehiveCombActor* GetPlacedCombActor() const;

	UFUNCTION(BlueprintPure, Category = "Uncapping Table|Comb Slot")
	bool IsCombPartFocusEngaged() const { return bCombPartFocusEngaged; }

	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;
	virtual bool TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void ClearPlacedItem_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual bool CanAcceptOccupantActor(AActor* CandidateActor) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Uncapping Table|Part Focus")
	TObjectPtr<UCombUncappingPartFocusActionComponent> CombPartFocusAction;

	UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Grabbed"))
	void ReceiveCombGrabbed(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Released"))
	void ReceiveCombReleased(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Grab Aborted"))
	void ReceiveCombGrabAborted(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

private:
	UFUNCTION()
	void HandleCombPartFocusBegin(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION()
	void HandleCombPartFocusCancel(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION()
	void HandleCombPartFocusAbort(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	void SetCombPartFocusEngaged(bool bNewEngaged);
	void RequestOwningUncappingTableRefresh() const;

	bool bCombPartFocusEngaged = false;
};
