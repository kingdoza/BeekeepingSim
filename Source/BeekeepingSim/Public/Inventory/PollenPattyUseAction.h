#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemPlacementUseAction.h"
#include "PollenPattyUseAction.generated.h"

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UPollenPattyUseAction : public UItemPlacementUseAction
{
	GENERATED_BODY()

public:
	UPollenPattyUseAction();

	virtual bool BeginUse(const FItemActionContext& Context) override;
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Pollen Patty")
	void ReceivePollenPattyUseStarted(const FItemActionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pollen Patty")
	void ReceivePollenPattyInstalled(const FItemActionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pollen Patty")
	void ReceivePollenPattyInstallBlocked(const FItemActionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pollen Patty")
	void ReceivePollenPattyUseEnded(const FItemActionContext& Context, bool bWasCanceled);
};
