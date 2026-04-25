// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BeekeeperHotbarComponent.h"

#include "Public/BeekeeperCharacter.h"
#include "Public/BeekeeperFocusComponent.h"
#include "Public/HotbarItemInterface.h"
#include "Public/ItemDefinition.h"
#include "Public/ItemInstance.h"

UBeekeeperHotbarComponent::UBeekeeperHotbarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.SetNum(SlotCount);
}

void UBeekeeperHotbarComponent::InitializeSlots()
{
	if (Slots.Num() != SlotCount)
	{
		Slots.SetNum(SlotCount);
	}

	for (FHotbarSlotData& Slot : Slots)
	{
		Slot.bIsEnabled = true;
	}

	if (!IsIndexValid(SelectedIndex))
	{
		SelectedIndex = INDEX_NONE;
	}

	if (ReevaluateSlotsInternal())
	{
		BroadcastHotbarChanged();
	}
}

void UBeekeeperHotbarComponent::HandleSlotInput(int32 Index)
{
	if (!IsIndexValid(Index))
	{
		return;
	}

	if (SelectedIndex == Index)
	{
		ClearSelection();
		return;
	}

	SelectSlot(Index);
}

void UBeekeeperHotbarComponent::HandleWheelInput(bool bForward)
{
	if (SelectedIndex == INDEX_NONE)
	{
		SelectSlot(0);
		return;
	}

	const int32 Offset = bForward ? 1 : -1;
	const int32 NextIndex = (SelectedIndex + Offset + SlotCount) % SlotCount;
	SelectSlot(NextIndex);
}

void UBeekeeperHotbarComponent::SelectSlot(int32 Index)
{
	if (!IsIndexValid(Index) || !IsSlotEnabled(Index))
	{
		return;
	}

	if (SelectedIndex == Index)
	{
		return;
	}

	SelectedIndex = Index;
	BroadcastHotbarChanged();
}

void UBeekeeperHotbarComponent::ClearSelection()
{
	if (SelectedIndex == INDEX_NONE)
	{
		return;
	}

	SelectedIndex = INDEX_NONE;
	BroadcastHotbarChanged();
}

void UBeekeeperHotbarComponent::ApplyFocusRule(bool bEngaged, const FFocusItemRule& Rule)
{
	const bool bWasEngaged = bIsEngagedFocusActive;
	bIsEngagedFocusActive = bEngaged;
	ActiveFocusRule = Rule;

	bool bSelectionChanged = false;
	if (!bWasEngaged && bIsEngagedFocusActive && SelectedIndex != INDEX_NONE)
	{
		SelectedIndex = INDEX_NONE;
		bSelectionChanged = true;
	}

	const bool bSlotsChanged = ReevaluateSlotsInternal();

	if (bWasEngaged != bIsEngagedFocusActive || bSelectionChanged || bSlotsChanged)
	{
		BroadcastHotbarChanged();
	}
}

void UBeekeeperHotbarComponent::ReevaluateSlots()
{
	if (ReevaluateSlotsInternal())
	{
		BroadcastHotbarChanged();
	}
}

