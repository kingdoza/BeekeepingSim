#include "Inventory/SmokerUseAction.h"

#include "Character/BeekeeperCharacter.h"
#include "Character/BeekeeperHeldItemVisualizerComponent.h"
#include "GameplayTagContainer.h"
#include "WorldActors/Beehive.h"

USmokerUseAction::USmokerUseAction()
{
	const FGameplayTag SmokerAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.Beehive.Smoker")), false);
	if (SmokerAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(SmokerAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool USmokerUseAction::BeginUse(const FItemActionContext& Context)
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

	ReceiveSmokerUseStarted(Context);
	return true;
}

void USmokerUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	if (ABeekeeperCharacter* Character = Context.Character)
	{
		if (UBeekeeperHeldItemVisualizerComponent* HeldItemVisualizer = Character->GetBeekeeperHeldItemVisualizer())
		{
			HeldItemVisualizer->EndHeldItemUseActive(bWasCanceled);
		}
	}

	ReceiveSmokerUseEnded(Context, bWasCanceled);
	Super::EndUse(Context, bWasCanceled);
}

FItemActionExecutionResult USmokerUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	FItemActionExecutionResult Result;
	ABeehive* Beehive = ResolveTargetBeehive(Context);
	if (!Beehive)
	{
		return Result;
	}

	const float DecreaseAmount = FMath::Max(0.0f, AggressionDecreasePerSecond) * FMath::Max(0.0f, DeltaTime);
	Beehive->DecreaseAggression(DecreaseAmount);
	Result.bSucceeded = true;
	return Result;
}

ABeehive* USmokerUseAction::ResolveTargetBeehive(const FItemActionContext& Context) const
{
	if (ABeehive* ByEffectTarget = Cast<ABeehive>(Context.ItemUseEffectTargetObject))
	{
		return ByEffectTarget;
	}

	return Cast<ABeehive>(Context.FocusEngagedHostActor);
}
