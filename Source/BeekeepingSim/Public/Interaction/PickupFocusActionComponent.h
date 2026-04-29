#pragma once

#include "CoreMinimal.h"
#include "Focus/FocusActionComponent.h"
#include "PickupFocusActionComponent.generated.h"

class ABeekeeperCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPickupFocusActionComponent : public UFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const override;

	virtual bool BeginFocusAction(ABeekeeperCharacter* InteractingCharacter) override;
};
