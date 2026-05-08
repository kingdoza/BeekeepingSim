// Fill out your copyright notice in the Description page of Project Settings.


#include "Focus/AnchoredFocusCursorActionComponent.h"

#include "Focus/CursorItemUseAreaScopeComponent.h"
#include "Focus/CursorPartFocusScopeComponent.h"
#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"
#include "Inventory/BeekeeperHotbarComponent.h"

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

bool UAnchoredFocusCursorActionComponent::ShouldClearHotbarSelectionOnFocusEngaged() const
{
	return true;
}

bool UAnchoredFocusCursorActionComponent::HandleConfirmInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	// Part-focus click is handled by dedicated LMB input path.
	// Consume engaged confirm to avoid accidental host-cancel fallback.
	return true;
}

bool UAnchoredFocusCursorActionComponent::HandleCancelInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	if (!GetOwner())
	{
		return false;
	}

	if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = GetOwner()->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
	{
		ItemUseAreaScope->HandleItemUseCanceled();
	}

	if (UCursorPartFocusScopeComponent* ScopeComponent = GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>())
	{
		return ScopeComponent->HandleCancelInput();
	}

	return false;
}

bool UAnchoredFocusCursorActionComponent::HandlePartFocusClickInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	if (!GetOwner())
	{
		return false;
	}

	if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = GetOwner()->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
	{
		if (ItemUseAreaScope->IsItemUseAreaScopeActive())
		{
			const UBeekeeperHotbarComponent* HotbarComponent = InteractingCharacter ? InteractingCharacter->GetBeekeeperHotbar() : nullptr;
			if (HotbarComponent && HotbarComponent->GetSelectedItemInstance())
			{
				if (ItemUseAreaScope->HandleItemUsePressed())
				{
					return true;
				}
			}
		}
	}

	if (UCursorPartFocusScopeComponent* ScopeComponent = GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>())
	{
		return ScopeComponent->HandlePartFocusClickInput();
	}

	return false;
}

bool UAnchoredFocusCursorActionComponent::HandlePartFocusClickReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	if (!GetOwner())
	{
		return false;
	}

	if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = GetOwner()->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
	{
		if (ItemUseAreaScope->IsItemUseAreaScopeActive() && ItemUseAreaScope->HandleItemUseReleased())
		{
			return true;
		}
	}

	return false;
}

bool UAnchoredFocusCursorActionComponent::HandlePartFocusPreviewKeyInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key)
{
	if (!GetOwner())
	{
		return false;
	}

	if (UCursorPartFocusScopeComponent* ScopeComponent = GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>())
	{
		return ScopeComponent->HandlePreviewKeyInput(Key);
	}

	return false;
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

	if (GetOwner())
	{
		if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = GetOwner()->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
		{
			ItemUseAreaScope->ActivateItemUseAreaScope(InteractingCharacter);
		}

		if (UCursorPartFocusScopeComponent* ScopeComponent = GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>())
		{
			ScopeComponent->ActivatePartFocusScope(InteractingCharacter);
		}
	}
}

void UAnchoredFocusCursorActionComponent::OnFocusCancelStarted(ABeekeeperCharacter* InteractingCharacter)
{
	Super::OnFocusCancelStarted(InteractingCharacter);
}

void UAnchoredFocusCursorActionComponent::OnFocusReturnCompleted(ABeekeeperCharacter* InteractingCharacter)
{
	Super::OnFocusReturnCompleted(InteractingCharacter);

	if (GetOwner())
	{
		if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = GetOwner()->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
		{
			ItemUseAreaScope->DeactivateItemUseAreaScope(true);
		}

		if (UCursorPartFocusScopeComponent* ScopeComponent = GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>())
		{
			ScopeComponent->DeactivatePartFocusScope(true);
		}
	}

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

	if (GetOwner())
	{
		if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = GetOwner()->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
		{
			ItemUseAreaScope->DeactivateItemUseAreaScope(true);
		}

		if (UCursorPartFocusScopeComponent* ScopeComponent = GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>())
		{
			ScopeComponent->DeactivatePartFocusScope(true);
		}
	}

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