FHotbarItemAcquireResult UBeekeeperHotbarComponent::TryAcquireItem(UItemDefinition* ItemDefinition, int32 Quantity)
{
	FHotbarItemAcquireResult Result;
	Result.RequestedQuantity = FMath::Max(0, Quantity);
	Result.RemainingQuantity = Result.RequestedQuantity;

	if (Slots.Num() != SlotCount)
	{
		Slots.SetNum(SlotCount);
	}

	if (!ItemDefinition)
	{
		Result.Message = FText::FromString(TEXT("Cannot acquire an item without a definition."));
		return Result;
	}

	if (Result.RequestedQuantity <= 0)
	{
		Result.Message = FText::FromString(TEXT("Acquire quantity must be greater than zero."));
		return Result;
	}

	const int32 MaxStack = FMath::Max(1, ItemDefinition->MaxStack);
	bool bHotbarChanged = false;

	for (FHotbarSlotData& Slot : Slots)
	{
		UItemInstance* ExistingItemInstance = Cast<UItemInstance>(Slot.ItemInstance);
		if (!ExistingItemInstance || ExistingItemInstance->GetDefinition() != ItemDefinition)
		{
			continue;
		}

		const int32 CurrentStackCount = ExistingItemInstance->GetStackCount();
		const int32 AvailableSpace = FMath::Max(0, MaxStack - CurrentStackCount);
		if (AvailableSpace <= 0)
		{
			continue;
		}

		const int32 QuantityToAdd = FMath::Min(Result.RemainingQuantity, AvailableSpace);
		ExistingItemInstance->SetStackCount(CurrentStackCount + QuantityToAdd);
		Result.AddedQuantity += QuantityToAdd;
		Result.RemainingQuantity -= QuantityToAdd;
		bHotbarChanged = true;

		if (Result.RemainingQuantity <= 0)
		{
			break;
		}
	}

	while (Result.RemainingQuantity > 0)
	{
		const int32 EmptySlotIndex = FindFirstEmptySlot();
		if (EmptySlotIndex == INDEX_NONE)
		{
			break;
		}

		const int32 StackQuantity = FMath::Min(Result.RemainingQuantity, MaxStack);
		UItemInstance* NewItemInstance = CreateItemInstance(ItemDefinition, StackQuantity);
		if (!NewItemInstance)
		{
			Result.Message = FText::FromString(TEXT("Failed to create a new item instance for the hotbar."));
			break;
		}

		Slots[EmptySlotIndex].ItemInstance = NewItemInstance;
		Result.AddedQuantity += StackQuantity;
		Result.RemainingQuantity -= StackQuantity;
		bHotbarChanged = true;
	}

	Result.bSuccess = Result.AddedQuantity > 0 && Result.RemainingQuantity == 0;
	Result.bPartiallySucceeded = Result.AddedQuantity > 0 && Result.RemainingQuantity > 0;

	if (Result.bSuccess)
	{
		Result.Message = FText::Format(
			NSLOCTEXT("BeekeeperHotbar", "AcquireItemSuccess", "Added {0} item(s) to the hotbar."),
			FText::AsNumber(Result.AddedQuantity));
	}
	else if (Result.bPartiallySucceeded)
	{
		Result.Message = FText::Format(
			NSLOCTEXT("BeekeeperHotbar", "AcquireItemPartial", "Added {0} item(s). {1} item(s) could not fit in the hotbar."),
			FText::AsNumber(Result.AddedQuantity),
			FText::AsNumber(Result.RemainingQuantity));
	}
	else if (Result.Message.IsEmpty())
	{
		Result.Message = NSLOCTEXT("BeekeeperHotbar", "AcquireItemFailed", "No hotbar space was available for this item.");
	}

	if (bHotbarChanged)
	{
		ReevaluateSlotsInternal();
		BroadcastHotbarChanged();
	}

	return Result;
}

bool UBeekeeperHotbarComponent::ReevaluateSlotsInternal()
{
	if (Slots.Num() != SlotCount)
	{
		Slots.SetNum(SlotCount);
	}

	bool bHasChanged = false;
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		const bool bNewEnabled = IsSlotAllowedByActiveRule(Index);
		if (Slots[Index].bIsEnabled != bNewEnabled)
		{
			Slots[Index].bIsEnabled = bNewEnabled;
			bHasChanged = true;
		}
	}

	if (ShouldClearSelectedSlot())
	{
		SelectedIndex = INDEX_NONE;
		bHasChanged = true;
	}

	if (bHasChanged)
	{
		return true;
	}

	return false;
}

UObject* UBeekeeperHotbarComponent::GetSelectedItem() const
{
	return IsIndexValid(SelectedIndex) ? Slots[SelectedIndex].ItemInstance.Get() : nullptr;
}

UItemInstance* UBeekeeperHotbarComponent::GetSelectedItemInstance() const
{
	return Cast<UItemInstance>(GetSelectedItem());
}

EHotbarPresentationMode UBeekeeperHotbarComponent::GetPresentationMode() const
{
	if (SelectedIndex == INDEX_NONE || !GetSelectedItem())
	{
		return EHotbarPresentationMode::None;
	}

	return bIsEngagedFocusActive ? EHotbarPresentationMode::OnCursor : EHotbarPresentationMode::InHand;
}

void UBeekeeperHotbarComponent::SetSlotItem(int32 Index, UObject* NewItemInstance)
{
	if (!IsIndexValid(Index))
	{
		return;
	}

	if (Slots[Index].ItemInstance == NewItemInstance)
	{
		return;
	}

	Slots[Index].ItemInstance = NewItemInstance;

	ReevaluateSlotsInternal();
	BroadcastHotbarChanged();
}

