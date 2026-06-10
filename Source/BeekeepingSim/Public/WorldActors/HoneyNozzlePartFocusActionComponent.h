#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "HoneyNozzlePartFocusActionComponent.generated.h"

class AHoneyContainerActor;
class AHoneyContainerSlotActor;
class UHoneyTransferComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UHoneyNozzlePartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	UHoneyNozzlePartFocusActionComponent();

	virtual bool CanBeginPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool BeginPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
	virtual FText ResolvePrimaryPromptActionText() const override;

private:
	bool ResolveTransferContext(AHoneyContainerActor*& OutSourceContainer, AHoneyContainerSlotActor*& OutSourceSlot, UHoneyTransferComponent*& OutTransferComponent) const;
	bool CanToggleTransfer() const;
};
