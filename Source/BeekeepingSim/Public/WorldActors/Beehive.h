// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Environment/GameTimeBucketListener.h"
#include "GameFramework/Actor.h"
#include "Focus/FocusInteractable.h"
#include "GameplayTagContainer.h"
#include "WorldActors/BeeSwarmTypes.h"
#include "Beehive.generated.h"

class UFocusTargetComponent;
class UAnchoredFocusCursorActionComponent;
class UChildActorComponent;
class UNiagaraComponent;
class USceneComponent;
class USplineComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class ABeekeeperCharacter;
class ABeehiveDualSwarmActor;
class ABeehiveCombActor;
class ABeehiveCombSlotActor;
class AQueenBeeActor;
class UCursorPartFocusScopeComponent;
class UCursorPartFocusActionComponent;
class UBeehiveCombLiftComponent;
class UCursorItemUseAreaScopeComponent;
class UItemUseAreaMeshProviderComponent;
class UCursorPartFocusRegistrationComponent;
class UChildCursorPartFocusProviderComponent;
class AItemPlacementSlotActor;
class UPlacedItemRemainingComponent;
class UItemDefinition;
class UPollenPattyItemDefinition;

UENUM(BlueprintType)
enum class EPollenPattyConsumptionSide : uint8
{
	Leftmost,
	Rightmost
};

UCLASS()
class BEEKEEPINGSIM_API ABeehive : public AActor, public IFocusInteractable, public IGameTimeBucketListener
{
	GENERATED_BODY()

public:
	ABeehive();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Beehive|Bee Swarm")
	void ApplyBeeSwarmSettings();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Bee Swarm")
	void ApplyBeeSwarmHour24(float Hour24);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Attraction Swarm")
	void ApplyAttractionSwarmSettings();

