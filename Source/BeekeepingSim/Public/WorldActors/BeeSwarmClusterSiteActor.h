// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldActors/WorldOccupancySiteActor.h"
#include "BeeSwarmClusterSiteActor.generated.h"

class ABeehive;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeeSwarmClusterSiteActor : public AWorldOccupancySiteActor
{
	GENERATED_BODY()

public:
	ABeeSwarmClusterSiteActor();

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster Site")
	float CalculateSelectionWeightForHive(const ABeehive* Hive) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster Site|Selection")
	float SelectionWeightMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster Site|Selection", meta = (ClampMin = "1.0"))
	float DistanceWeightScaleCm = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster Site|Selection", meta = (ClampMin = "0.0001"))
	float DistanceWeightExponent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster Site|Selection")
	bool bUse2DDistanceForSelection = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster Site|Selection", meta = (ClampMin = "0.0"))
	float MaxSelectionDistanceCm = 0.0f;
};
