#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "DisinfectantUseAction.generated.h"

class ABeehive;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UDisinfectantUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UDisinfectantUseAction();

	virtual bool BeginUse(const FItemActionContext& Context) override;
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Disinfectant")
	void ReceiveDisinfectantUseStarted(const FItemActionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Disinfectant")
	void ReceiveDisinfectantUseEnded(const FItemActionContext& Context, bool bWasCanceled);

private:
	ABeehive* ResolveTargetBeehive(const FItemActionContext& Context) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Disinfectant", meta = (ClampMin = "0.0"))
	float SanitationIncreasePerSecond = 10.0f;
};
