#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Focus/FocusTargetComponent.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/PlacementOccupantComponent.h"
#include "WorldActors/PlacedItemRemainingComponent.h"

bool UPlacementSlotRetrievePartFocusActionComponent::CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const
{
	return CanRetrievePlacementOccupantWithInventory(InteractingCharacter);
}

bool UPlacementSlotRetrievePartFocusActionComponent::HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	UItemInstance* AcquiredItemInstance = nullptr;
	AActor* SlotActor = nullptr;
	if (!TryRetrievePlacementOccupant(InteractingCharacter, AcquiredItemInstance, SlotActor))
	{
		return false;
	}

	IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
	return true;
}

bool UPlacementSlotRetrievePartFocusActionComponent::CanRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter) const
{
	return CanRetrievePlacementOccupantWithInventory(InteractingCharacter);
}

bool UPlacementSlotRetrievePartFocusActionComponent::BuildRetrieveAcquireSpec(ABeekeeperCharacter* InteractingCharacter, FItemAcquireSpec& OutAcquireSpec, FText* OutFailureReason) const
{
	OutAcquireSpec = FItemAcquireSpec();
	if (OutFailureReason)
	{
		*OutFailureReason = FText::GetEmpty();
	}

	const UPlacementOccupantComponent* Occupant = ResolvePlacementOccupant();
	if (!Occupant)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::FromString(TEXT("획득 대상이 없습니다."));
		}
		return false;
	}

	AActor* SlotActor = Occupant->GetOwningPlacementSlotActor();
	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::FromString(TEXT("배치 슬롯 정보를 찾을 수 없습니다."));
		}
		return false;
	}

	if (!InteractingCharacter)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::FromString(TEXT("상호작용 캐릭터를 찾을 수 없습니다."));
		}
		return false;
	}

	if (!Occupant->CanRetrievePlacementOccupant(InteractingCharacter))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::FromString(TEXT("현재 상태에서는 획득할 수 없습니다."));
		}
		return false;
	}

	UItemDefinition* ItemDefinition = Occupant->GetReturnItemDefinition();
	if (!ItemDefinition)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::FromString(TEXT("획득할 아이템 정의가 없습니다."));
		}
		return false;
	}

	OutAcquireSpec.ItemDefinition = ItemDefinition;
	OutAcquireSpec.Quantity = 1;

	UPlacedItemRemainingComponent* RemainingComponent = GetOwner() ? GetOwner()->FindComponentByClass<UPlacedItemRemainingComponent>() : nullptr;
	if (RemainingComponent && RemainingComponent->HasRemaining())
	{
		OutAcquireSpec.bOverrideDurability = true;
		OutAcquireSpec.Durability = RemainingComponent->GetCurrentAmount();
	}

	return true;
}

bool UPlacementSlotRetrievePartFocusActionComponent::CanRetrievePlacementOccupantWithInventory(ABeekeeperCharacter* InteractingCharacter, FText* OutFailureReason) const
{
	FItemAcquireSpec AcquireSpec;
	if (!BuildRetrieveAcquireSpec(InteractingCharacter, AcquireSpec, OutFailureReason))
	{
		return false;
	}

	UBeekeeperHotbarComponent* Hotbar = InteractingCharacter ? InteractingCharacter->GetBeekeeperHotbar() : nullptr;
	if (!Hotbar)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::FromString(TEXT("인벤토리를 찾을 수 없습니다."));
		}
		return false;
	}

	const FHotbarItemAcquireResult PreviewResult = Hotbar->PreviewAcquireItemBySpec(AcquireSpec);
	const bool bCanAcquire = PreviewResult.bSuccess && PreviewResult.AddedQuantity == 1 && PreviewResult.RemainingQuantity == 0;
	if (!bCanAcquire && OutFailureReason)
	{
		*OutFailureReason = PreviewResult.Message;
	}

	return bCanAcquire;
}

void UPlacementSlotRetrievePartFocusActionComponent::AppendPartFocusPromptEntries(const FPartFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const
{
	FFocusPromptEntry Entry;
	Entry.EntryId = FName(TEXT("Retrieve"));
	Entry.KeyText = FText::FromString(TEXT("RMB"));
	Entry.ActionText = FText::FromString(TEXT("획득"));
	Entry.SortPriority = 50;

	FText FailureReason;
	Entry.bEnabled = CanRetrievePlacementOccupantWithInventory(Context.InteractingCharacter, &FailureReason);
	if (!Entry.bEnabled)
	{
		Entry.DisabledReason = FailureReason;
	}

	OutEntries.Add(MoveTemp(Entry));
}

bool UPlacementSlotRetrievePartFocusActionComponent::TryRetrievePlacementOccupant(ABeekeeperCharacter* InteractingCharacter, UItemInstance*& OutAcquiredItemInstance, AActor*& OutSlotActor)
{
	OutAcquiredItemInstance = nullptr;
	OutSlotActor = nullptr;

	if (!InteractingCharacter)
	{
		return false;
	}

	UPlacementOccupantComponent* Occupant = ResolvePlacementOccupant();
	if (!Occupant)
	{
		return false;
	}

	UBeekeeperHotbarComponent* Hotbar = InteractingCharacter->GetBeekeeperHotbar();
	if (!Hotbar)
	{
		return false;
	}

	FItemAcquireSpec AcquireSpec;
	if (!BuildRetrieveAcquireSpec(InteractingCharacter, AcquireSpec, nullptr))
	{
		return false;
	}

	const FHotbarItemAcquireResult AcquireResult = Hotbar->TryAcquireItemBySpec(AcquireSpec);
	if (!AcquireResult.bSuccess || AcquireResult.AddedQuantity != 1)
	{
		return false;
	}

	AActor* SlotActor = Occupant->GetOwningPlacementSlotActor();
	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return false;
	}

	OutAcquiredItemInstance = AcquireResult.LastModifiedItemInstance.Get();
	UPlacedItemRemainingComponent* RemainingComponent = GetOwner() ? GetOwner()->FindComponentByClass<UPlacedItemRemainingComponent>() : nullptr;
	if (RemainingComponent && OutAcquiredItemInstance)
	{
		RemainingComponent->WriteBackToItemInstance(OutAcquiredItemInstance);
	}

	OutSlotActor = SlotActor;
	return true;
}

UPlacementOccupantComponent* UPlacementSlotRetrievePartFocusActionComponent::ResolvePlacementOccupant()
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UPlacementOccupantComponent>() : nullptr;
}

const UPlacementOccupantComponent* UPlacementSlotRetrievePartFocusActionComponent::ResolvePlacementOccupant() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UPlacementOccupantComponent>() : nullptr;
}
