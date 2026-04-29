#include "Interaction/PickupFocusActionComponent.h"

#include "Engine/Engine.h"
#include "Character/BeekeeperCharacter.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "WorldActors/WorldItemPickup.h"

bool UPickupFocusActionComponent::CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	if (!Super::CanBeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	const AWorldItemPickup* PickupOwner = Cast<AWorldItemPickup>(GetOwner());
	return PickupOwner && PickupOwner->IsPickupValid() && InteractingCharacter && InteractingCharacter->GetBeekeeperHotbar();
}

bool UPickupFocusActionComponent::BeginFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!Super::BeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	AWorldItemPickup* PickupOwner = Cast<AWorldItemPickup>(GetOwner());
	UBeekeeperHotbarComponent* HotbarComponent = InteractingCharacter ? InteractingCharacter->GetBeekeeperHotbar() : nullptr;
	if (!PickupOwner || !PickupOwner->IsPickupValid() || !HotbarComponent)
	{
		AbortFocusAction(InteractingCharacter);
		return false;
	}

	const FHotbarItemAcquireResult AcquireResult = HotbarComponent->TryAcquireItem(PickupOwner->GetItemDefinition(), 1);
	if (!AcquireResult.bSuccess && !AcquireResult.bPartiallySucceeded)
	{
		const FString DebugMessage = AcquireResult.Message.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Pickup failed: %s"), *DebugMessage);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				INDEX_NONE,
				2.0f,
				FColor::Yellow,
				FString::Printf(TEXT("Pickup failed: %s"), *DebugMessage));
		}

		AbortFocusAction(InteractingCharacter);
		return false;
	}

	if (AcquireResult.bPartiallySucceeded)
	{
		const FString DebugMessage = AcquireResult.Message.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Pickup partially succeeded: %s"), *DebugMessage);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				INDEX_NONE,
				2.0f,
				FColor::Yellow,
				FString::Printf(TEXT("Pickup partial: %s"), *DebugMessage));
		}
	}

	PickupOwner->ConsumePickup();
	AbortFocusAction(InteractingCharacter);
	return true;
}
