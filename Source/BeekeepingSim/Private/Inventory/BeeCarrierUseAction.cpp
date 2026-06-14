#include "Inventory/BeeCarrierUseAction.h"

#include "GameplayTagContainer.h"
#include "Inventory/BeeCarrierItemDefinition.h"
#include "Inventory/ItemInstance.h"
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
	const UItemInstance* SourceBeeCarrier = ResolveSourceBeeCarrier(Context);
	return ClusterActor
		&& !ClusterActor->IsBeesCaptured()
		&& ClusterActor->GetRemainingBeeAmount() > KINDA_SMALL_NUMBER
		&& SourceBeeCarrier
		&& SourceBeeCarrier->GetBeeCarrierFreeCapacity() > KINDA_SMALL_NUMBER
		&& Context.bHasItemUseAreaHit
		&& Context.ItemUseAreaHitComponent;
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
	const UItemInstance* SourceBeeCarrier = ResolveSourceBeeCarrier(Context);
	return ClusterActor
		&& !ClusterActor->IsBeesCaptured()
		&& ClusterActor->GetRemainingBeeAmount() > KINDA_SMALL_NUMBER
		&& SourceBeeCarrier
		&& SourceBeeCarrier->GetBeeCarrierFreeCapacity() > KINDA_SMALL_NUMBER
		&& Context.bHasItemUseAreaHit
		&& Context.ItemUseAreaHitComponent;
}

FItemActionExecutionResult UBeeCarrierUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	FItemActionExecutionResult Result;
	ABeeSwarmClusterActor* ClusterActor = ResolveTargetCluster(Context);
	UItemInstance* SourceBeeCarrier = ResolveSourceBeeCarrier(Context);
	if (!ClusterActor || ClusterActor->IsBeesCaptured() || !SourceBeeCarrier)
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
	float CaptureRate = FMath::Max(0.0f, BaseBeeCapturePerSecond)
		+ BonusSpeed * FMath::Max(0.0f, DragSpeedToBeeCaptureScale);
	CaptureRate = FMath::Clamp(CaptureRate, 0.0f, FMath::Max(0.0f, MaxBeeCapturePerSecond));

	float RequestedBeeAmount = CaptureRate * SafeDeltaTime;
	RequestedBeeAmount = FMath::Min(RequestedBeeAmount, SourceBeeCarrier->GetBeeCarrierFreeCapacity());

	const float ActualCaptured = ClusterActor->CaptureBees(RequestedBeeAmount);
	if (ActualCaptured > KINDA_SMALL_NUMBER)
	{
		SourceBeeCarrier->AddCapturedBees(ActualCaptured);
		Result.bSucceeded = true;
	}
	return Result;
}

ABeeSwarmClusterActor* UBeeCarrierUseAction::ResolveTargetCluster(const FItemActionContext& Context) const
{
	return Cast<ABeeSwarmClusterActor>(Context.ItemUseEffectTargetObject);
}

UItemInstance* UBeeCarrierUseAction::ResolveSourceBeeCarrier(const FItemActionContext& Context) const
{
	UItemInstance* SourceItemInstance = Context.SourceItemInstance;
	if (!SourceItemInstance || !Cast<UBeeCarrierItemDefinition>(SourceItemInstance->GetDefinition()))
	{
		return nullptr;
	}

	return SourceItemInstance;
}

void UBeeCarrierUseAction::ResetDragState()
{
	bHasLastImpactPoint = false;
	LastImpactPoint = FVector::ZeroVector;
}
