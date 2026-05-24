#include "Inventory/HoldItemUseAction.h"

FGameplayTagQuery UHoldItemUseAction::GetUseAreaTagQuery() const
{
	return UseAreaTagQuery;
}

bool UHoldItemUseAction::CanBeginUse(const FItemActionContext& Context) const
{
	return CanExecute(Context);
}

bool UHoldItemUseAction::BeginUse(const FItemActionContext& Context)
{
	return CanBeginUse(Context);
}

void UHoldItemUseAction::TickUse(const FItemActionContext& Context, float DeltaTime)
{
}

void UHoldItemUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
}

bool UHoldItemUseAction::CanApplyUseEffect(const FItemActionContext& Context) const
{
	return CanBeginUse(Context);
}

FItemActionExecutionResult UHoldItemUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	return Execute(Context);
}
