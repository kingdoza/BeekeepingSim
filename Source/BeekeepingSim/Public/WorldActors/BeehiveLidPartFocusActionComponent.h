#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "BeehiveLidPartFocusActionComponent.generated.h"

class ABeehive;
class UCursorPartFocusScopeComponent;
class ABeekeeperCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeehiveLidPartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	UBeehiveLidPartFocusActionComponent();
};
