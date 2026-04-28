// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BeekeeperHotbarComponent.h"

#include "Public/BeekeeperCharacter.h"
#include "Public/BeekeeperFocusComponent.h"
#include "Public/FocusActionComponent.h"
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
	if (bIsEngagedFocusActive && ActiveFocusAction && ActiveFocusAction->ShouldBlockHotbarSlotInputWhileEngaged())
	{
		return;
	}

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

// void UBeekeeperHotbarComponent::HandleWheelInput(bool bForward)
// {
// 	if (SelectedIndex == INDEX_NONE)
// 	{
// 		SelectSlot(0);
// 		return;
// 	}
//
// 	const int32 Offset = bForward ? 1 : -1;
// 	const int32 NextIndex = (SelectedIndex + Offset + SlotCount) % SlotCount;
// 	SelectSlot(NextIndex);
// }

void UBeekeeperHotbarComponent::HandleWheelInput(bool bForward)
{
	if (bIsEngagedFocusActive && ActiveFocusAction && ActiveFocusAction->ShouldBlockHotbarWheelInputWhileEngaged())
	{
		return;
	}

	if (SlotCount <= 0)
	{
		return;
	}

	if (SelectedIndex == INDEX_NONE)
	{
		SelectSlot(0);
		return;
	}

	const int32 Offset = bForward ? 1 : -1;

	int32 NextIndex = SelectedIndex;

	for (int32 i = 0; i < SlotCount; ++i)
	{
		NextIndex = (NextIndex + Offset + SlotCount) % SlotCount;

		if (IsSlotEnabled(NextIndex))
		{
			SelectSlot(NextIndex);
			return;
		}
	}
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

	const bool bShouldClearSelectionOnEngage =
		ShouldClearSelectionByActiveFocusPolicy();

	bool bSelectionChanged = false;
	if (!bWasEngaged && bIsEngagedFocusActive && bShouldClearSelectionOnEngage && SelectedIndex != INDEX_NONE)
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

void UBeekeeperHotbarComponent::NotifyHotbarItemsChanged()
{
	ReevaluateSlotsInternal();
	BroadcastHotbarChanged();
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

	if (bIsEngagedFocusActive && ActiveFocusAction)
	{
		return ActiveFocusAction->GetHotbarPresentationModeWhileEngaged();
	}

	return EHotbarPresentationMode::InHand;
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

bool UBeekeeperHotbarComponent::SwapSlots(const int32 FromIndex, const int32 ToIndex)
{
	if (!IsIndexValid(FromIndex) || !IsIndexValid(ToIndex) || FromIndex == ToIndex)
	{
		return false;
	}

	Slots.Swap(FromIndex, ToIndex);
	ReevaluateSlotsInternal();
	BroadcastHotbarChanged();
	return true;
}

FItemSlotMoveResult UBeekeeperHotbarComponent::MovePartialToSlot(const int32 FromIndex, const int32 ToIndex, const int32 Quantity)
{
	FItemSlotMoveResult Result;
	Result.RequestedQuantity = FMath::Max(0, Quantity);
	Result.RemainingQuantity = Result.RequestedQuantity;

	if (!IsIndexValid(FromIndex) || !IsIndexValid(ToIndex) || FromIndex == ToIndex || Result.RequestedQuantity <= 0)
	{
		return Result;
	}

	UItemInstance* SourceItem = Cast<UItemInstance>(Slots[FromIndex].ItemInstance.Get());
	if (!SourceItem || !SourceItem->GetDefinition())
	{
		return Result;
	}

	UItemDefinition* SourceDefinition = SourceItem->GetDefinition();
	const int32 MaxStack = FMath::Max(1, SourceDefinition->MaxStack);
	int32 SourceCount = SourceItem->GetStackCount();
	if (SourceCount <= 0)
	{
		return Result;
	}

	int32 QuantityToMove = FMath::Min(Result.RequestedQuantity, SourceCount);
	UItemInstance* TargetItem = Cast<UItemInstance>(Slots[ToIndex].ItemInstance.Get());

	if (!TargetItem)
	{
		const int32 StackToCreate = FMath::Min(QuantityToMove, MaxStack);
		UItemInstance* NewItem = CreateItemInstance(SourceDefinition, StackToCreate);
		if (!NewItem)
		{
			return Result;
		}

		Slots[ToIndex].ItemInstance = NewItem;
		Result.MovedQuantity += StackToCreate;
		QuantityToMove -= StackToCreate;
	}
	else
	{
		if (TargetItem->GetDefinition() != SourceDefinition)
		{
			Result.Message = FText::FromString(TEXT("Partial move failed: target has a different item type."));
			return Result;
		}

		const int32 TargetCount = TargetItem->GetStackCount();
		const int32 Available = FMath::Max(0, MaxStack - TargetCount);
		const int32 AddCount = FMath::Min(QuantityToMove, Available);
		if (AddCount > 0)
		{
			TargetItem->SetStackCount(TargetCount + AddCount);
			Result.MovedQuantity += AddCount;
			QuantityToMove -= AddCount;
		}

		while (QuantityToMove > 0)
		{
			const int32 EmptyIndex = FindFirstEmptySlot();
			if (EmptyIndex == INDEX_NONE || EmptyIndex == ToIndex)
			{
				break;
			}

			const int32 StackToCreate = FMath::Min(QuantityToMove, MaxStack);
			UItemInstance* NewItem = CreateItemInstance(SourceDefinition, StackToCreate);
			if (!NewItem)
			{
				break;
			}

			Slots[EmptyIndex].ItemInstance = NewItem;
			Result.MovedQuantity += StackToCreate;
			QuantityToMove -= StackToCreate;
		}
	}

	if (Result.MovedQuantity > 0)
	{
		SourceItem->SetStackCount(SourceCount - Result.MovedQuantity);
		if (SourceItem->GetStackCount() <= 0)
		{
			Slots[FromIndex].ItemInstance = nullptr;
		}

		ReevaluateSlotsInternal();
		BroadcastHotbarChanged();
		Result.bSuccess = true;
	}

	Result.RemainingQuantity = Result.RequestedQuantity - Result.MovedQuantity;
	return Result;
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

	FocusComponent = OwnerCharacter->GetBeekeeperFocus();
	if (FocusComponent)
	{
		FocusComponent->OnFocusRuleChanged.AddDynamic(this, &UBeekeeperHotbarComponent::HandleFocusRuleChanged);
	}
}

void UBeekeeperHotbarComponent::HandleFocusRuleChanged(bool bHasFocusTarget, FFocusItemRule FocusItemRule)
{
	ActiveFocusAction = (bHasFocusTarget && FocusComponent) ? FocusComponent->GetEngagedFocusAction() : nullptr;
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

	if (AllItemsRootTag.IsValid() && AllowedItemTags.HasTagExact(AllItemsRootTag))
	{
		return true;
	}

	const FGameplayTagContainer ItemTags = GetItemTagsForSlot(Index);
	return ItemTags.HasAny(AllowedItemTags);
}

bool UBeekeeperHotbarComponent::ShouldClearSelectedSlot() const
{
	if (!IsIndexValid(SelectedIndex) || IsSlotEnabled(SelectedIndex))
	{
		return false;
	}

	if (!ShouldClearSelectionByActiveFocusPolicy())
	{
		return false;
	}

	return true;
}

bool UBeekeeperHotbarComponent::ShouldClearSelectionByActiveFocusPolicy() const
{
	return !bIsEngagedFocusActive
		|| !ActiveFocusAction
		|| ActiveFocusAction->ShouldClearHotbarSelectionOnFocusEngaged();
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
