#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "QueenCellRemovalUseAction.generated.h"

class ABeehiveCombActor;
class UItemUseAreaMeshComponent;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UQueenCellRemovalUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UQueenCellRemovalUseAction();

	virtual bool CanBeginUse(const FItemActionContext& Context) const override;
	virtual bool CanApplyUseEffect(const FItemActionContext& Context) const override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

private:
	ABeehiveCombActor* ResolveTargetComb(const FItemActionContext& Context) const;
	UItemUseAreaMeshComponent* ResolveQueenCellUseArea(const FItemActionContext& Context) const;
	bool CanRemoveWithContext(const FItemActionContext& Context) const;
};
