#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlacedItemActor.generated.h"

class UCursorPartFocusActionComponent;
class UItemDefinition;
class UItemInstance;
class UPlacedItemRetrievePartFocusActionComponent;
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
	UItemDefinition* GetItemDefinition() const { return ItemDefinition; }

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	AActor* GetOwningPlacementSlotActor() const { return OwningPlacementSlotActor; }

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UPrimitiveComponent* GetPartFocusHitComponent() const;

	UFUNCTION(BlueprintPure, Category = "Placed Item")
	UCursorPartFocusActionComponent* GetPartFocusActionComponent() const;

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
	TObjectPtr<UPlacedItemRetrievePartFocusActionComponent> RetrieveAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placed Item")
	TObjectPtr<UItemDefinition> ItemDefinition;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Placed Item")
	TObjectPtr<AActor> OwningPlacementSlotActor;
};
