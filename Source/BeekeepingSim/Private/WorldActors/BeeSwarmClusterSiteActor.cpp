// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldActors/BeeSwarmClusterSiteActor.h"

#include "Components/SceneComponent.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeeSwarmClusterActor.h"

ABeeSwarmClusterSiteActor::ABeeSwarmClusterSiteActor()
{
	AcceptedOccupantClass = ABeeSwarmClusterActor::StaticClass();
}

float ABeeSwarmClusterSiteActor::CalculateSelectionWeightForHive(const ABeehive* Hive) const
{
	if (!IsValid(Hive))
	{
		return 0.0f;
	}

	const USceneComponent* SwarmExitPoint = Hive->GetSwarmExitPointComponent();
	const FVector HiveLocation = SwarmExitPoint ? SwarmExitPoint->GetComponentLocation() : Hive->GetActorLocation();
	const FVector SiteLocation = GetOccupantSpawnTransform().GetLocation();
	const float Distance = bUse2DDistanceForSelection
		? FVector::Dist2D(HiveLocation, SiteLocation)
		: FVector::Dist(HiveLocation, SiteLocation);

	if (MaxSelectionDistanceCm > 0.0f && Distance > MaxSelectionDistanceCm)
	{
		return 0.0f;
	}

	const float SafeMultiplier = FMath::IsFinite(SelectionWeightMultiplier)
		? FMath::Max(0.0f, SelectionWeightMultiplier)
		: 0.0f;
	if (SafeMultiplier <= 0.0f)
	{
		return 0.0f;
	}

	const float SafeDistanceScale = FMath::IsFinite(DistanceWeightScaleCm) && DistanceWeightScaleCm > KINDA_SMALL_NUMBER
		? DistanceWeightScaleCm
		: 1.0f;
	const float SafeExponent = FMath::IsFinite(DistanceWeightExponent) && DistanceWeightExponent > KINDA_SMALL_NUMBER
		? DistanceWeightExponent
		: 1.0f;

	const float Weight = SafeMultiplier / FMath::Pow(1.0f + (Distance / SafeDistanceScale), SafeExponent);
	return FMath::IsFinite(Weight) ? FMath::Max(0.0f, Weight) : 0.0f;
}
