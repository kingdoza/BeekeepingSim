// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Focus/FocusActionComponent.h"
#include "AnchoredFocusActionComponent.generated.h"

class ABeekeeperCharacter;
class USceneComponent;

UENUM()
enum class EAnchoredFocusActionState : uint8
{
	Idle,
	BlendingToFocus,
	Focused,
	BlendingBack
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UAnchoredFocusActionComponent : public UFocusActionComponent
{
	GENERATED_BODY()

public:
	UAnchoredFocusActionComponent();

	virtual bool CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const override;

	virtual bool BeginFocusAction(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool CancelFocusAction(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void AbortFocusAction(ABeekeeperCharacter* InteractingCharacter) override;

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	USceneComponent* ResolveAnchorByTag(const FName& AnchorTag) const;

	bool UpdateCameraBlend(float DeltaTime, const FTransform& TargetTransform, float InterpSpeed);

	virtual void OnFocusEngagedStarted(ABeekeeperCharacter* InteractingCharacter);

	virtual void OnFocusCancelStarted(ABeekeeperCharacter* InteractingCharacter);

	virtual void OnFocusReturnCompleted(ABeekeeperCharacter* InteractingCharacter);

	virtual void OnFocusActionAborted(ABeekeeperCharacter* InteractingCharacter);

	ABeekeeperCharacter* GetCurrentInteractingCharacter() const { return CurrentInteractingCharacter; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Action")
	FName FocusAnchorTag = TEXT("FocusAnchor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Action")
	FName CharacterAnchorTag = TEXT("CharacterAnchor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Action", meta = (ClampMin = "0.1"))
	float FocusBlendInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Action", meta = (ClampMin = "0.1"))
	float ReturnBlendInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Action", meta = (ClampMin = "0.0"))
	float BlendCompletionDistance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Action", meta = (ClampMin = "0.0"))
	float BlendCompletionAngle = 1.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> CurrentInteractingCharacter;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> FocusAnchorComponent;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> CharacterAnchorComponent;

	EAnchoredFocusActionState ActionState = EAnchoredFocusActionState::Idle;
};
