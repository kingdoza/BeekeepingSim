#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TimeOfDayProvider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameTimeOfDayChangedSignature, float, Hour24);

UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UTimeOfDayProvider : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API ITimeOfDayProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Time Of Day")
	float GetCurrentHour24() const;
};

