#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "BeeCarrierUseAction.generated.h"

class ABeeSwarmClusterActor;
class UItemInstance;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UBeeCarrierUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UBeeCarrierUseAction();

	virtual bool CanBeginUse(const FItemActionContext& Context) const override;
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual bool CanApplyUseEffect(const FItemActionContext& Context) const override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier|Deprecated", meta = (ClampMin = "0.0", DeprecatedProperty, DeprecationMessage = "Use BaseBeeCapturePerSecond."))
	float BaseAliveRadiusDecreasePerSecond = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier|Deprecated", meta = (ClampMin = "0.0", DeprecatedProperty, DeprecationMessage = "Use DragSpeedToBeeCaptureScale."))
	float DragSpeedToAliveRadiusDecreaseScale = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier|Deprecated", meta = (ClampMin = "0.0", DeprecatedProperty, DeprecationMessage = "Use MaxBeeCapturePerSecond."))
	float MaxAliveRadiusDecreasePerSecond = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
	float BaseBeeCapturePerSecond = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
	float DragSpeedToBeeCaptureScale = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
	float MaxBeeCapturePerSecond = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
	float MinDragSpeedForBonus = 20.0f;

	UPROPERTY(Transient)
	bool bHasLastImpactPoint = false;

	UPROPERTY(Transient)
	FVector LastImpactPoint = FVector::ZeroVector;

private:
	ABeeSwarmClusterActor* ResolveTargetCluster(const FItemActionContext& Context) const;
	UItemInstance* ResolveSourceBeeCarrier(const FItemActionContext& Context) const;
	void ResetDragState();
};
