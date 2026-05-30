#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlacedItemRemainingComponent.generated.h"

class AActor;
class UItemDefinition;
class UItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlacedItemRemainingRatioChangedSignature, float, Ratio);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacedItemRemainingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining")
	void InitializeFromPlacement(UItemInstance* SourceItemInstance, UItemDefinition* ReturnItemDefinition, AActor* OwningPlacementSlotActor);

	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining")
	void DeactivateRemaining();

	UFUNCTION(BlueprintPure, Category = "Placed Item|Remaining")
	bool HasRemaining() const { return bHasRemaining; }

	UFUNCTION(BlueprintPure, Category = "Placed Item|Remaining")
	float GetCurrentAmount() const { return CurrentAmount; }

	UFUNCTION(BlueprintPure, Category = "Placed Item|Remaining")
	float GetMaxAmount() const { return MaxAmount; }

	UFUNCTION(BlueprintPure, Category = "Placed Item|Remaining")
	float GetRemainingRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining")
	void SetCurrentAmount(float NewAmount);

	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining")
	float ConsumeAmount(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining")
	void WriteBackToItemInstance(UItemInstance* TargetItemInstance) const;

	UPROPERTY(BlueprintAssignable, Category = "Placed Item|Remaining")
	FPlacedItemRemainingRatioChangedSignature OnRemainingRatioChanged;

private:
	void ApplyCurrentAmount(float NewAmount, bool bHandleDepletion);
	void TryClearOwningSlotWhenDepleted();
	void BroadcastRemainingRatioChanged();

	UPROPERTY(VisibleAnywhere, Category = "Placed Item|Remaining")
	bool bHasRemaining = false;

	UPROPERTY(VisibleAnywhere, Category = "Placed Item|Remaining")
	float CurrentAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Placed Item|Remaining")
	float MaxAmount = 0.0f;

	UPROPERTY(Transient)
	bool bClearOwningSlotWhenDepleted = false;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwningPlacementSlotActor = nullptr;
};

