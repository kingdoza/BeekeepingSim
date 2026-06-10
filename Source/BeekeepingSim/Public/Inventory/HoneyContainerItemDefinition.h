#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "HoneyContainerItemDefinition.generated.h"

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UHoneyContainerItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UHoneyContainerItemDefinition()
	{
		MaxStack = 1;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Honey Container", meta = (ClampMin = "0.0"))
	float MaxVolumeMl = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Honey Container", meta = (ClampMin = "0.0"))
	float DefaultCurrentVolumeMl = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultHoneyDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultHoneyRipeness = 0.0f;
};
