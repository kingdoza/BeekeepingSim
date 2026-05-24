#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "BeehiveCombPartFocusActionComponent.generated.h"

class UCursorPartFocusScopeComponent;
class ABeekeeperCharacter;
class ABeehiveCombActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeehiveCombPartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	UBeehiveCombPartFocusActionComponent();
};
