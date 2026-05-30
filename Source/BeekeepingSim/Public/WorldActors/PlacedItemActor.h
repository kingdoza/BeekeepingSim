#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlacedItemActor.generated.h"

class UCursorPartFocusActionComponent;
class UItemDefinition;
class UItemInstance;
class UPlacedItemRemainingComponent;
class UPlacedItemRemainingVisualComponent;
class UPlacedItemRetrievePartFocusActionComponent;
class UPlacementOccupantComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API APlacedItemActor : public AActor
{
	GENERATED_BODY()

public:
	APlacedItemActor();

	UFUNCTION(BlueprintCallable, Category = "Placed Item")
	void InitializePlacedItem(UItemInstance* SourceItemInstance, AActor* InOwningPlacementSlotActor);

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UItemDefinition* GetItemDefinition() const;

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	AActor* GetOwningPlacementSlotActor() const;

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UPrimitiveComponent* GetPartFocusHitComponent() const;

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UCursorPartFocusActionComponent* GetPartFocusActionComponent() const;

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UPlacementOccupantComponent* GetPlacementOccupantComponent() const { return PlacementOccupant; }

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UPlacedItemRemainingComponent* GetRemainingComponent() const { return RemainingComponent; }

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UPlacedItemRetrievePartFocusActionComponent* GetPlacementRetrieveActionComponent() const { return RetrieveAction.Get(); }

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UStaticMeshComponent* GetItemMeshComponent() const { return ItemMesh; }

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	FText GetPlacedItemDisplayName() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Placed Item")
	void ReceivePlacedItemInitialized(UItemInstance* SourceItemInstance);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlacementOccupantComponent> PlacementOccupant;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placed Item", meta = (DeprecatedProperty, DeprecationMessage = "Use PlacementOccupant return definition API."))
	TObjectPtr<UItemDefinition> ItemDefinition;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Placed Item", meta = (DeprecatedProperty, DeprecationMessage = "Use PlacementOccupant owning slot API."))
	TObjectPtr<AActor> OwningPlacementSlotActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlacedItemRetrievePartFocusActionComponent> RetrieveAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlacedItemRemainingComponent> RemainingComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPlacedItemRemainingVisualComponent> RuntimeRemainingVisualComponent;

private:
	void InitializeRemainingVisualComponent(UItemDefinition* ReturnItemDefinition);
};
