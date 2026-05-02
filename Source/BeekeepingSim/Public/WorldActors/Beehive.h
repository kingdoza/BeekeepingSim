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
class USceneComponent;
class USplineComponent;
class UStaticMeshComponent;
class ABeekeeperCharacter;
class ABeehiveDualSwarmActor;

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

private:
	static float NormalizeHour24(float Hour24);
	static float EvaluateActivity(const FBeehiveDirectionalSwarmSettings& Settings, float Hour24);
	FBeehiveDualSwarmNiagaraParameters BuildDualSwarmParameters() const;
	void ApplySettingsToDualSwarmChildActor();
	void EnsureDualSwarmChildActorClass();

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