	UFUNCTION(BlueprintPure, Category = "Beehive|Attraction Swarm")
	int32 CalculateAttractionSwarmSpawnAmount() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Bee Swarm")
	void SetColonyBeeCount(int32 NewBeeCount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Part Focus")
	void RebuildCursorPartFocusDescriptors();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Part Focus")
	void SetLidOpenForPartFocus(bool bOpen);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void RefreshCombStateFromSlots();

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetCurrentCombCount() const { return CurrentCombCount; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 FindManagedCombSlotIndex(const ABeehiveCombActor* CombActor) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	UChildActorComponent* GetCombSlotComponentByIndex(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	bool GetCombSlotWorldTransformByIndex(int32 Index, FTransform& OutTransform) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	bool BuildCombSlotRestRelativeTransform(int32 Index, FTransform& OutTransform) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	USceneComponent* GetCombLiftTargetRoot() const { return CombLiftTargetRoot; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	ABeehiveCombActor* GetLiftedCombActor() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 CalculateCombSpawnAmount() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceAllCombTargetBeeCountsByConfiguredRatio();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceCombTargetBeeCountByConfiguredRatio(int32 CombIndex);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Queen Bee")
	void UpdateQueenBeeLocation();

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Bee")
	AQueenBeeActor* GetQueenBeeActor() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Bee")
	bool IsQueenBeeAttachedToComb(const ABeehiveCombActor* CombActor) const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Queen Bee")
	bool TryBrushQueenBeeFromCombVisibleFace(ABeehiveCombActor* CombActor);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Colony Population")
	void ApplyColonyPopulationUpdate();

	UFUNCTION(BlueprintPure, Category = "Beehive|Colony Population")
	float CalculateBeeIncreaseAmount() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Colony Population")
	float CalculateBeeDecreaseAmount() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Colony Population")
	float GetItemEggLayingBonus() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Colony Population")
	float GetItemLifespanBonus() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Colony Population")
	float GetTemperatureScore() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Production")
	void ApplyHoneyProductionUpdate();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Ripeness")
	void ApplyHoneyRipenessUpdate();

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey Production")
	float CalculateTotalHoneyIncreaseAmount() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Pollen Patty")
	void ApplyPollenPattyConsumptionUpdate();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Sanitation")
	void IncreaseSanitation(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Sanitation")
	void SetSanitationValue(float NewValue);

	UFUNCTION(BlueprintPure, Category = "Beehive|Sanitation")
	float GetSanitationValue() const { return SanitationValue; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Sanitation")
	float GetSanitationRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Aggression")
	void DecreaseAggression(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Aggression")
	void SetAggressionValue(float NewValue);

	UFUNCTION(BlueprintPure, Category = "Beehive|Aggression")
	float GetAggressionValue() const { return AggressionValue; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Aggression")
	float GetAggressionRatio() const;

	virtual void GetGameTimeBucketSubscriptions_Implementation(TArray<FGameTimeBucketSubscription>& OutSubscriptions) const override;
	virtual void OnGameTimeBucketEvent_Implementation(const FGameTimeBucketEvent& Event) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BeehiveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFocusTargetComponent> FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAnchoredFocusCursorActionComponent> FocusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm")
	TObjectPtr<USplineComponent> SwarmSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive")
	bool bIsLidOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm")
	TObjectPtr<UChildActorComponent> BeehiveSwarmChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Attraction Swarm")
	TObjectPtr<UNiagaraComponent> AttractionSwarmNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Bee")
	TObjectPtr<UChildActorComponent> QueenBeeChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	TObjectPtr<USceneComponent> CombRackRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	TObjectPtr<USceneComponent> CombLiftTargetRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	TObjectPtr<UBeehiveCombLiftComponent> CombLiftComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	TObjectPtr<UCursorPartFocusScopeComponent> CursorPartFocusScope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	TObjectPtr<UCursorPartFocusActionComponent> LidPartFocusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	TObjectPtr<UCursorPartFocusRegistrationComponent> CursorPartFocusRegistration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	TObjectPtr<UChildCursorPartFocusProviderComponent> ChildCursorPartFocusProvider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Item Use Area")
	TObjectPtr<UCursorItemUseAreaScopeComponent> ItemUseAreaScope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Item Use Area")
	TObjectPtr<UItemUseAreaMeshProviderComponent> ItemUseAreaMeshProvider;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm")
	TSubclassOf<ABeehiveDualSwarmActor> BeeSplineSwarmActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm", meta = (ClampMin = "0"))
	int32 ColonyBeeCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm", meta = (ClampMin = "0.0", ClampMax = "24.0", UIMin = "0.0", UIMax = "24.0"))
	float BeeSwarmHour24 = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm Time", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 BeeSwarmBucketMinutes = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm Time")
	bool bApplyBeeSwarmOnBeginPlayBucket = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm")
	FBeehiveDualSwarmCommonSettings DualSwarmCommonSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm")
	FBeehiveDirectionalSwarmSettings OutgoingSwarmSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Bee Swarm")
	FBeehiveDirectionalSwarmSettings IngoingSwarmSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Attraction Swarm")
	FBeehiveAttractionSwarmSettings AttractionSwarmSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	TSubclassOf<ABeehiveCombActor> CombActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	TSubclassOf<ABeehiveCombSlotActor> CombSlotActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Bee")
	TSubclassOf<AQueenBeeActor> QueenBeeActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Bee Time", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 QueenBeeLocationBucketMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Bee Time")
	bool bUpdateQueenBeeLocationOnBeginPlayBucket = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Bee", meta = (ClampMin = "1.0"))
	float QueenBeeCenterWeightMultiplier = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population Time", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 ColonyPopulationBucketMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population Time")
	bool bApplyColonyPopulationOnBeginPlayBucket = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population", meta = (ClampMin = "0.0"))
	float BeeIncreaseCoefficient = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population", meta = (ClampMin = "0.0"))
	float BeeDecreaseCoefficient = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population", meta = (ClampMin = "0.0"))
	float BeeDecreaseAbsoluteAmountPerBucket = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production Time", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 HoneyProductionBucketMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production Time")
	bool bApplyHoneyProductionOnBeginPlayBucket = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production", meta = (ClampMin = "0.0"))
	float HoneyProductionCoefficient = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Ripeness", meta = (ClampMin = "0.0"))
	float HoneyRipenessIncreasePerBucket = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoneyDistributionDeviationRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty Time", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 PollenPattyConsumptionBucketMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty Time")
	bool bApplyPollenPattyConsumptionOnBeginPlayBucket = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty", meta = (ClampMin = "0.0"))
	float PollenPattyConsumptionAmountPerBucket = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty")
	EPollenPattyConsumptionSide PollenPattyConsumptionSide = EPollenPattyConsumptionSide::Leftmost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty")
	FGameplayTagContainer PollenPattyConsumptionAreaTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Sanitation", meta = (ClampMin = "0.0"))
	float MaxSanitationValue = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Sanitation", meta = (ClampMin = "0.0"))
	float SanitationValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Aggression", meta = (ClampMin = "0.0"))
	float MaxAggressionValue = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Aggression", meta = (ClampMin = "0.0"))
	float AggressionValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 MaxCombCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 InitialCombCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	float CombSlotSpacing = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CombTargetBeeCountReduceRatio = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CombSpawnAmountRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	FVector2D CombPlaneSize = FVector2D(100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	FName LidPartComponentTag = TEXT("LidMesh");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	FName LidOutlineComponentTag = TEXT("LidMesh");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	TArray<FName> PreviewOnlyPartComponentTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	FText LidPartDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Part Focus")
	FText LidPartInteractionKeyText;

	UPROPERTY(Transient)
	int32 CurrentCombCount = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UChildActorComponent>> CombSlotComponents;

	UPROPERTY(Transient)
	int32 LastAppliedAttractionSwarmSpawnAmount = INDEX_NONE;

private:
	static float NormalizeHour24(float Hour24);
	static float EvaluateActivity(const FBeehiveDirectionalSwarmSettings& Settings, float Hour24);
	FBeehiveDualSwarmNiagaraParameters BuildDualSwarmParameters() const;
	void ApplySettingsToDualSwarmChildActor();
	void EnsureDualSwarmChildActorClass();
	void EnsureQueenBeeChildActorClass();
	ABeehiveCombSlotActor* GetCombSlotActorByIndex(int32 Index) const;
	int32 GetOccupiedCombCount() const;
	bool ChooseQueenBeeCombSlotIndex(int32& OutSlotIndex) const;
	float CalculateQueenBeeCombSlotWeight(int32 SlotIndex) const;
	USceneComponent* ResolveQueenBeeAttachPoint(int32 SlotIndex, bool bFrontFace) const;
	void DistributeHoneyIncreaseToCombs(float TotalHoneyIncrease);
	void ApplyInitialCombCountToSlots();
	void RefreshCombLayoutAndParameters();
	void ApplyInitialCombSetupForBeginPlay();
	void RefreshCombSlotComponents();
	void RefreshCombSlotTransforms();
	void RefreshCombSpawnAmounts(bool bSkipLiftedComb = false, bool bPreserveTargetRatios = true);
	void ClampCombAuthoringCounts();
	void RefreshCurrentCombCountFromSlots();
	void RegisterCombPartsToScope();
	void RebuildItemUseAreaDescriptorsIfAvailable();
	void BindCombPartFocusActionDelegates(ABeehiveCombActor* CombActor, UCursorPartFocusActionComponent* ActionComponent);
	bool IsManagedActiveCombActor(const ABeehiveCombActor* CombActor) const;
	AItemPlacementSlotActor* FindPollenPattyConsumptionTargetSlot(UPlacedItemRemainingComponent*& OutRemainingComponent) const;
	bool DoesSlotMatchPollenPattyConsumptionTags(const AItemPlacementSlotActor* SlotActor) const;
	const UPollenPattyItemDefinition* ResolveActivePollenPattyItemDefinitionForEggLayingBonus() const;
	const UItemDefinition* ResolvePlacedItemDefinitionForEggLayingBonus(const AActor* OccupiedActor) const;
	UPrimitiveComponent* FindPrimitiveComponentByTag(FName ComponentTag) const;

	UFUNCTION()
	void HandleCombPartFocusBegin(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION()
	void HandleCombPartFocusCancel(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION()
	void HandleCombPartFocusAbort(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusEntered(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusExited(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusConfirmed(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusCanceled(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveLidPartFocusStateChanged(bool bIsOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Part Focus")
	void ReceiveCombPartFocusBegin(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Part Focus")
	void ReceiveCombPartFocusCancel(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Part Focus")
	void ReceiveCombPartFocusAbort(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

public:
	virtual void OnFocusEnter_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusExit_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusConfirm_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusCancel_Implementation(ABeekeeperCharacter* InteractingCharacter) override;
};
