#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlacementOccupantComponent.generated.h"

class ABeekeeperCharacter;
class UItemDefinition;
class UItemInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacementOccupantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
	void InitializeFromPlacement(UItemInstance* SourceItemInstance, AActor* InOwningPlacementSlotActor);

	UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
	void SetOwningPlacementSlotActor(AActor* InOwningPlacementSlotActor);

	UFUNCTION(BlueprintPure, Category = "Placement Occupant")
	UItemDefinition* GetReturnItemDefinition() const;

	UFUNCTION(BlueprintPure, Category = "Placement Occupant")
	AActor* GetOwningPlacementSlotActor() const;

	UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
	bool CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const;

	UFUNCTION(BlueprintCallable, Category = "Placement Occupant")
	void PreClearPlacementOccupant();

	UFUNCTION(BlueprintNativeEvent, Category = "Placement Occupant")
	bool ReceiveCanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Placement Occupant")
	void ReceivePreClearPlacementOccupant();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Placement Occupant")
	TObjectPtr<UItemDefinition> RuntimeReturnItemDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Occupant")
	TObjectPtr<UItemDefinition> AuthoredReturnItemDefinition = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Placement Occupant")
	TObjectPtr<AActor> OwningPlacementSlotActor = nullptr;
};

