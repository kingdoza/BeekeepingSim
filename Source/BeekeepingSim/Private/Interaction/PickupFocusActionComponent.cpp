#include "Interaction/PickupFocusActionComponent.h"

#include "Engine/Engine.h"
#include "Character/BeekeeperCharacter.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemDefinition.h"
#include "WorldActors/WorldItemPickup.h"

UPickupFocusActionComponent::UPickupFocusActionComponent()
{
	PromptActionText = FText::FromString(TEXT("획득"));
}

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

void UPickupFocusActionComponent::AppendFocusPromptEntries(const FFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const
{
	FFocusPromptEntry Entry;
	Entry.EntryId = FName(TEXT("Pickup"));
	Entry.KeyText = Context.BasePromptData.InteractionKeyText;
	Entry.ActionText = ResolveFocusPromptActionText();
	Entry.bEnabled = false;
	Entry.SortPriority = 0;

	const AWorldItemPickup* PickupOwner = Cast<AWorldItemPickup>(GetOwner());
	if (!PickupOwner || !PickupOwner->IsPickupValid())
	{
		Entry.DisabledReason = FText::FromString(TEXT("획득 대상을 사용할 수 없습니다."));
		OutEntries.Add(MoveTemp(Entry));
		return;
	}

	UItemDefinition* ItemDefinition = PickupOwner->GetItemDefinition();
	if (!ItemDefinition)
	{
		Entry.DisabledReason = FText::FromString(TEXT("획득할 아이템 정의가 없습니다."));
		OutEntries.Add(MoveTemp(Entry));
		return;
	}

	ABeekeeperCharacter* InteractingCharacter = Context.InteractingCharacter;
	UBeekeeperHotbarComponent* HotbarComponent = InteractingCharacter ? InteractingCharacter->GetBeekeeperHotbar() : nullptr;
	if (!HotbarComponent)
	{
		Entry.DisabledReason = FText::FromString(TEXT("인벤토리를 찾을 수 없습니다."));
		OutEntries.Add(MoveTemp(Entry));
		return;
	}

	FItemAcquireSpec AcquireSpec;
	AcquireSpec.ItemDefinition = ItemDefinition;
	AcquireSpec.Quantity = 1;
	if (ItemDefinition->bUsesDurability)
	{
		AcquireSpec.bOverrideDurability = true;
		AcquireSpec.Durability = FMath::Max(0.0f, ItemDefinition->MaxDurability);
	}

	const FHotbarItemAcquireResult PreviewResult = HotbarComponent->PreviewAcquireItemBySpec(AcquireSpec);
	Entry.bEnabled = PreviewResult.bSuccess && PreviewResult.AddedQuantity == 1 && PreviewResult.RemainingQuantity == 0;
	if (!Entry.bEnabled)
	{
		Entry.DisabledReason = PreviewResult.Message;
	}

	OutEntries.Add(MoveTemp(Entry));
}
