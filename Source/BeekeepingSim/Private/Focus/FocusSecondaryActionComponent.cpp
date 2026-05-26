#include "Focus/FocusSecondaryActionComponent.h"

bool UFocusSecondaryActionComponent::CanExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	return InteractingCharacter != nullptr;
}

bool UFocusSecondaryActionComponent::ExecuteSecondaryFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	return CanExecuteSecondaryFocusAction(InteractingCharacter);
}

