#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UncappingTable.generated.h"

class AUncappingTableCombSlot;
class UAnchoredFocusCursorActionComponent;
class UChildActorComponent;
class UChildCursorPartFocusProviderComponent;
class UCursorItemUseAreaScopeComponent;
class UCursorPartFocusRegistrationComponent;
class UCursorPartFocusScopeComponent;
class UFocusTargetComponent;
class UItemUseAreaMeshProviderComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AUncappingTable : public AActor
{
	GENERATED_BODY()

public:
	AUncappingTable();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Uncapping Table|Part Focus")
	void RebuildCursorPartFocusDescriptors();

	UFUNCTION(BlueprintCallable, Category = "Uncapping Table|Item Use Area")
	void RebuildItemUseAreaDescriptors();

	UFUNCTION(BlueprintPure, Category = "Uncapping Table|Comb Slot")
	AUncappingTableCombSlot* GetCombSlotActor() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TableMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FocusAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> CharacterAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFocusTargetComponent> FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAnchoredFocusCursorActionComponent> FocusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCursorPartFocusScopeComponent> CursorPartFocusScope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCursorPartFocusRegistrationComponent> CursorPartFocusRegistration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildCursorPartFocusProviderComponent> ChildCursorPartFocusProvider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCursorItemUseAreaScopeComponent> ItemUseAreaScope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshProviderComponent> ItemUseAreaMeshProvider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> CombSlotRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> CombSlotChildActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Uncapping Table|Comb Slot")
	TSubclassOf<AUncappingTableCombSlot> CombSlotActorClass;

private:
	void EnsureCombSlotChildActorClass();
};
