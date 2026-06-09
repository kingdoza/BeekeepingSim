#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "CombUncappingPartFocusActionComponent.generated.h"

class AUncappingTableCombSlot;
class ABeehiveCombActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UCombUncappingPartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	UCombUncappingPartFocusActionComponent();

	virtual bool CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void AppendPartFocusPromptEntries(const FPartFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const override;

	virtual bool CanBeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool BeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void UpdatePartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, float DeltaTime) override;
	virtual bool EndPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, bool bCanceled) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping|Drag", meta = (ClampMin = "0.0"))
	float CombFlipDragThresholdPixels = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping|Drag", meta = (ClampMin = "0.0"))
	float HorizontalDominanceRatio = 1.5f;

private:
	void ResetDragState();
	AUncappingTableCombSlot* ResolveOwnerSlot() const;
	ABeehiveCombActor* ResolvePlacedCombActor() const;
	void RebuildOwnerTableDescriptors() const;

	bool bFlipExecutedThisDrag = false;
};
