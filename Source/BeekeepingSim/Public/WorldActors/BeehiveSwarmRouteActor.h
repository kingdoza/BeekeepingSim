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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route", meta = (ClampMin = "0.0"))
	float ForwardLeadDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route", meta = (ClampMin = "1.0"))
	float AutoMiddlePointSpacing = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route", meta = (ClampMin = "1"))
	int32 MaxAutoMiddlePointCount = 11;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Beehive Swarm Route")
	FVector RouteEndWorldLocation = FVector::ZeroVector;

private:
	int32 CalculateAutoMiddlePointCount(float LeadToEndDistance) const;
	void BuildAutoRouteSplinePoints(const FVector& StartWorldLocation, const FVector& EndWorldLocation, TArray<FVector>& OutWorldPoints) const;
};
