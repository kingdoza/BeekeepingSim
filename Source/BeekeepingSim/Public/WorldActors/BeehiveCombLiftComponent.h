#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BeehiveCombLiftComponent.generated.h"

class ABeehive;
class ABeehiveCombActor;
class ABeekeeperCharacter;
class UChildActorComponent;
class USceneComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeehiveCombLiftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBeehiveCombLiftComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Lift")
	bool LiftComb(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Lift")
	bool ReturnComb(ABeehiveCombActor* CombActor);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Lift")
	void AbortCombLift(ABeehiveCombActor* CombActor);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Lift")
	void ReturnAllLiftedCombs();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Lift")
	void ReapplyLiftedCombTransformAfterLayoutRefresh();

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb Lift")
	int32 GetLiftedCombSlotIndex() const { return LiftedCombSlotIndex; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Lift", meta = (ClampMin = "0.0"))
	float CombLiftMoveDuration = 0.25f;

private:
	struct FCombLiftMoveTask
	{
		TWeakObjectPtr<UChildActorComponent> SlotComponent;
		FTransform StartRelativeTransform = FTransform::Identity;
		FTransform TargetRelativeTransform = FTransform::Identity;
		float Elapsed = 0.0f;
		float Duration = 0.0f;
		bool bActive = false;
		bool bClearLiftedIndexOnComplete = false;
		int32 SlotIndex = INDEX_NONE;
	};

	ABeehive* GetOwnerBeehive() const;
	bool BuildLiftTargetRelativeTransform(int32 SlotIndex, ABeekeeperCharacter* InteractingCharacter, FTransform& OutTransform);
	bool BuildLiftTargetRelativeTransformFromStoredRotation(int32 SlotIndex, FTransform& OutTransform) const;
	void StartMoveTask(UChildActorComponent* SlotComponent, int32 SlotIndex, const FTransform& StartRelativeTransform, const FTransform& TargetRelativeTransform, bool bClearLiftedIndexOnComplete);
	void ApplyRelativeTransformImmediately(UChildActorComponent* SlotComponent, int32 SlotIndex, const FTransform& TargetRelativeTransform, bool bClearLiftedIndexNow);
	void StopMoveTask();
	void AbortAllLiftedCombsImmediately();

	UPROPERTY(Transient)
	int32 LiftedCombSlotIndex = INDEX_NONE;

	FCombLiftMoveTask ActiveMoveTask;
};
