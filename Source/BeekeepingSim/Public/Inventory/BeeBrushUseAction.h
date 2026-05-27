#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "BeeBrushUseAction.generated.h"

class ABeehiveCombActor;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UBeeBrushUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UBeeBrushUseAction();

	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Brush", meta = (ClampMin = "0.0"))
	float BeeRemovalPerSecond = 30.0f;

	UPROPERTY(Transient)
	float PendingBeeRemoval = 0.0f;

private:
	ABeehiveCombActor* ResolveTargetComb(const FItemActionContext& Context) const;
};

