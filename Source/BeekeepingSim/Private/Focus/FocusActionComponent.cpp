// Fill out your copyright notice in the Description page of Project Settings.


#include "Focus/FocusActionComponent.h"

UFocusActionComponent::UFocusActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFocusActionComponent::CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	return InteractingCharacter != nullptr && !bIsActionEngaged;
}

bool UFocusActionComponent::BeginFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!CanBeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	bIsActionEngaged = true;
	return true;
}

bool UFocusActionComponent::CancelFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!InteractingCharacter || !bIsActionEngaged)
	{
		return false;
	}

	bIsActionEngaged = false;
	return true;
}

bool UFocusActionComponent::HandleConfirmInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return false;
}

bool UFocusActionComponent::HandleCancelInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return false;
}

bool UFocusActionComponent::HandlePartFocusClickInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return HandlePartFocusPointerPressedInputWhileEngaged(InteractingCharacter);
}

bool UFocusActionComponent::HandlePartFocusClickReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return HandlePartFocusPointerReleasedInputWhileEngaged(InteractingCharacter);
}

bool UFocusActionComponent::HandlePartFocusPointerPressedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return false;
}

bool UFocusActionComponent::HandlePartFocusPointerReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return false;
}

bool UFocusActionComponent::HandlePartFocusPreviewKeyInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key)
{
	return false;
}

bool UFocusActionComponent::HandleSecondaryInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter)
{
	return false;
}

void UFocusActionComponent::AbortFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	bIsActionEngaged = false;
}

bool UFocusActionComponent::WantsCrosshairHiddenWhileEngaged() const
{
	return false;
}

bool UFocusActionComponent::ShouldRestoreCrosshairOnCancelStart() const
{
	return false;
}

EHotbarPresentationMode UFocusActionComponent::GetHotbarPresentationModeWhileEngaged() const
{
	return EHotbarPresentationMode::InHand;
}

bool UFocusActionComponent::ShouldClearHotbarSelectionOnFocusEngaged() const
{
	return true;
}

bool UFocusActionComponent::ShouldBlockHotbarSlotInputWhileEngaged() const
{
	return false;
}

bool UFocusActionComponent::ShouldBlockHotbarWheelInputWhileEngaged() const
{
	return false;
}

void UFocusActionComponent::SetPromptActionText(const FText& NewText)
{
	PromptActionText = NewText;
}

FText UFocusActionComponent::GetPromptActionText() const
{
	return PromptActionText;
}

void UFocusActionComponent::SetEngagedPromptActionText(const FText& NewText)
{
	EngagedPromptActionText = NewText;
}

FText UFocusActionComponent::GetEngagedPromptActionText() const
{
	return EngagedPromptActionText;
}

FText UFocusActionComponent::ResolveFocusPromptActionText() const
{
	if (IsActionEngaged() && !EngagedPromptActionText.IsEmpty())
	{
		return EngagedPromptActionText;
	}

	return PromptActionText;
}

void UFocusActionComponent::AppendFocusPromptEntries(const FFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const
{
	if (Context.BasePromptData.InteractionKeyText.IsEmpty())
	{
		return;
	}

	FFocusPromptEntry Entry;
	Entry.EntryId = FName(TEXT("Primary"));
	Entry.KeyText = Context.BasePromptData.InteractionKeyText;
	Entry.ActionText = ResolveFocusPromptActionText();
	Entry.bEnabled = CanBeginFocusAction(Context.InteractingCharacter);
	Entry.SortPriority = 0;
	OutEntries.Add(MoveTemp(Entry));
}
