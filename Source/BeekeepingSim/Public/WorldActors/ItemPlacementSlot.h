#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemPlacementSlot.generated.h"

class AActor;
class ABeekeeperCharacter;
class UItemInstance;

UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UItemPlacementSlot : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IItemPlacementSlot
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Placement Slot")
	bool TryPlaceItem(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Placement Slot")
	bool IsPlacementOccupied() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Placement Slot")
	void ClearPlacedItem();
};
