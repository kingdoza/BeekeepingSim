#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "PlacedItemRetrievePartFocusActionComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacedItemRetrievePartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
};

