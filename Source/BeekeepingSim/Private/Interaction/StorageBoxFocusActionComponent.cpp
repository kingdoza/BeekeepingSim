#include "Interaction/StorageBoxFocusActionComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"
#include "Character/BeekeeperController.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/StorageBoxComponent.h"
#include "UI/StorageBoxWidget.h"

namespace
{
void CenterMouseCursorInViewportForStorageBox(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return;
	}

	PlayerController->SetMouseLocation(ViewportSizeX / 2, ViewportSizeY / 2);
}
}

UStorageBoxFocusActionComponent::UStorageBoxFocusActionComponent()
{
	PromptActionText = FText::FromString(TEXT("열기"));
}

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
	CenterMouseCursorInViewportForStorageBox(PlayerController);
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

void UStorageBoxFocusActionComponent::AppendFocusPromptEntries(const FFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const
{
	FFocusPromptEntry Entry;
	Entry.EntryId = FName(TEXT("StorageOpen"));
	Entry.KeyText = Context.BasePromptData.InteractionKeyText;
	Entry.ActionText = ResolveFocusPromptActionText();
	Entry.bEnabled = CanBeginFocusAction(Context.InteractingCharacter);
	Entry.SortPriority = 0;

	if (!Entry.bEnabled)
	{
		if (!StorageWidgetClass)
		{
			Entry.DisabledReason = FText::FromString(TEXT("Storage 위젯 클래스가 설정되지 않았습니다."));
		}
		else if (!Context.InteractingCharacter)
		{
			Entry.DisabledReason = FText::FromString(TEXT("상호작용 캐릭터를 찾을 수 없습니다."));
		}
		else if (!Context.InteractingCharacter->GetBeekeeperHotbar())
		{
			Entry.DisabledReason = FText::FromString(TEXT("인벤토리를 찾을 수 없습니다."));
		}
		else if (!(GetOwner() && GetOwner()->FindComponentByClass<UStorageBoxComponent>()))
		{
			Entry.DisabledReason = FText::FromString(TEXT("Storage 컴포넌트를 찾을 수 없습니다."));
		}
		else
		{
			const APlayerController* PlayerController = Context.InteractingCharacter ? Cast<APlayerController>(Context.InteractingCharacter->GetController()) : nullptr;
			if (!PlayerController || !PlayerController->IsLocalController())
			{
				Entry.DisabledReason = FText::FromString(TEXT("로컬 플레이어 컨트롤러가 필요합니다."));
			}
		}
	}

	OutEntries.Add(MoveTemp(Entry));
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
