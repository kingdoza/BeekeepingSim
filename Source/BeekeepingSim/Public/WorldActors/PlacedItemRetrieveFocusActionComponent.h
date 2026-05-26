#pragma once

#include "CoreMinimal.h"
#include "Focus/FocusSecondaryActionComponent.h"
#include "PlacedItemRetrieveFocusActionComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacedItemRetrieveFocusActionComponent : public UFocusSecondaryActionComponent
{
	GENERATED_BODY()

public:
	virtual bool CanExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool ExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter) override;
};

