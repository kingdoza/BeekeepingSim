#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoneyDecantingTable.generated.h"

class AHoneyContainerSlotActor;
class UAnchoredFocusCursorActionComponent;
class UChildActorComponent;
class UChildCursorPartFocusProviderComponent;
class UCursorItemUseAreaScopeComponent;
class UCursorPartFocusRegistrationComponent;
class UCursorPartFocusScopeComponent;
class UFocusTargetComponent;
class UHoneyTransferComponent;
class UItemUseAreaMeshProviderComponent;
class UNiagaraComponent;
class USceneComponent;
class UStaticMeshComponent;

enum class EHoneyTransferState : uint8;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AHoneyDecantingTable : public AActor
{
	GENERATED_BODY()

public:
	AHoneyDecantingTable();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Honey Decanting Table|Part Focus")
	void RebuildCursorPartFocusDescriptors();

	UFUNCTION(BlueprintCallable, Category = "Honey Decanting Table|Item Use Area")
	void RebuildItemUseAreaDescriptors();

	UFUNCTION(BlueprintPure, Category = "Honey Decanting Table|Slots")
	AHoneyContainerSlotActor* GetSourceSlotActor() const;

	UFUNCTION(BlueprintPure, Category = "Honey Decanting Table|Slots")
	AHoneyContainerSlotActor* GetTargetSlotActor() const;

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
	TObjectPtr<USceneComponent> SourceSlotRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> SourceSlotChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> TargetSlotRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> TargetSlotChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHoneyTransferComponent> HoneyTransferComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> HoneyStreamNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Decanting Table|Slots")
	TSubclassOf<AHoneyContainerSlotActor> SourceSlotActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Decanting Table|Slots")
	TSubclassOf<AHoneyContainerSlotActor> TargetSlotActorClass;

private:
	void EnsureSlotChildActorClasses();
	void ConfigureTransferComponent();
	void BindTransferComponentEvents();
	void HandleHoneyTransferStateChanged(EHoneyTransferState OldState, EHoneyTransferState NewState);
};
