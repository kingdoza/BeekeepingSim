#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "BeehiveCombPartFocusActionComponent.generated.h"

class UCursorPartFocusScopeComponent;
class ABeekeeperCharacter;
class ABeehiveCombActor;

enum class EBeehiveCombDragMode : uint8
{
	None,
	Flip,
	Shake
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeehiveCombPartFocusActionComponent : public UCursorPartFocusActionComponent
{
	GENERATED_BODY()

public:
	UBeehiveCombPartFocusActionComponent();

	virtual bool CanBeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const override;
	virtual bool BeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) override;
	virtual void UpdatePartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, float DeltaTime) override;
	virtual bool EndPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, bool bCanceled) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
	float CombFlipDragThresholdPixels = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
	float HorizontalDominanceRatio = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
	float CombShakeStrokeThresholdPixels = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "1"))
	int32 RequiredShakeStrokeCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShakeBeeReductionRatio = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
	float VerticalDominanceRatio = 1.25f;

private:
	void ResetDragInterpretationState();
	ABeehiveCombActor* ResolveOwnerCombActor() const;
	void UpdateShakeMode(const FVector2D& DeltaSinceLastUpdate, ABeehiveCombActor* CombActor);

	EBeehiveCombDragMode ActiveDragMode = EBeehiveCombDragMode::None;
	float AccumulatedShakeDistanceInCurrentDirection = 0.0f;
	float LastShakeDirectionSign = 0.0f;
	int32 ShakeStrokeCount = 0;
	bool bFlipExecutedThisDrag = false;
	bool bShakeExecutedThisDrag = false;
};
