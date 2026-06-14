#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "QueenCageUseAction.generated.h"

class AQueenBeeActor;
class UItemInstance;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UQueenCageUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UQueenCageUseAction();

	virtual bool CanBeginUse(const FItemActionContext& Context) const override;
	virtual bool CanApplyUseEffect(const FItemActionContext& Context) const override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

private:
	UItemInstance* ResolveSourceQueenCage(const FItemActionContext& Context) const;
	AQueenBeeActor* ResolveTargetQueenBee(const FItemActionContext& Context) const;
	bool CanCaptureWithContext(const FItemActionContext& Context) const;
};
