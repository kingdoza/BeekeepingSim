#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "BeeCarrierItemDefinition.generated.h"

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UBeeCarrierItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UBeeCarrierItemDefinition()
	{
		MaxStack = 1;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Bee Carrier", meta = (ClampMin = "0.0"))
	float MaxCapturedBeeAmount = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Bee Carrier", meta = (ClampMin = "0.0"))
	float DefaultCapturedBeeAmount = 0.0f;
};
