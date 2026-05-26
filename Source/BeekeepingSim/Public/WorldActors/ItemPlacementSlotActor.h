#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusProvider.h"
#include "Focus/ItemUseAreaProvider.h"
#include "GameFramework/Actor.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "ItemPlacementSlotActor.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AItemPlacementSlotActor : public AActor, public IItemUseAreaProvider, public IItemPlacementSlot, public ICursorPartFocusProvider
{
	GENERATED_BODY()

public:
	AItemPlacementSlotActor();
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetItemUseAreaDescriptors_Implementation(TArray<FItemUseAreaDescriptor>& OutDescriptors) const override;
	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;
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
	TObjectPtr<UStaticMeshComponent> SlotMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> AttachComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
	FName AreaId = TEXT("ItemPlacementSlot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Placement Slot")
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
