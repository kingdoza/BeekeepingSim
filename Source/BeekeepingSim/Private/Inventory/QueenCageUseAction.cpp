#include "Inventory/QueenCageUseAction.h"

#include "GameplayTagContainer.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/QueenCageItemDefinition.h"
#include "WorldActors/QueenBeeActor.h"
#include "WorldActors/QueenBeeCaptureSource.h"

UQueenCageUseAction::UQueenCageUseAction()
{
	const FGameplayTag QueenCageAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.QueenBee.QueenCage")), false);
	if (QueenCageAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(QueenCageAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool UQueenCageUseAction::CanBeginUse(const FItemActionContext& Context) const
{
	return Super::CanBeginUse(Context) && CanCaptureWithContext(Context);
}

bool UQueenCageUseAction::CanApplyUseEffect(const FItemActionContext& Context) const
{
	return Super::CanApplyUseEffect(Context) && CanCaptureWithContext(Context);
}

FItemActionExecutionResult UQueenCageUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	(void)DeltaTime;

	FItemActionExecutionResult Result;
	UItemInstance* SourceQueenCage = ResolveSourceQueenCage(Context);
	AQueenBeeActor* QueenBee = ResolveTargetQueenBee(Context);
	AActor* CaptureSourceActor = Context.FocusEngagedHostActor;
	if (!SourceQueenCage
		|| !QueenBee
		|| QueenBee->IsCaptured()
		|| !CaptureSourceActor
		|| !CaptureSourceActor->GetClass()->ImplementsInterface(UQueenBeeCaptureSource::StaticClass()))
	{
		return Result;
	}

	FQueenCageItemState CapturedState;
	if (!IQueenBeeCaptureSource::Execute_CaptureQueenBee(CaptureSourceActor, QueenBee, CapturedState))
	{
		return Result;
	}

	SourceQueenCage->SetQueenCageState(CapturedState);
	Result.bSucceeded = SourceQueenCage->HasCapturedQueen();
	return Result;
}

UItemInstance* UQueenCageUseAction::ResolveSourceQueenCage(const FItemActionContext& Context) const
{
	UItemInstance* SourceItemInstance = Context.SourceItemInstance;
	if (!SourceItemInstance
		|| !Cast<UQueenCageItemDefinition>(SourceItemInstance->GetDefinition())
		|| !SourceItemInstance->CanAcceptQueenBee())
	{
		return nullptr;
	}

	return SourceItemInstance;
}

AQueenBeeActor* UQueenCageUseAction::ResolveTargetQueenBee(const FItemActionContext& Context) const
{
	return Cast<AQueenBeeActor>(Context.ItemUseEffectTargetObject);
}

bool UQueenCageUseAction::CanCaptureWithContext(const FItemActionContext& Context) const
{
	const UItemInstance* SourceQueenCage = ResolveSourceQueenCage(Context);
	AQueenBeeActor* QueenBee = ResolveTargetQueenBee(Context);
	AActor* CaptureSourceActor = Context.FocusEngagedHostActor;
	if (!SourceQueenCage
		|| !QueenBee
		|| QueenBee->IsCaptured()
		|| !CaptureSourceActor
		|| !CaptureSourceActor->GetClass()->ImplementsInterface(UQueenBeeCaptureSource::StaticClass())
		|| !Context.bHasItemUseAreaHit
		|| !Context.ItemUseAreaHitComponent)
	{
		return false;
	}

	return IQueenBeeCaptureSource::Execute_CanCaptureQueenBee(CaptureSourceActor, QueenBee);
}
