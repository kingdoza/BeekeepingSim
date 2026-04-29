#include "Interaction/StorageBoxFocusActionComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"
#include "Character/BeekeeperController.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/StorageBoxComponent.h"
#include "UI/StorageBoxWidget.h"

bool UStorageBoxFocusActionComponent::CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	if (!Super::CanBeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	if (!InteractingCharacter || !StorageWidgetClass)
	{
		return false;
	}

	if (!InteractingCharacter->GetBeekeeperHotbar())
	{
		return false;
	}

	UStorageBoxComponent* OwnerStorageComponent = GetOwner() ? GetOwner()->FindComponentByClass<UStorageBoxComponent>() : nullptr;
	if (!OwnerStorageComponent)
	{
		return false;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(InteractingCharacter->GetController());
	return PlayerController && PlayerController->IsLocalController();
}

void UStorageBoxFocusActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupInteractionUI();
	bIsActionEngaged = false;
	Super::EndPlay(EndPlayReason);
}

bool UStorageBoxFocusActionComponent::BeginFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!CanBeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	if (!Super::BeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	ActiveCharacter = InteractingCharacter;
	StorageComponent = GetOwner() ? GetOwner()->FindComponentByClass<UStorageBoxComponent>() : nullptr;
	APlayerController* PlayerController = Cast<APlayerController>(InteractingCharacter->GetController());
	UBeekeeperHotbarComponent* HotbarComponent = InteractingCharacter->GetBeekeeperHotbar();

	if (!StorageComponent || !PlayerController || !HotbarComponent)
	{
		CleanupInteractionUI();
		bIsActionEngaged = false;
		return false;
	}

	InteractingCharacter->SetFocusInteractionInputLocked(true);
	PlayerController->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	PlayerController->SetInputMode(InputMode);
	bAppliedInputMode = true;

	ActiveWidget = CreateWidget<UStorageBoxWidget>(PlayerController, StorageWidgetClass);
	if (!ActiveWidget)
	{
		CleanupInteractionUI();
		bIsActionEngaged = false;
		return false;
	}

	if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(PlayerController))
	{
		BeekeeperController->SetActiveStorageComponent(StorageComponent);
	}

	ActiveWidget->InitializeStorageWidget(StorageComponent, HotbarComponent);
	ActiveWidget->AddToViewport();

	return true;
}

bool UStorageBoxFocusActionComponent::CancelFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!bIsActionEngaged)
	{
		return false;
	}

	CleanupInteractionUI();
	bIsActionEngaged = false;
	return true;
}

void UStorageBoxFocusActionComponent::AbortFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	CleanupInteractionUI();
	bIsActionEngaged = false;
}

bool UStorageBoxFocusActionComponent::WantsCrosshairHiddenWhileEngaged() const
{
	return true;
}

bool UStorageBoxFocusActionComponent::ShouldRestoreCrosshairOnCancelStart() const
{
	return true;
}

EHotbarPresentationMode UStorageBoxFocusActionComponent::GetHotbarPresentationModeWhileEngaged() const
{
	return EHotbarPresentationMode::InHand;
}

bool UStorageBoxFocusActionComponent::ShouldClearHotbarSelectionOnFocusEngaged() const
{
	return true;
}

bool UStorageBoxFocusActionComponent::ShouldBlockHotbarSlotInputWhileEngaged() const
{
	return true;
}

bool UStorageBoxFocusActionComponent::ShouldBlockHotbarWheelInputWhileEngaged() const
{
	return true;
}

void UStorageBoxFocusActionComponent::CleanupInteractionUI()
{
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	if (bAppliedInputMode)
	{
		APlayerController* PlayerController = ActiveCharacter ? Cast<APlayerController>(ActiveCharacter->GetController()) : nullptr;
		if (PlayerController)
		{
			if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(PlayerController))
			{
				BeekeeperController->ClearActiveItemSlotDragOperation();
				if (BeekeeperController->GetActiveStorageComponent() == StorageComponent)
				{
					BeekeeperController->ClearActiveStorageComponent();
				}
			}

			PlayerController->bShowMouseCursor = false;
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
		}

		if (ActiveCharacter)
		{
			ActiveCharacter->SetFocusInteractionInputLocked(false);
		}
	}

	bAppliedInputMode = false;
	StorageComponent = nullptr;
	ActiveCharacter = nullptr;
}
