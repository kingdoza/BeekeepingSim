#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldItemPickup.generated.h"

class UPickupFocusActionComponent;
class UFocusTargetComponent;
class UItemDefinition;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API AWorldItemPickup : public AActor
{
	GENERATED_BODY()

public:
	AWorldItemPickup();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UItemDefinition* GetItemDefinition() const { return ItemDefinition; }

	UFUNCTION(BlueprintPure, Category = "Pickup")
	bool IsPickupValid() const;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void ConsumePickup();

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void RefreshPickupPresentation();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFocusTargetComponent> FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPickupFocusActionComponent> FocusAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UItemDefinition> ItemDefinition = nullptr;
};
