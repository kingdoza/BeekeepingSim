#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FocusSecondaryActionComponent.generated.h"

class ABeekeeperCharacter;

UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UFocusSecondaryActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Focus Secondary Action")
	virtual bool CanExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter) const;

	UFUNCTION(BlueprintCallable, Category = "Focus Secondary Action")
	virtual bool ExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter);
};

