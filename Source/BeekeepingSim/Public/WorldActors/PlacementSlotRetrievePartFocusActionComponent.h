#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "PlacementSlotRetrievePartFocusActionComponent.generated.h"

class AActor;
class ABeekeeperCharacter;
struct FItemAcquireSpec;
class UItemInstance;
class UPlacementOccupantComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacementSlotRetrievePartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void AppendPartFocusPromptEntries(const FPartFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const override;

	bool CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const;
	bool BuildRetrieveAcquireSpec(ABeekeeperCharacter* InteractingCharacter, FItemAcquireSpec& OutAcquireSpec, FText* OutFailureReason = nullptr) const;
	bool CanRetrievePlacementOccupantWithInventory(ABeekeeperCharacter* InteractingCharacter, FText* OutFailureReason = nullptr) const;
	bool TryRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter, UItemInstance*& OutAcquiredItemInstance, AActor*& OutSlotActor);

protected:
	UPlacementOccupantComponent* ResolvePlacementOccupant();
	const UPlacementOccupantComponent* ResolvePlacementOccupant() const;
};
