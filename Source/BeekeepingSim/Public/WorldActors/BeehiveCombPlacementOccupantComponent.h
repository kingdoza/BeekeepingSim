#pragma once

#include "CoreMinimal.h"
#include "WorldActors/PlacementOccupantComponent.h"
#include "BeehiveCombPlacementOccupantComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeehiveCombPlacementOccupantComponent : public UPlacementOccupantComponent
{
	GENERATED_BODY()

public:
	virtual bool ReceiveCanRetrievePlacementOccupant_Implementation(ABeekeeperCharacter* InteractingCharacter) const override;
};

