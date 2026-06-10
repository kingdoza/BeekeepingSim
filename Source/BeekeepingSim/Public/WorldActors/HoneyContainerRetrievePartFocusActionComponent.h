#pragma once

#include "CoreMinimal.h"
#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"
#include "HoneyContainerRetrievePartFocusActionComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UHoneyContainerRetrievePartFocusActionComponent : public UPlacementSlotRetrievePartFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
};
