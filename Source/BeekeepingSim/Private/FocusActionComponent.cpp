// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/FocusActionComponent.h"

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
