#pragma once

#include "CoreMinimal.h"
#include "WorldActors/PlacedItemRemainingVisualComponent.h"
#include "PlacedItemAreaScaleRemainingVisualComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacedItemAreaScaleRemainingVisualComponent : public UPlacedItemRemainingVisualComponent
{
	GENERATED_BODY()

public:
	virtual void ApplyRemainingRatio(float Ratio) override;
};

