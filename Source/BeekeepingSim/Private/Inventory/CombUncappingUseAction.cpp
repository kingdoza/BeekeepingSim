#include "Inventory/CombUncappingUseAction.h"

#include "Components/PrimitiveComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "GameplayTagContainer.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/UncappingTable.h"
#include "WorldActors/UncappingTableCombSlot.h"

UCombUncappingUseAction::UCombUncappingUseAction()
{
	const FGameplayTag UncappingAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.UncappingTable.HoneyComb")), false);
	if (UncappingAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(UncappingAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool UCombUncappingUseAction::CanBeginUse(const FItemActionContext& Context) const
{
	if (!Super::CanBeginUse(Context))
	{
		return false;
	}

	return CanUseOnCurrentUncappingTableContext(Context);
}

void UCombUncappingUseAction::TickUse(const FItemActionContext& Context, float DeltaTime)
{
	TimeSinceLastStamp += FMath::Max(0.0f, DeltaTime);
	Super::TickUse(Context, DeltaTime);
}

void UCombUncappingUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	ResetStampState();
	Super::EndUse(Context, bWasCanceled);
}

bool UCombUncappingUseAction::CanApplyUseEffect(const FItemActionContext& Context) const
{
	if (!Super::CanApplyUseEffect(Context))
	{
		return false;
	}

	return CanUseOnCurrentUncappingTableContext(Context);
}

FItemActionExecutionResult UCombUncappingUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	(void)DeltaTime;

	FItemActionExecutionResult Result;
	ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Context.ItemUseEffectTargetObject);
	if (!CombActor || !CanUseOnCurrentUncappingTableContext(Context))
	{
		return Result;
	}

	EBeehiveCombVisibleFace TargetFace;
	if (!ResolveTargetFace(CombActor, Context.ItemUseAreaHitComponent, TargetFace))
	{
		return Result;
	}

	if (bHasLastStamp)
	{
		const bool bIntervalSatisfied = TimeSinceLastStamp >= FMath::Max(0.0f, MinStampInterval);
		const bool bDistanceSatisfied = FVector::DistSquared(Context.ItemUseAreaImpactPoint, LastStampWorldPoint)
			>= FMath::Square(FMath::Max(0.0f, MinStampDistanceCm));
		if (!bIntervalSatisfied || !bDistanceSatisfied)
		{
			return Result;
		}
	}

	const bool bWasFaceComplete = CombActor->IsWaxCappingFaceComplete(TargetFace);
	bHasLastStamp = true;
	LastStampWorldPoint = Context.ItemUseAreaImpactPoint;
	TimeSinceLastStamp = 0.0f;

	const bool bMaskChanged = CombActor->ApplyWaxCappingBrush(
		Context.ItemUseAreaHitComponent,
		Context.ItemUseAreaImpactPoint,
		BrushRadiusCm);
	Result.bSucceeded = bMaskChanged;

	if (bMaskChanged && !bWasFaceComplete && CombActor->IsWaxCappingFaceComplete(TargetFace))
	{
		RebuildHostItemUseAreaDescriptors(Context);
	}

	return Result;
}

void UCombUncappingUseAction::ResetStampState()
{
	bHasLastStamp = false;
	LastStampWorldPoint = FVector::ZeroVector;
	TimeSinceLastStamp = 0.0f;
}

bool UCombUncappingUseAction::ResolveTargetFace(
	const ABeehiveCombActor* CombActor,
	UPrimitiveComponent* HitComponent,
	EBeehiveCombVisibleFace& OutFace) const
{
	if (!CombActor || !HitComponent)
	{
		return false;
	}

	if (HitComponent == CombActor->GetFrontWaxCappingUseAreaMesh())
	{
		OutFace = EBeehiveCombVisibleFace::Front;
		return true;
	}

	if (HitComponent == CombActor->GetBackWaxCappingUseAreaMesh())
	{
		OutFace = EBeehiveCombVisibleFace::Back;
		return true;
	}

	return false;
}

bool UCombUncappingUseAction::CanUseOnCurrentUncappingTableContext(const FItemActionContext& Context) const
{
	const AUncappingTable* UncappingTable = Cast<AUncappingTable>(Context.FocusEngagedHostActor);
	const AUncappingTableCombSlot* CombSlot = UncappingTable ? UncappingTable->GetCombSlotActor() : nullptr;
	if (!UncappingTable || !CombSlot || CombSlot->IsCombPartFocusEngaged())
	{
		return false;
	}

	const ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Context.ItemUseEffectTargetObject);
	if (!CombActor || !Context.bHasItemUseAreaHit || !Context.ItemUseAreaHitComponent)
	{
		return false;
	}

	EBeehiveCombVisibleFace TargetFace;
	return ResolveTargetFace(CombActor, Context.ItemUseAreaHitComponent, TargetFace);
}

void UCombUncappingUseAction::RebuildHostItemUseAreaDescriptors(const FItemActionContext& Context) const
{
	if (AUncappingTable* UncappingTable = Cast<AUncappingTable>(Context.FocusEngagedHostActor))
	{
		UncappingTable->RebuildItemUseAreaDescriptors();
	}
}
