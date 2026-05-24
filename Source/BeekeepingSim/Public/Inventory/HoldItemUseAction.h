#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/ItemAction.h"
#include "HoldItemUseAction.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UHoldItemUseAction : public UItemAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Item Action|Use Area")
	virtual FGameplayTagQuery GetUseAreaTagQuery() const;

	UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
	virtual bool CanBeginUse(const FItemActionContext& Context) const;

	UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
	virtual bool BeginUse(const FItemActionContext& Context);

	UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
	virtual void TickUse(const FItemActionContext& Context, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled);

	UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
	virtual bool CanApplyUseEffect(const FItemActionContext& Context) const;

	UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action|Use Area")
	FGameplayTagQuery UseAreaTagQuery;
};