FText UBeekeeperHotbarComponent::GetSelectedItemDisplayName() const
{
	const UItemInstance* ItemInstance = GetSelectedItemInstance();
	return ItemInstance ? ItemInstance->GetDisplayName() : FText::GetEmpty();
}

UTexture2D* UBeekeeperHotbarComponent::GetSelectedItemIcon() const
{
	const UItemInstance* ItemInstance = GetSelectedItemInstance();
	return ItemInstance ? ItemInstance->GetIcon() : nullptr;
}

int32 UBeekeeperHotbarComponent::GetSelectedItemStackCount() const
{
	const UItemInstance* ItemInstance = GetSelectedItemInstance();
	return ItemInstance ? ItemInstance->GetStackCount() : 0;
}

bool UBeekeeperHotbarComponent::IsSlotEnabled(int32 Index) const
{
	return IsIndexValid(Index) && Slots[Index].bIsEnabled;
}

void UBeekeeperHotbarComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABeekeeperCharacter>(GetOwner());
	InitializeSlots();

	if (!OwnerCharacter)
	{
		return;
	}

	if (UBeekeeperFocusComponent* FocusComponent = OwnerCharacter->GetBeekeeperFocus())
	{
		FocusComponent->OnFocusRuleChanged.AddDynamic(this, &UBeekeeperHotbarComponent::HandleFocusRuleChanged);
	}
}

void UBeekeeperHotbarComponent::HandleFocusRuleChanged(bool bHasFocusTarget, FFocusItemRule FocusItemRule)
{
	ApplyFocusRule(bHasFocusTarget, FocusItemRule);
}

void UBeekeeperHotbarComponent::BroadcastHotbarChanged()
{
	OnHotbarChanged.Broadcast();
}

bool UBeekeeperHotbarComponent::IsIndexValid(int32 Index) const
{
	return Slots.IsValidIndex(Index);
}

FGameplayTagContainer UBeekeeperHotbarComponent::GetItemTagsForSlot(int32 Index) const
{
	FGameplayTagContainer ItemTags;
	if (!IsIndexValid(Index))
	{
		return ItemTags;
	}

	UObject* ItemInstance = Slots[Index].ItemInstance.Get();
	if (!ItemInstance || !ItemInstance->GetClass()->ImplementsInterface(UHotbarItemInterface::StaticClass()))
	{
		return ItemTags;
	}

	return IHotbarItemInterface::Execute_GetHotbarItemTags(ItemInstance);
}

bool UBeekeeperHotbarComponent::IsSlotAllowedByActiveRule(int32 Index) const
{
	if (!IsIndexValid(Index))
	{
		return false;
	}

	if (!bIsEngagedFocusActive)
	{
		return true;
	}

	if (!Slots[Index].ItemInstance)
	{
		return true;
	}

	const FGameplayTagContainer& AllowedItemTags = ActiveFocusRule.AllowedItemTags;
	if (AllowedItemTags.IsEmpty())
	{
		return false;
	}

	if (AllItemsRootTag.IsValid() && AllowedItemTags.HasTag(AllItemsRootTag))
	{
		return true;
	}

	const FGameplayTagContainer ItemTags = GetItemTagsForSlot(Index);
	return ItemTags.HasAny(AllowedItemTags);
}

bool UBeekeeperHotbarComponent::ShouldClearSelectedSlot() const
{
	return IsIndexValid(SelectedIndex) && !IsSlotEnabled(SelectedIndex);
}

int32 UBeekeeperHotbarComponent::FindFirstEmptySlot() const
{
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (!Slots[Index].ItemInstance)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

UItemInstance* UBeekeeperHotbarComponent::CreateItemInstance(UItemDefinition* ItemDefinition, int32 StackCount)
{
	if (!ItemDefinition || StackCount <= 0)
	{
		return nullptr;
	}

	UItemInstance* NewItemInstance = NewObject<UItemInstance>(this);
	if (!NewItemInstance)
	{
		return nullptr;
	}

	NewItemInstance->InitializeFromDefinition(ItemDefinition, StackCount);
	return NewItemInstance;
}
