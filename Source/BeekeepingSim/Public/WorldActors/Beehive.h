// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Environment/GameTimeBucketListener.h"
#include "GameFramework/Actor.h"
#include "Focus/FocusInteractable.h"
#include "WorldActors/BeeSwarmTypes.h"
#include "Beehive.generated.h"

class UFocusTargetComponent;
class UAnchoredFocusCursorActionComponent;
class UChildActorComponent;
class UNiagaraComponent;
class USceneComponent;
class USplineComponent;
class UStaticMeshComponent;
class ABeekeeperCharacter;
class ABeehiveDualSwarmActor;
class ABeehiveCombActor;

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

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetCurrentCombCount() const { return CurrentCombCount; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Comb")
	TObjectPtr<USceneComponent> CombRackRoot;

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

	UPROPERTY(Transient)
	int32 CurrentCombCount = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UChildActorComponent>> CombSlotComponents;

	UPROPERTY(Transient)
	bool bCombCountInitialized = false;

private:
	static float NormalizeHour24(float Hour24);
	static float EvaluateActivity(const FBeehiveDirectionalSwarmSettings& Settings, float Hour24);
	FBeehiveDualSwarmNiagaraParameters BuildDualSwarmParameters() const;
	void ApplySettingsToDualSwarmChildActor();
	void EnsureDualSwarmChildActorClass();
	void RefreshCombLayoutAndParameters();
	void RefreshCombSlotComponents();
	void RefreshCombSlotTransforms();
	void RefreshCombSpawnAmounts();
	void ClampCurrentCombCount();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusEntered(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusExited(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusConfirmed(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusCanceled(ABeekeeperCharacter* InteractingCharacter);

public:
	virtual void OnFocusEnter_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusExit_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusConfirm_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusCancel_Implementation(ABeekeeperCharacter* InteractingCharacter) override;
};
