#include "Inventory/ItemPlacementUseAction.h"

#include "WorldActors/ItemPlacementSlot.h"

UItemPlacementUseAction::UItemPlacementUseAction()
{
}

FItemActionExecutionResult UItemPlacementUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	(void)DeltaTime;

	FItemActionExecutionResult Result;
	if (!PlacedActorClass || !Context.ItemUseEffectTargetObject)
	{
		return Result;
	}

	UObject* EffectTargetObject = Context.ItemUseEffectTargetObject;
	if (!EffectTargetObject->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return Result;
	}

	const bool bPlaced = IItemPlacementSlot::Execute_TryPlaceItem(EffectTargetObject, PlacedActorClass, Context.SourceItemInstance, Context.Character);
	if (!bPlaced)
	{
		return Result;
	}

	Result.bSucceeded = true;
	Result.bConsumedItem = true;
	Result.StackDelta = -1;
	return Result;
}
