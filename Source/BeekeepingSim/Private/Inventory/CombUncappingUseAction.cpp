#include "Inventory/CombUncappingUseAction.h"

#include "Components/PrimitiveComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "GameplayTagContainer.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/UncappingTable.h"

UCombUncappingUseAction::UCombUncappingUseAction()
{
	const FGameplayTag UncappingAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.UncappingTable.Comb")), false);
	if (UncappingAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(UncappingAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
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

	const ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Context.ItemUseEffectTargetObject);
	if (!CombActor || !Context.bHasItemUseAreaHit || !Context.ItemUseAreaHitComponent)
	{
		return false;
	}

	EBeehiveCombVisibleFace TargetFace;
	return ResolveTargetFace(CombActor, Context.ItemUseAreaHitComponent, TargetFace);
}

FItemActionExecutionResult UCombUncappingUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	(void)DeltaTime;

	FItemActionExecutionResult Result;
	ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Context.ItemUseEffectTargetObject);
	if (!CombActor || !Context.bHasItemUseAreaHit || !Context.ItemUseAreaHitComponent)
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

void UCombUncappingUseAction::RebuildHostItemUseAreaDescriptors(const FItemActionContext& Context) const
{
	if (AUncappingTable* UncappingTable = Cast<AUncappingTable>(Context.FocusEngagedHostActor))
	{
		UncappingTable->RebuildItemUseAreaDescriptors();
	}
}
