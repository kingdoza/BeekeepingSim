// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Environment/GameTimeBucketListener.h"
#include "GameFramework/Actor.h"
#include "Focus/FocusInteractable.h"
#include "Focus/ItemUseAreaProvider.h"
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
class AQueenBeeActor;
class UCursorPartFocusScopeComponent;
class UCursorPartFocusActionComponent;
class UBeehiveCombLiftComponent;
class UCursorItemUseAreaScopeComponent;

UCLASS()
class BEEKEEPINGSIM_API ABeehive : public AActor, public IFocusInteractable, public IGameTimeBucketListener, public IItemUseAreaProvider
{
	GENERATED_BODY()

public:
	ABeehive();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
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

	virtual void GetItemUseAreaDescriptors_Implementation(TArray<FItemUseAreaDescriptor>& OutDescriptors) const override;

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

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Beehive|Comb")
	void IncreaseCurrentCombCountForTest();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Beehive|Comb")
	void DecreaseCurrentCombCountForTest();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetCurrentCombCountForTest(int32 NewCount);

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

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey Production")
	float CalculateTotalHoneyIncreaseAmount() const;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Item Use Area")
	TObjectPtr<UCursorItemUseAreaScopeComponent> ItemUseAreaScope;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production Time", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 HoneyProductionBucketMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production Time")
	bool bApplyHoneyProductionOnBeginPlayBucket = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production", meta = (ClampMin = "0.0"))
	float HoneyProductionCoefficient = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoneyDistributionDeviationRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 MaxCombCount = 6;

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
	bool bCombCountInitialized = false;

	UPROPERTY(Transient)
	int32 LastAppliedAttractionSwarmSpawnAmount = INDEX_NONE;

private:
	static float NormalizeHour24(float Hour24);
	static float EvaluateActivity(const FBeehiveDirectionalSwarmSettings& Settings, float Hour24);
	FBeehiveDualSwarmNiagaraParameters BuildDualSwarmParameters() const;
	void ApplySettingsToDualSwarmChildActor();
	void EnsureDualSwarmChildActorClass();
	void EnsureQueenBeeChildActorClass();
	bool ChooseQueenBeeCombSlotIndex(int32& OutSlotIndex) const;
	float CalculateQueenBeeCombSlotWeight(int32 SlotIndex) const;
	USceneComponent* ResolveQueenBeeAttachPoint(int32 SlotIndex, bool bFrontFace) const;
	void DistributeHoneyIncreaseToCombs(float TotalHoneyIncrease);
	void RefreshCombLayoutAndParameters();
	void RefreshCombSlotComponents();
	void RefreshCombSlotTransforms();
	void RefreshCombSpawnAmounts(bool bSkipLiftedComb = false);
	void ClampCurrentCombCount();
	void RegisterCombPartsToScope();
	void BindCombPartFocusActionDelegates(ABeehiveCombActor* CombActor, UCursorPartFocusActionComponent* ActionComponent);
	bool IsManagedActiveCombActor(const ABeehiveCombActor* CombActor) const;
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
