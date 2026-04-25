#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ItemActionContext.h"
#include "ItemActionTypes.h"
#include "ItemAction.generated.h"

class UItemInstance;

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UItemAction : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Action")
	virtual void InitializeAction(UItemInstance* InOwningItemInstance, const FItemActionSpec& InSpec);

	UFUNCTION(BlueprintPure, Category = "Item Action")
	virtual bool CanExecute(const FItemActionContext& Context) const;

	UFUNCTION(BlueprintCallable, Category = "Item Action")
	virtual FItemActionExecutionResult Execute(const FItemActionContext& Context);

	UFUNCTION(BlueprintPure, Category = "Item Action")
	virtual FText GetActionDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Item Action")
	virtual FGameplayTag GetActionTypeTag() const;

	UFUNCTION(BlueprintPure, Category = "Item Action")
	UItemInstance* GetOwningItemInstance() const { return OwningItemInstance; }

protected:
	UPROPERTY(Transient)
	TObjectPtr<UItemInstance> OwningItemInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	FGameplayTag ActionTag;
};
