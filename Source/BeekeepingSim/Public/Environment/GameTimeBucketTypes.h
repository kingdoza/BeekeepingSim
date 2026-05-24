#pragma once

#include "CoreMinimal.h"
#include "GameTimeBucketTypes.generated.h"

UENUM(BlueprintType)
enum class EGameTimeBucketCatchUpPolicy : uint8
{
	LatestOnly,
	CatchUp
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FGameTimeBucketSubscription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket", meta = (ClampMin = "1", ClampMax = "1440"))
	int32 BucketMinutes = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket")
	bool bApplyImmediatelyOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket")
	EGameTimeBucketCatchUpPolicy CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Time Bucket")
	FName SubscriptionTag = NAME_None;
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FGameTimeBucketEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	float Hour24 = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	int32 BucketMinutes = 10;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	int32 BucketIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	int32 BucketStartMinute = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	int32 BucketEndMinute = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	bool bWrappedDay = false;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	bool bInitialApply = false;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	bool bCatchUp = false;

	UPROPERTY(BlueprintReadOnly, Category = "Game Time Bucket")
	FName SubscriptionTag = NAME_None;
};

