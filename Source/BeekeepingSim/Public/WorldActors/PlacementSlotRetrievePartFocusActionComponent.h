#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "PlacementSlotRetrievePartFocusActionComponent.generated.h"

class AActor;
class ABeekeeperCharacter;
class UItemInstance;
class UPlacementOccupantComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacementSlotRetrievePartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;

	bool CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const;
	bool TryRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter, UItemInstance*& OutAcquiredItemInstance, AActor*& OutSlotActor);

protected:
	UPlacementOccupantComponent* ResolvePlacementOccupant();
	const UPlacementOccupantComponent* ResolvePlacementOccupant() const;
};
