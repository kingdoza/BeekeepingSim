#include "Inventory/HoldItemUseAction.h"

#include "Inventory/ActiveUseDurabilityItemDefinition.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"

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

float UHoldItemUseAction::ResolveActiveUseDurabilityDelta(
	const FItemActionContext& Context,
	const FItemActionExecutionResult& EffectResult,
	float DeltaTime,
	bool bIsOverValidUseArea) const
{
	UItemInstance* SourceItem = Context.SourceItemInstance;
	if (!SourceItem)
	{
		return 0.0f;
	}

	const UActiveUseDurabilityItemDefinition* ActiveUseDefinition = Cast<UActiveUseDurabilityItemDefinition>(SourceItem->GetDefinition());
	if (!ActiveUseDefinition)
	{
		return 0.0f;
	}

	const UItemDefinition* SourceDefinition = SourceItem->GetDefinition();
	if (!SourceDefinition
		|| !SourceDefinition->bUsesDurability
		|| SourceDefinition->MaxDurability <= 0.0f
		|| SourceDefinition->MaxStack != 1
		|| ActiveUseDefinition->DurabilityDrainPerSecond <= 0.0f)
	{
		return 0.0f;
	}

	if (SourceItem->GetCurrentDurability() <= 0.0f)
	{
		return 0.0f;
	}

	switch (ActiveUseDefinition->DrainPolicy)
	{
	case EActiveUseDurabilityDrainPolicy::WhileUseSessionActive:
		break;
	case EActiveUseDurabilityDrainPolicy::WhileOverValidUseArea:
		if (!bIsOverValidUseArea)
		{
			return 0.0f;
		}
		break;
	case EActiveUseDurabilityDrainPolicy::WhenUseEffectSucceeded:
	default:
		if (!bIsOverValidUseArea || !EffectResult.bSucceeded)
		{
			return 0.0f;
		}
		break;
	}

	return -ActiveUseDefinition->DurabilityDrainPerSecond * FMath::Max(0.0f, DeltaTime);
}

bool UHoldItemUseAction::HasUsableActiveUseDurability(const FItemActionContext& Context) const
{
	UItemInstance* SourceItem = Context.SourceItemInstance;
	if (!SourceItem)
	{
		return false;
	}

	const UActiveUseDurabilityItemDefinition* ActiveUseDefinition = Cast<UActiveUseDurabilityItemDefinition>(SourceItem->GetDefinition());
	if (!ActiveUseDefinition)
	{
		return true;
	}

	const UItemDefinition* SourceDefinition = SourceItem->GetDefinition();
	if (!SourceDefinition
		|| !SourceDefinition->bUsesDurability
		|| SourceDefinition->MaxDurability <= 0.0f
		|| SourceDefinition->MaxStack != 1
		|| ActiveUseDefinition->DurabilityDrainPerSecond <= 0.0f)
	{
		return false;
	}

	return SourceItem->GetCurrentDurability() > 0.0f;
}

bool UHoldItemUseAction::ReceiveCanBeginUse_Implementation(const FItemActionContext& Context) const
{
	return CanExecute(Context) && HasUsableActiveUseDurability(Context);
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
	return ReceiveCanBeginUse(Context) && HasUsableActiveUseDurability(Context);
}

FItemActionExecutionResult UHoldItemUseAction::ReceiveApplyUseEffect_Implementation(const FItemActionContext& Context, float DeltaTime)
{
	return Execute(Context);
}
