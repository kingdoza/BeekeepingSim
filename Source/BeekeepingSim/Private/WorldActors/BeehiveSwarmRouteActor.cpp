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

	const FVector MidWorldLocation = (StartWorldLocation + EndWorldLocation) * 0.5f
		+ FVector(0.0f, 0.0f, RouteMidPointHeightOffset);

	SwarmSpline->ClearSplinePoints(false);
	SwarmSpline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local, false);
	SwarmSpline->AddSplinePoint(GetActorTransform().InverseTransformPosition(MidWorldLocation), ESplineCoordinateSpace::Local, false);
	SwarmSpline->AddSplinePoint(GetActorTransform().InverseTransformPosition(EndWorldLocation), ESplineCoordinateSpace::Local, false);
	SwarmSpline->SetSplinePointType(0, ESplinePointType::Curve, false);
	SwarmSpline->SetSplinePointType(1, ESplinePointType::Curve, false);
	SwarmSpline->SetSplinePointType(2, ESplinePointType::Curve, false);
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
