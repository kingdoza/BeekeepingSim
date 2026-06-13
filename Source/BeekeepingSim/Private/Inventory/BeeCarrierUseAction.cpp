#include "Inventory/BeeCarrierUseAction.h"

#include "GameplayTagContainer.h"
#include "WorldActors/BeeSwarmClusterActor.h"

UBeeCarrierUseAction::UBeeCarrierUseAction()
{
	const FGameplayTag BeeCarrierAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.SwarmCluster.BeeCarrier")), false);
	if (BeeCarrierAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(BeeCarrierAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool UBeeCarrierUseAction::CanBeginUse(const FItemActionContext& Context) const
{
	if (!Super::CanBeginUse(Context))
	{
		return false;
	}

	const ABeeSwarmClusterActor* ClusterActor = ResolveTargetCluster(Context);
	return ClusterActor && !ClusterActor->IsCaptured() && Context.bHasItemUseAreaHit && Context.ItemUseAreaHitComponent;
}

void UBeeCarrierUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	ResetDragState();
	Super::EndUse(Context, bWasCanceled);
}

bool UBeeCarrierUseAction::CanApplyUseEffect(const FItemActionContext& Context) const
{
	if (!Super::CanApplyUseEffect(Context))
	{
		return false;
	}

	const ABeeSwarmClusterActor* ClusterActor = ResolveTargetCluster(Context);
	return ClusterActor && !ClusterActor->IsCaptured();
}

FItemActionExecutionResult UBeeCarrierUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	FItemActionExecutionResult Result;
	ABeeSwarmClusterActor* ClusterActor = ResolveTargetCluster(Context);
	if (!ClusterActor || ClusterActor->IsCaptured())
	{
		return Result;
	}

	const float SafeDeltaTime = FMath::Max(0.0f, DeltaTime);
	float DragSpeedCmPerSecond = 0.0f;

	if (Context.bHasItemUseAreaHit)
	{
		if (bHasLastImpactPoint && SafeDeltaTime > KINDA_SMALL_NUMBER)
		{
			DragSpeedCmPerSecond = FVector::Dist(Context.ItemUseAreaImpactPoint, LastImpactPoint) / SafeDeltaTime;
		}

		LastImpactPoint = Context.ItemUseAreaImpactPoint;
		bHasLastImpactPoint = true;
	}
	else
	{
		bHasLastImpactPoint = false;
	}

	const float BonusSpeed = FMath::Max(0.0f, DragSpeedCmPerSecond - FMath::Max(0.0f, MinDragSpeedForBonus));
	float Rate = FMath::Max(0.0f, BaseAliveRadiusDecreasePerSecond)
		+ BonusSpeed * FMath::Max(0.0f, DragSpeedToAliveRadiusDecreaseScale);
	Rate = FMath::Clamp(Rate, 0.0f, FMath::Max(0.0f, MaxAliveRadiusDecreasePerSecond));

	const float ActualDecrease = ClusterActor->DecreaseAliveRadius(Rate * SafeDeltaTime);
	Result.bSucceeded = ActualDecrease > KINDA_SMALL_NUMBER;
	return Result;
}

ABeeSwarmClusterActor* UBeeCarrierUseAction::ResolveTargetCluster(const FItemActionContext& Context) const
{
	return Cast<ABeeSwarmClusterActor>(Context.ItemUseEffectTargetObject);
}

void UBeeCarrierUseAction::ResetDragState()
{
	bHasLastImpactPoint = false;
	LastImpactPoint = FVector::ZeroVector;
}
