#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "QueenCageItemDefinition.generated.h"

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UQueenCageItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UQueenCageItemDefinition()
	{
		MaxStack = 1;
	}
};
