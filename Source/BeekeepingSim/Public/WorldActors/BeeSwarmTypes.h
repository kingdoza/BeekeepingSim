#pragma once

#include "CoreMinimal.h"
#include "BeeSwarmTypes.generated.h"

class UCurveFloat;

DECLARE_LOG_CATEGORY_EXTERN(LogBeekeepingBeeSwarm, Log, All);

UENUM(BlueprintType)
enum class EBeehiveSwarmState : uint8
{
	Normal,
	Swarming
};

UENUM(BlueprintType)
enum class EBeeSplineSwarmControlMode : uint8
{
	Standalone,
	ExternalControlled
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeeSwarmSpeedRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpeedMin = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpeedMax = 160.0f;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeeSwarmStateSpeedSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FBeeSwarmSpeedRange Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FBeeSwarmSpeedRange Swarming;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeeSplineSwarmSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector StartShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector EndShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	TObjectPtr<UCurveFloat> SpawnAmountByHour = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	float FallbackSpawnAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FBeeSwarmStateSpeedSettings SpeedRanges;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeeSplineSwarmAppliedParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector StartShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector EndShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpawnAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpeedMin = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpeedMax = 160.0f;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeehiveSwarmSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector StartShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector EndShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	TObjectPtr<UCurveFloat> ActivityByHour = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float FallbackActivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpawnAmountScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float MaxSpawnAmount = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FBeeSwarmSpeedRange SpeedRange;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeehiveDualSwarmCommonSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector StartShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector EndShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float SpawnAmountScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float MaxSpawnAmount = 1000.0f;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeehiveDirectionalSwarmSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	TObjectPtr<UCurveFloat> SpawnAmountByHour = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float FallbackActivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FBeeSwarmSpeedRange SpeedRange;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeehiveAttractionSwarmSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
	float AttractionPower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
	float NoisePower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
	float SpawnSphereRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
	float SpawnAmountScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0"))
	int32 MaxSpawnAmount = 1000;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeehiveDualSwarmNiagaraParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector StartShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector EndShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float OutgoingSpawnAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float OutgoingSpeedMin = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float OutgoingSpeedMax = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float IngoingSpawnAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float IngoingSpeedMin = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0"))
	float IngoingSpeedMax = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Disease = 0.0f;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeeSplineSwarmOverrideSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	bool bOverrideStartShapeExtent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector StartShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	bool bOverrideEndShapeExtent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FVector EndShapeExtent = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	bool bOverrideSpawnAmountByHour = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	TObjectPtr<UCurveFloat> SpawnAmountByHour = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	bool bOverrideFallbackSpawnAmount = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	float FallbackSpawnAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	bool bOverrideSpeedRanges = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm")
	FBeeSwarmStateSpeedSettings SpeedRanges;
};
