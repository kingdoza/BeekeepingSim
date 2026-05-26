#include "Inventory/DisinfectantUseAction.h"

#include "Character/BeekeeperCharacter.h"
#include "Character/BeekeeperHeldItemVisualizerComponent.h"
#include "WorldActors/Beehive.h"
#include "GameplayTagContainer.h"

UDisinfectantUseAction::UDisinfectantUseAction()
{
	const FGameplayTag DisinfectantAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.Beehive.Disinfectant")), false);
	if (DisinfectantAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(DisinfectantAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool UDisinfectantUseAction::BeginUse(const FItemActionContext& Context)
{
	if (!Super::BeginUse(Context))
	{
		return false;
	}

	if (ABeekeeperCharacter* Character = Context.Character)
	{
		if (UBeekeeperHeldItemVisualizerComponent* HeldItemVisualizer = Character->GetBeekeeperHeldItemVisualizer())
		{
			HeldItemVisualizer->BeginHeldItemUseActive();
		}
	}

	ReceiveDisinfectantUseStarted(Context);
	return true;
}

void UDisinfectantUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	if (ABeekeeperCharacter* Character = Context.Character)
	{
		if (UBeekeeperHeldItemVisualizerComponent* HeldItemVisualizer = Character->GetBeekeeperHeldItemVisualizer())
		{
			HeldItemVisualizer->EndHeldItemUseActive(bWasCanceled);
		}
	}

	ReceiveDisinfectantUseEnded(Context, bWasCanceled);
	Super::EndUse(Context, bWasCanceled);
}

FItemActionExecutionResult UDisinfectantUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	FItemActionExecutionResult Result;
	ABeehive* Beehive = ResolveTargetBeehive(Context);
	if (!Beehive)
	{
		return Result;
	}

	const float IncreaseAmount = FMath::Max(0.0f, SanitationIncreasePerSecond) * FMath::Max(0.0f, DeltaTime);
	Beehive->IncreaseSanitation(IncreaseAmount);
	Result.bSucceeded = true;
	return Result;
}

ABeehive* UDisinfectantUseAction::ResolveTargetBeehive(const FItemActionContext& Context) const
{
	if (ABeehive* ByEffectTarget = Cast<ABeehive>(Context.ItemUseEffectTargetObject))
	{
		return ByEffectTarget;
	}

	return Cast<ABeehive>(Context.FocusEngagedHostActor);
}
