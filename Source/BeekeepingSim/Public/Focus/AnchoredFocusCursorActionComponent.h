// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Focus/AnchoredFocusActionComponent.h"
#include "AnchoredFocusCursorActionComponent.generated.h"

class APlayerController;
class ABeekeeperCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UAnchoredFocusCursorActionComponent : public UAnchoredFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool WantsCrosshairHiddenWhileEngaged() const override;

	virtual bool ShouldRestoreCrosshairOnCancelStart() const override;

	virtual EHotbarPresentationMode GetHotbarPresentationModeWhileEngaged() const override;

	virtual bool ShouldClearHotbarSelectionOnFocusEngaged() const override;

	virtual bool HandleConfirmInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool HandleCancelInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool HandlePartFocusClickInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool HandlePartFocusClickReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool HandlePartFocusPointerPressedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool HandlePartFocusPointerReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool HandlePartFocusPreviewKeyInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key) override;

protected:
	virtual void OnFocusEngagedStarted(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusCancelStarted(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusReturnCompleted(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusActionAborted(ABeekeeperCharacter* InteractingCharacter) override;

private:
	APlayerController* ResolveLocalPlayerController(ABeekeeperCharacter* InteractingCharacter) const;

	void ApplyEngagedInputMode(APlayerController* PlayerController) const;

	void RestoreDefaultInputMode(APlayerController* PlayerController) const;
};
