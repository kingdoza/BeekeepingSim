#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "PollenPattyItemDefinition.generated.h"

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UPollenPattyItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pollen Patty", meta = (ClampMin = "1.0"))
	float EggLayingMultiplier = 1.2f;
};

