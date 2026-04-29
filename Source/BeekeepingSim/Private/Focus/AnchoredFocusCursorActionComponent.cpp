// Fill out your copyright notice in the Description page of Project Settings.


#include "Focus/AnchoredFocusCursorActionComponent.h"

#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"

bool UAnchoredFocusCursorActionComponent::WantsCrosshairHiddenWhileEngaged() const
{
	return true;
}

bool UAnchoredFocusCursorActionComponent::ShouldRestoreCrosshairOnCancelStart() const
{
	return true;
}

EHotbarPresentationMode UAnchoredFocusCursorActionComponent::GetHotbarPresentationModeWhileEngaged() const
{
	return EHotbarPresentationMode::OnCursor;
}

void UAnchoredFocusCursorActionComponent::OnFocusEngagedStarted(ABeekeeperCharacter* InteractingCharacter)
{
	Super::OnFocusEngagedStarted(InteractingCharacter);

	APlayerController* PlayerController = ResolveLocalPlayerController(InteractingCharacter);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;
	ApplyEngagedInputMode(PlayerController);
}

void UAnchoredFocusCursorActionComponent::OnFocusCancelStarted(ABeekeeperCharacter* InteractingCharacter)
{
	Super::OnFocusCancelStarted(InteractingCharacter);
}

void UAnchoredFocusCursorActionComponent::OnFocusReturnCompleted(ABeekeeperCharacter* InteractingCharacter)
{
	Super::OnFocusReturnCompleted(InteractingCharacter);

	APlayerController* PlayerController = ResolveLocalPlayerController(InteractingCharacter);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = false;
	RestoreDefaultInputMode(PlayerController);
}

void UAnchoredFocusCursorActionComponent::OnFocusActionAborted(ABeekeeperCharacter* InteractingCharacter)
{
	Super::OnFocusActionAborted(InteractingCharacter);

	APlayerController* PlayerController = ResolveLocalPlayerController(InteractingCharacter);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = false;
	RestoreDefaultInputMode(PlayerController);
}

APlayerController* UAnchoredFocusCursorActionComponent::ResolveLocalPlayerController(ABeekeeperCharacter* InteractingCharacter) const
{
	if (!InteractingCharacter)
	{
		return nullptr;
	}

	APlayerController* PlayerController = Cast<APlayerController>(InteractingCharacter->GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return nullptr;
	}

	return PlayerController;
}

void UAnchoredFocusCursorActionComponent::ApplyEngagedInputMode(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}

void UAnchoredFocusCursorActionComponent::RestoreDefaultInputMode(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
}
