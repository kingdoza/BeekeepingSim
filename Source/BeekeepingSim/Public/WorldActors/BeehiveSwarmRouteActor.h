#pragma once

#include "CoreMinimal.h"
#include "WorldActors/BeeSplineSwarmActor.h"
#include "BeehiveSwarmRouteActor.generated.h"

class ABeeSwarmClusterActor;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeehiveSwarmRouteActor : public ABeeSplineSwarmActor
{
	GENERATED_BODY()

public:
	ABeehiveSwarmRouteActor();

	UFUNCTION(BlueprintCallable, Category = "Beehive Swarm Route")
	void ConfigureRoute(FVector StartWorldLocation, FVector EndWorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Beehive Swarm Route")
	void ConfigureRouteToCluster(FVector StartWorldLocation, ABeeSwarmClusterActor* ClusterActor);

	UFUNCTION(BlueprintPure, Category = "Beehive Swarm Route")
	FVector GetRouteEndWorldLocation() const { return RouteEndWorldLocation; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route")
	float RouteMidPointHeightOffset = 120.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Beehive Swarm Route")
	FVector RouteEndWorldLocation = FVector::ZeroVector;
};
