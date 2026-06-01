#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "SmokerUseAction.generated.h"

class ABeehive;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API USmokerUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	USmokerUseAction();

	virtual bool BeginUse(const FItemActionContext& Context) override;
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Smoker")
	void ReceiveSmokerUseStarted(const FItemActionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Smoker")
	void ReceiveSmokerUseEnded(const FItemActionContext& Context, bool bWasCanceled);

private:
	ABeehive* ResolveTargetBeehive(const FItemActionContext& Context) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoker", meta = (ClampMin = "0.0"))
	float AggressionDecreasePerSecond = 10.0f;
};
