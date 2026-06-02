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

	UFUNCTION(BlueprintPure, Category = "Item Action|Use Area")
	virtual float ResolveActiveUseDurabilityDelta(
		const FItemActionContext& Context,
		const FItemActionExecutionResult& EffectResult,
		float DeltaTime,
		bool bIsOverValidUseArea) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action|Use Area")
	FGameplayTagQuery UseAreaTagQuery;

	bool HasUsableActiveUseDurability(const FItemActionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Item Action|Use Area")
	bool ReceiveCanBeginUse(const FItemActionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Item Action|Use Area")
	bool ReceiveBeginUse(const FItemActionContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Item Action|Use Area")
	void ReceiveTickUse(const FItemActionContext& Context, float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "Item Action|Use Area")
	void ReceiveEndUse(const FItemActionContext& Context, bool bWasCanceled);

	UFUNCTION(BlueprintNativeEvent, Category = "Item Action|Use Area")
	bool ReceiveCanApplyUseEffect(const FItemActionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Item Action|Use Area")
	FItemActionExecutionResult ReceiveApplyUseEffect(const FItemActionContext& Context, float DeltaTime);
};
