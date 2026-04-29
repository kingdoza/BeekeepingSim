#include "Inventory/ItemAction.h"

#include "Inventory/ItemInstance.h"

void UItemAction::InitializeAction(UItemInstance* InOwningItemInstance, const FItemActionSpec& InSpec)
{
	OwningItemInstance = InOwningItemInstance;
	ActionTag = InSpec.ActionTag;
}

bool UItemAction::CanExecute(const FItemActionContext& Context) const
{
	return OwningItemInstance != nullptr && Context.Character != nullptr;
}

FItemActionExecutionResult UItemAction::Execute(const FItemActionContext& Context)
{
	FItemActionExecutionResult Result;
	Result.bSucceeded = false;
	Result.Message = CanExecute(Context)
		? FText::FromString(TEXT("No item action behavior is implemented."))
		: FText::FromString(TEXT("Item action cannot execute in the current context."));
	return Result;
}

FText UItemAction::GetActionDisplayName() const
{
	return FText::FromName(ActionTag.GetTagName());
}

FGameplayTag UItemAction::GetActionTypeTag() const
{
	return ActionTag;
}
