#include "Focus/CursorPartFocusActionComponent.h"

UCursorPartFocusActionComponent::UCursorPartFocusActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCursorPartFocusActionComponent::CanBeginPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const
{
	return ScopeComponent != nullptr && InteractingCharacter != nullptr && !bIsPartActionEngaged;
}

bool UCursorPartFocusActionComponent::BeginPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	if (!CanBeginPartFocusAction(ScopeComponent, InteractingCharacter))
	{
		return false;
	}

	bIsPartActionEngaged = true;
	ReceivePartFocusBegin(ScopeComponent, InteractingCharacter);
	OnPartFocusBegin.Broadcast(this, ScopeComponent, InteractingCharacter);
	return true;
}

bool UCursorPartFocusActionComponent::CancelPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	if (!ScopeComponent || !InteractingCharacter || !bIsPartActionEngaged)
	{
		return false;
	}

	ReceivePartFocusCancel(ScopeComponent, InteractingCharacter);
	OnPartFocusCancel.Broadcast(this, ScopeComponent, InteractingCharacter);
	bIsPartActionEngaged = false;
	return true;
}

void UCursorPartFocusActionComponent::AbortPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	ReceivePartFocusAbort(ScopeComponent, InteractingCharacter);
	OnPartFocusAbort.Broadcast(this, ScopeComponent, InteractingCharacter);
	bIsPartActionEngaged = false;
}

bool UCursorPartFocusActionComponent::CanHandlePreviewKeyAction(ECursorPartFocusPreviewInputKey Key) const
{
	switch (Key)
	{
	case ECursorPartFocusPreviewInputKey::R:
		return bEnableRPreviewAction;
	case ECursorPartFocusPreviewInputKey::F:
		return bEnableFPreviewAction;
	case ECursorPartFocusPreviewInputKey::C:
		return bEnableCPreviewAction;
	default:
		return false;
	}
}

bool UCursorPartFocusActionComponent::HandlePreviewKeyAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key)
{
	if (!ScopeComponent || !InteractingCharacter || !CanHandlePreviewKeyAction(Key))
	{
		return false;
	}

	ReceivePartFocusPreviewKeyAction(ScopeComponent, InteractingCharacter, Key);
	OnPartFocusPreviewKeyAction.Broadcast(this, ScopeComponent, InteractingCharacter, Key);
	switch (Key)
	{
	case ECursorPartFocusPreviewInputKey::R:
		ReceivePartFocusPreviewR(ScopeComponent, InteractingCharacter);
		OnPartFocusPreviewR.Broadcast(this, ScopeComponent, InteractingCharacter);
		break;
	case ECursorPartFocusPreviewInputKey::F:
		ReceivePartFocusPreviewF(ScopeComponent, InteractingCharacter);
		OnPartFocusPreviewF.Broadcast(this, ScopeComponent, InteractingCharacter);
		break;
	case ECursorPartFocusPreviewInputKey::C:
		ReceivePartFocusPreviewC(ScopeComponent, InteractingCharacter);
		OnPartFocusPreviewC.Broadcast(this, ScopeComponent, InteractingCharacter);
		break;
	default:
		break;
	}

	return true;
}

bool UCursorPartFocusActionComponent::CanBeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const
{
	return false;
}

bool UCursorPartFocusActionComponent::BeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	bIsPartFocusDragInProgress = false;
	return false;
}

void UCursorPartFocusActionComponent::UpdatePartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, float DeltaTime)
{
}

bool UCursorPartFocusActionComponent::EndPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, bool bCanceled)
{
	bIsPartFocusDragInProgress = false;
	return false;
}
