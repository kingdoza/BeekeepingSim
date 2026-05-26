#include "Inventory/HoldItemUseAction.h"

FGameplayTagQuery UHoldItemUseAction::GetUseAreaTagQuery() const
{
	return UseAreaTagQuery;
}

bool UHoldItemUseAction::CanBeginUse(const FItemActionContext& Context) const
{
	return ReceiveCanBeginUse(Context);
}

bool UHoldItemUseAction::BeginUse(const FItemActionContext& Context)
{
	return ReceiveBeginUse(Context);
}

void UHoldItemUseAction::TickUse(const FItemActionContext& Context, float DeltaTime)
{
	ReceiveTickUse(Context, DeltaTime);
}

void UHoldItemUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	ReceiveEndUse(Context, bWasCanceled);
}

bool UHoldItemUseAction::CanApplyUseEffect(const FItemActionContext& Context) const
{
	return ReceiveCanApplyUseEffect(Context);
}

FItemActionExecutionResult UHoldItemUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	return ReceiveApplyUseEffect(Context, DeltaTime);
}

bool UHoldItemUseAction::ReceiveCanBeginUse_Implementation(const FItemActionContext& Context) const
{
	return CanExecute(Context);
}

bool UHoldItemUseAction::ReceiveBeginUse_Implementation(const FItemActionContext& Context)
{
	return ReceiveCanBeginUse(Context);
}

void UHoldItemUseAction::ReceiveTickUse_Implementation(const FItemActionContext& Context, float DeltaTime)
{
}

void UHoldItemUseAction::ReceiveEndUse_Implementation(const FItemActionContext& Context, bool bWasCanceled)
{
}

bool UHoldItemUseAction::ReceiveCanApplyUseEffect_Implementation(const FItemActionContext& Context) const
{
	return ReceiveCanBeginUse(Context);
}

FItemActionExecutionResult UHoldItemUseAction::ReceiveApplyUseEffect_Implementation(const FItemActionContext& Context, float DeltaTime)
{
	return Execute(Context);
}
