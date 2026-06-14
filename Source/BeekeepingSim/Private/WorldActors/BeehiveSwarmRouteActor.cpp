#include "WorldActors/BeehiveSwarmRouteActor.h"

#include "Components/SplineComponent.h"
#include "WorldActors/BeeSwarmClusterActor.h"

ABeehiveSwarmRouteActor::ABeehiveSwarmRouteActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABeehiveSwarmRouteActor::ConfigureRoute(FVector StartWorldLocation, FVector EndWorldLocation)
{
	RouteEndWorldLocation = EndWorldLocation;
	SetActorLocation(StartWorldLocation);

	if (!SwarmSpline)
	{
		return;
	}

	TArray<FVector> RouteWorldPoints;
	BuildAutoRouteSplinePoints(StartWorldLocation, EndWorldLocation, RouteWorldPoints);

	SwarmSpline->ClearSplinePoints(false);
	for (int32 Index = 0; Index < RouteWorldPoints.Num(); ++Index)
	{
		const FVector LocalPoint = (Index == 0)
			? FVector::ZeroVector
			: GetActorTransform().InverseTransformPosition(RouteWorldPoints[Index]);
		SwarmSpline->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local, false);
		SwarmSpline->SetSplinePointType(Index, ESplinePointType::Curve, false);
	}
	SwarmSpline->UpdateSpline();

	ApplySplineLengthParameter();
}

void ABeehiveSwarmRouteActor::ConfigureRouteToCluster(FVector StartWorldLocation, ABeeSwarmClusterActor* ClusterActor)
{
	const USceneComponent* ClusterCenter = ClusterActor ? ClusterActor->GetClusterCenterComponent() : nullptr;
	const FVector EndWorldLocation = ClusterCenter
		? ClusterCenter->GetComponentLocation()
		: (ClusterActor ? ClusterActor->GetActorLocation() : StartWorldLocation);
	ConfigureRoute(StartWorldLocation, EndWorldLocation);
}

int32 ABeehiveSwarmRouteActor::CalculateAutoMiddlePointCount(float LeadToEndDistance) const
{
	const float Spacing = FMath::Max(1.0f, AutoMiddlePointSpacing);

	int32 SegmentCount = FMath::Max(2, FMath::CeilToInt(FMath::Max(0.0f, LeadToEndDistance) / Spacing));
	if (SegmentCount % 2 != 0)
	{
		++SegmentCount;
	}

	int32 AutoMiddlePointCount = SegmentCount - 1;
	AutoMiddlePointCount = FMath::Clamp(AutoMiddlePointCount, 1, FMath::Max(1, MaxAutoMiddlePointCount));
	if (AutoMiddlePointCount % 2 == 0)
	{
		--AutoMiddlePointCount;
	}

	return FMath::Max(1, AutoMiddlePointCount);
}

void ABeehiveSwarmRouteActor::BuildAutoRouteSplinePoints(
	const FVector& StartWorldLocation,
	const FVector& EndWorldLocation,
	TArray<FVector>& OutWorldPoints) const
{
	OutWorldPoints.Reset();

	const FVector LeadPoint = StartWorldLocation + GetActorForwardVector() * FMath::Max(0.0f, ForwardLeadDistance);
	const int32 AutoMiddlePointCount = CalculateAutoMiddlePointCount(FVector::Distance(LeadPoint, EndWorldLocation));

	OutWorldPoints.Reserve(AutoMiddlePointCount + 3);
	OutWorldPoints.Add(StartWorldLocation);
	OutWorldPoints.Add(LeadPoint);

	for (int32 Index = 0; Index < AutoMiddlePointCount; ++Index)
	{
		const float Alpha = static_cast<float>(Index + 1) / static_cast<float>(AutoMiddlePointCount + 1);
		const FVector BasePoint = FMath::Lerp(LeadPoint, EndWorldLocation, Alpha);
		const float HeightRatio = FMath::Sin(Alpha * UE_PI);
		OutWorldPoints.Add(BasePoint + FVector::UpVector * RouteMidPointHeightOffset * HeightRatio);
	}

	OutWorldPoints.Add(EndWorldLocation);
}
