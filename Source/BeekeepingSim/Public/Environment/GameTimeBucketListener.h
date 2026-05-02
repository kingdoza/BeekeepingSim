#pragma once

#include "CoreMinimal.h"
#include "Environment/GameTimeBucketTypes.h"
#include "UObject/Interface.h"
#include "GameTimeBucketListener.generated.h"

UINTERFACE(Blueprintable)
class BEEKEEPINGSIM_API UGameTimeBucketListener : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IGameTimeBucketListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game Time Bucket")
	void GetGameTimeBucketSubscriptions(TArray<FGameTimeBucketSubscription>& OutSubscriptions) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game Time Bucket")
	void OnGameTimeBucketEvent(const FGameTimeBucketEvent& Event);
};

