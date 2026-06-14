#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemInstance.h"
#include "UObject/Interface.h"
#include "QueenBeeCaptureSource.generated.h"

class AQueenBeeActor;

UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UQueenBeeCaptureSource : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IQueenBeeCaptureSource
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Queen Bee|Capture")
	bool CanCaptureQueenBee(AQueenBeeActor* QueenBee) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Queen Bee|Capture")
	bool CaptureQueenBee(AQueenBeeActor* QueenBee, FQueenCageItemState& OutCapturedState);
};
