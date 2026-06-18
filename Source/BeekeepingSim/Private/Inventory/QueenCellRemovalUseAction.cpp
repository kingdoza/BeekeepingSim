#include "Inventory/QueenCellRemovalUseAction.h"

#include "Focus/ItemUseAreaMeshComponent.h"
#include "GameplayTagContainer.h"
#include "WorldActors/BeehiveCombActor.h"

UQueenCellRemovalUseAction::UQueenCellRemovalUseAction()
{
	const FGameplayTag QueenCellAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.Beehive.QueenCell")), false);
	if (QueenCellAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(QueenCellAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool UQueenCellRemovalUseAction::CanBeginUse(const FItemActionContext& Context) const
{
	return Super::CanBeginUse(Context) && CanRemoveWithContext(Context);
}

bool UQueenCellRemovalUseAction::CanApplyUseEffect(const FItemActionContext& Context) const
{
	return Super::CanApplyUseEffect(Context) && CanRemoveWithContext(Context);
}

FItemActionExecutionResult UQueenCellRemovalUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	(void)DeltaTime;

	FItemActionExecutionResult Result;
	ABeehiveCombActor* CombActor = ResolveTargetComb(Context);
	UItemUseAreaMeshComponent* UseArea = ResolveQueenCellUseArea(Context);
	if (!CombActor || !UseArea)
	{
		return Result;
	}

	FGuid QueenCellId;
	if (!CombActor->ResolveQueenCellIdFromUseArea(UseArea, QueenCellId))
	{
		return Result;
	}

	Result.bSucceeded = CombActor->RemoveQueenCell(QueenCellId);
	return Result;
}

ABeehiveCombActor* UQueenCellRemovalUseAction::ResolveTargetComb(const FItemActionContext& Context) const
{
	if (ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Context.ItemUseEffectTargetObject))
	{
		return CombActor;
	}

	const UItemUseAreaMeshComponent* UseArea = ResolveQueenCellUseArea(Context);
	return UseArea ? Cast<ABeehiveCombActor>(UseArea->GetOwner()) : nullptr;
}

UItemUseAreaMeshComponent* UQueenCellRemovalUseAction::ResolveQueenCellUseArea(const FItemActionContext& Context) const
{
	return Context.bHasItemUseAreaHit ? Cast<UItemUseAreaMeshComponent>(Context.ItemUseAreaHitComponent) : nullptr;
}

bool UQueenCellRemovalUseAction::CanRemoveWithContext(const FItemActionContext& Context) const
{
	ABeehiveCombActor* CombActor = ResolveTargetComb(Context);
	UItemUseAreaMeshComponent* UseArea = ResolveQueenCellUseArea(Context);
	if (!CombActor || !UseArea)
	{
		return false;
	}

	FGuid QueenCellId;
	return CombActor->ResolveQueenCellIdFromUseArea(UseArea, QueenCellId);
}
