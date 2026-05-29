#include "Inventory/BeeBrushUseAction.h"

#include "GameplayTagContainer.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeehiveCombActor.h"

UBeeBrushUseAction::UBeeBrushUseAction()
{
	const FGameplayTag BeeBrushAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.Beehive.BeeBrush")), false);
	if (BeeBrushAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(BeeBrushAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

void UBeeBrushUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	PendingBeeRemoval = 0.0f;
	Super::EndUse(Context, bWasCanceled);
}

FItemActionExecutionResult UBeeBrushUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	FItemActionExecutionResult Result;
	ABeehiveCombActor* CombActor = ResolveTargetComb(Context);
	if (!CombActor)
	{
		return Result;
	}

	if (ABeehive* Beehive = Cast<ABeehive>(Context.FocusEngagedHostActor))
	{
		Result.bSucceeded |= Beehive->TryBrushQueenBeeFromCombVisibleFace(CombActor);
	}

	PendingBeeRemoval += FMath::Max(0.0f, BeeRemovalPerSecond) * FMath::Max(0.0f, DeltaTime);
	const int32 RemoveAmount = FMath::FloorToInt(PendingBeeRemoval);
	if (RemoveAmount <= 0)
	{
		return Result;
	}

	PendingBeeRemoval -= static_cast<float>(RemoveAmount);
	CombActor->ReduceVisibleFaceTargetBeeCountByAmount(RemoveAmount);
	Result.bSucceeded = true;
	return Result;
}

ABeehiveCombActor* UBeeBrushUseAction::ResolveTargetComb(const FItemActionContext& Context) const
{
	return Cast<ABeehiveCombActor>(Context.ItemUseEffectTargetObject);
}
