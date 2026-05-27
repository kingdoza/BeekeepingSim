#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusProvider.h"
#include "Focus/ItemUseAreaActivationProvider.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "ItemPlacementSlotActor.generated.h"

class UStaticMesh;
class UItemUseAreaMeshComponent;
class UMaterialInterface;
class USceneComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AItemPlacementSlotActor : public AActor, public IItemUseAreaActivationProvider, public IItemPlacementSlot, public ICursorPartFocusProvider
{
	GENERATED_BODY()

public:
	AItemPlacementSlotActor();
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;
	virtual bool IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const override;
	virtual bool TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter) override;
	virtual bool IsPlacementOccupied_Implementation() const override;
	virtual void ClearPlacedItem_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Item Placement Slot")
	AActor* GetPlacedActor() const { return PlacedActor; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshComponent> SlotMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> AttachComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot", meta = (DeprecatedProperty, DeprecationMessage = "Use SlotMeshComponent AreaId instead."))
	FName AreaId = TEXT("ItemPlacementSlot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot", meta = (DeprecatedProperty, DeprecationMessage = "Use SlotMeshComponent AreaTags instead."))
	FGameplayTagContainer AreaTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
	TObjectPtr<UStaticMesh> SlotMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
	TObjectPtr<UMaterialInterface> SlotMeshMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
	FTransform SlotMeshRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
	FTransform AttachRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
	FName AttachSocketName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Placement Slot")
	mutable TObjectPtr<AActor> PlacedActor = nullptr;

private:
	bool SanitizeAndCheckOccupied() const;
	UStaticMesh* ResolveClassDefaultSlotMesh() const;
	void ApplySlotAuthoringSettings();
	void RefreshSlotVisualState();
	void RequestHostPartFocusRebuild() const;
	void RequestHostItemUseAreaRebuild() const;
};
