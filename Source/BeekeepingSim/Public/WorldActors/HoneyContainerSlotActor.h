#pragma once

#include "CoreMinimal.h"
#include "WorldActors/ItemPlacementSlotActor.h"
#include "HoneyContainerSlotActor.generated.h"

class ABeekeeperCharacter;
class AHoneyContainerActor;
class UItemDefinition;
class UItemInstance;
class USceneComponent;

UENUM(BlueprintType)
enum class EHoneyContainerSlotRole : uint8
{
	Source,
	Target
};

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AHoneyContainerSlotActor : public AItemPlacementSlotActor
{
	GENERATED_BODY()

public:
	AHoneyContainerSlotActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;
	virtual bool TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void ClearPlacedItem_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Honey Container Slot")
	AHoneyContainerActor* GetPlacedHoneyContainerActor() const;

	UFUNCTION(BlueprintPure, Category = "Honey Container Slot")
	EHoneyContainerSlotRole GetSlotRole() const { return SlotRole; }

	UFUNCTION(BlueprintCallable, Category = "Honey Container Slot")
	void SetSlotRole(EHoneyContainerSlotRole NewRole);

	UFUNCTION(BlueprintPure, Category = "Honey Container Slot")
	USceneComponent* GetSlotPourTargetComponent() const { return AttachComponent; }

protected:
	virtual void BeginPlay() override;
	virtual bool CanAcceptOccupantActor(AActor* CandidateActor) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honey Container Slot")
	EHoneyContainerSlotRole SlotRole = EHoneyContainerSlotRole::Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honey Container Slot")
	FGameplayTagQuery AcceptedItemTagQuery;

private:
	bool CanAcceptSourceItem(const UItemInstance* SourceItemInstance) const;
	bool DoesDefinitionMatchAcceptedQuery(const UItemDefinition* ItemDefinition) const;
	void ApplySlotRoleAuthoring();
	void RequestOwningHostRefresh() const;
	void StopOwningTransferIfActive() const;
	FText ResolveContainerDisplayName(const AHoneyContainerActor* ContainerActor) const;
};
