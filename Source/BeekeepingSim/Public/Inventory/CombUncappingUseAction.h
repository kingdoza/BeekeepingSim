#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "CombUncappingUseAction.generated.h"

class ABeehiveCombActor;
class UPrimitiveComponent;
enum class EBeehiveCombVisibleFace : uint8;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UCombUncappingUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UCombUncappingUseAction();

	virtual void TickUse(const FItemActionContext& Context, float DeltaTime) override;
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual bool CanApplyUseEffect(const FItemActionContext& Context) const override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping", meta = (ClampMin = "0.0"))
	float BrushRadiusCm = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping", meta = (ClampMin = "0.0"))
	float MinStampInterval = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping", meta = (ClampMin = "0.0"))
	float MinStampDistanceCm = 4.0f;

private:
	void ResetStampState();
	bool ResolveTargetFace(const ABeehiveCombActor* CombActor, UPrimitiveComponent* HitComponent, EBeehiveCombVisibleFace& OutFace) const;
	void RebuildHostItemUseAreaDescriptors(const FItemActionContext& Context) const;

	bool bHasLastStamp = false;
	FVector LastStampWorldPoint = FVector::ZeroVector;
	float TimeSinceLastStamp = 0.0f;
};
