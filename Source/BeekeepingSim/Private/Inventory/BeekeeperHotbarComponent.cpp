// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/BeekeeperHotbarComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Focus/BeekeeperFocusComponent.h"
#include "Focus/FocusActionComponent.h"
#include "Inventory/HotbarItemInterface.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/ItemStackMoveUtils.h"

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

	if (!IsIndexValid(LastSelectedIndex))
	{
		LastSelectedIndex = 0;
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
	LastSelectedIndex = Index;
	BroadcastHotbarChanged();
}

void UBeekeeperHotbarComponent::ClearSelection()
{
	if (SelectedIndex == INDEX_NONE)
	{
		return;
	}

	RememberSelectedIndex();
	SelectedIndex = INDEX_NONE;
	BroadcastHotbarChanged();
}

void UBeekeeperHotbarComponent::ToggleSelectionFromLastSelectedSlot()
{
	if (bIsEngagedFocusActive && ActiveFocusAction && ActiveFocusAction->ShouldBlockHotbarSlotInputWhileEngaged())
	{
		return;
	}

	if (SelectedIndex != INDEX_NONE)
	{
		RememberSelectedIndex();
		ClearSelection();
		return;
	}

	const int32 TargetIndex = ResolveToggleFallbackSelectionIndex();
	if (TargetIndex != INDEX_NONE)
	{
		SelectSlot(TargetIndex);
	}
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
		RememberSelectedIndex();
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
	FItemAcquireSpec AcquireSpec;
	AcquireSpec.ItemDefinition = ItemDefinition;
	AcquireSpec.Quantity = Quantity;
	if (ItemDefinition && ItemDefinition->bUsesDurability)
	{
		AcquireSpec.bOverrideDurability = true;
		AcquireSpec.Durability = FMath::Max(0.0f, ItemDefinition->MaxDurability);
	}

	return TryAcquireItemBySpec(AcquireSpec);
}

FHotbarItemAcquireResult UBeekeeperHotbarComponent::TryAcquireItemBySpec(const FItemAcquireSpec& AcquireSpec)
{
	if (Slots.Num() != SlotCount)
	{
		Slots.SetNum(SlotCount);
	}

	bool bHotbarChanged = false;
	FHotbarItemAcquireResult Result = EvaluateAcquireItemBySpec(AcquireSpec, true, bHotbarChanged);
	if (bHotbarChanged)
	{
		ReevaluateSlotsInternal();
		BroadcastHotbarChanged();
	}

	return Result;
}

FHotbarItemAcquireResult UBeekeeperHotbarComponent::PreviewAcquireItemBySpec(const FItemAcquireSpec& AcquireSpec) const
{
	bool bIgnoredHotbarChanged = false;
	return const_cast<UBeekeeperHotbarComponent*>(this)->EvaluateAcquireItemBySpec(AcquireSpec, false, bIgnoredHotbarChanged);
}

FHotbarItemAcquireResult UBeekeeperHotbarComponent::EvaluateAcquireItemBySpec(const FItemAcquireSpec& AcquireSpec, bool bApplyMutation, bool& bOutHotbarChanged)
{
	bOutHotbarChanged = false;

	FHotbarItemAcquireResult Result;
	Result.RequestedQuantity = FMath::Max(0, AcquireSpec.Quantity);
	Result.RemainingQuantity = Result.RequestedQuantity;
	Result.LastModifiedItemInstance = nullptr;

	UItemDefinition* ItemDefinition = AcquireSpec.ItemDefinition.Get();
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

	const bool bHasDurabilityOverride = ItemDefinition->bUsesDurability;
	const float DurabilityOverride = ItemDefinition->bUsesDurability
		? (AcquireSpec.bOverrideDurability ? AcquireSpec.Durability : FMath::Max(0.0f, ItemDefinition->MaxDurability))
		: 0.0f;

	const int32 MaxStack = ItemStackMoveUtils::ResolveMaxStack(ItemDefinition);
	const int32 SlotNum = Slots.Num();

	for (int32 SlotIndex = 0; SlotIndex < SlotNum && Result.RemainingQuantity > 0; ++SlotIndex)
	{
		FHotbarSlotData& Slot = Slots[SlotIndex];
		UItemInstance* ExistingItemInstance = Cast<UItemInstance>(Slot.ItemInstance);
		if (!ItemStackMoveUtils::HasCompatibleStackState(ExistingItemInstance, ItemDefinition, bHasDurabilityOverride, DurabilityOverride))
		{
			continue;
		}

		int32 QuantityToAdd = 0;
		if (bApplyMutation)
		{
			QuantityToAdd = ItemStackMoveUtils::MergeIntoStack(ExistingItemInstance, Result.RemainingQuantity, MaxStack);
			if (QuantityToAdd > 0)
			{
				Result.LastModifiedItemInstance = ExistingItemInstance;
			}
		}
		else
		{
			const int32 AvailableSpace = ItemStackMoveUtils::GetAvailableStackSpace(ExistingItemInstance, MaxStack);
			QuantityToAdd = ItemStackMoveUtils::ClampQuantityToAvailable(Result.RemainingQuantity, AvailableSpace);
		}

		if (QuantityToAdd <= 0)
		{
			continue;
		}

		Result.AddedQuantity += QuantityToAdd;
		Result.RemainingQuantity -= QuantityToAdd;
		Result.LastModifiedSlotIndex = SlotIndex;
		bOutHotbarChanged = bOutHotbarChanged || bApplyMutation;
	}

	if (Result.RemainingQuantity > 0)
	{
		TArray<int32> EmptySlotIndices;
		EmptySlotIndices.Reserve(SlotNum);
		for (int32 SlotIndex = 0; SlotIndex < SlotNum; ++SlotIndex)
		{
			if (!Slots[SlotIndex].ItemInstance)
			{
				EmptySlotIndices.Add(SlotIndex);
			}
		}

		for (const int32 EmptySlotIndex : EmptySlotIndices)
		{
			if (Result.RemainingQuantity <= 0)
			{
				break;
			}

			const int32 StackQuantity = ItemStackMoveUtils::ClampQuantityToAvailable(Result.RemainingQuantity, MaxStack);
			if (StackQuantity <= 0)
			{
				break;
			}

			if (bApplyMutation)
			{
				UItemInstance* NewItemInstance = CreateItemInstance(ItemDefinition, StackQuantity, bHasDurabilityOverride, DurabilityOverride);
				if (!NewItemInstance)
				{
					Result.Message = FText::FromString(TEXT("Failed to create a new item instance for the hotbar."));
					break;
				}

				Slots[EmptySlotIndex].ItemInstance = NewItemInstance;
				Result.LastModifiedItemInstance = NewItemInstance;
				bOutHotbarChanged = true;
			}

			Result.AddedQuantity += StackQuantity;
			Result.RemainingQuantity -= StackQuantity;
			Result.LastModifiedSlotIndex = EmptySlotIndex;
		}
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

	if (!bApplyMutation)
	{
		Result.LastModifiedItemInstance = nullptr;
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
		RememberSelectedIndex();
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
	const int32 MaxStack = ItemStackMoveUtils::ResolveMaxStack(SourceDefinition);
	int32 SourceCount = SourceItem->GetStackCount();
	if (SourceCount <= 0)
	{
		return Result;
	}

	int32 QuantityToMove = ItemStackMoveUtils::ClampQuantityToAvailable(Result.RequestedQuantity, SourceCount);
	UItemInstance* TargetItem = Cast<UItemInstance>(Slots[ToIndex].ItemInstance.Get());

	if (!TargetItem)
	{
		const int32 StackToCreate = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
		UItemInstance* NewItem = CreateItemInstance(SourceDefinition, StackToCreate, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
		if (!NewItem)
		{
			return Result;
		}
		NewItem->CopyRuntimeStateFrom(SourceItem);

		Slots[ToIndex].ItemInstance = NewItem;
		Result.MovedQuantity += StackToCreate;
		QuantityToMove -= StackToCreate;
	}
	else
	{
		if (!ItemStackMoveUtils::CanMergeItemStacks(TargetItem, SourceItem))
		{
			Result.Message = FText::FromString(TEXT("Partial move failed: target has a different item type."));
			return Result;
		}

		const int32 AddCount = ItemStackMoveUtils::MergeIntoStack(TargetItem, QuantityToMove, MaxStack);
		if (AddCount > 0)
		{
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

			const int32 StackToCreate = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
			UItemInstance* NewItem = CreateItemInstance(SourceDefinition, StackToCreate, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
			if (!NewItem)
			{
				break;
			}
			NewItem->CopyRuntimeStateFrom(SourceItem);

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

	ItemStackMoveUtils::UpdateRemainingQuantity(Result);
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

bool UBeekeeperHotbarComponent::ApplySelectedItemStackDelta(int32 StackDelta)
{
	if (StackDelta == 0 || !IsIndexValid(SelectedIndex))
	{
		return false;
	}

	UItemInstance* SelectedItem = Cast<UItemInstance>(Slots[SelectedIndex].ItemInstance.Get());
	if (!SelectedItem)
	{
		return false;
	}

	const int32 PreviousCount = SelectedItem->GetStackCount();
	SelectedItem->SetStackCount(PreviousCount + StackDelta);
	if (SelectedItem->GetStackCount() <= 0)
	{
		RememberSelectedIndex();
		Slots[SelectedIndex].ItemInstance = nullptr;
	}

	ReevaluateSlotsInternal();
	BroadcastHotbarChanged();
	return true;
}

FHotbarItemDurabilityMutationResult UBeekeeperHotbarComponent::ApplySelectedItemDurabilityDelta(float DurabilityDelta, bool bRemoveWhenDepleted)
{
	FHotbarItemDurabilityMutationResult Result;

	if (FMath::IsNearlyZero(DurabilityDelta))
	{
		Result.Message = FText::FromString(TEXT("Durability delta is zero."));
		return Result;
	}

	if (!IsIndexValid(SelectedIndex))
	{
		Result.Message = FText::FromString(TEXT("No selected slot exists."));
		return Result;
	}

	UItemInstance* SelectedItem = Cast<UItemInstance>(Slots[SelectedIndex].ItemInstance.Get());
	if (!SelectedItem)
	{
		Result.Message = FText::FromString(TEXT("No selected item exists."));
		return Result;
	}

	if (!SelectedItem->HasDurability())
	{
		Result.Message = FText::FromString(TEXT("Selected item does not use durability."));
		return Result;
	}

	Result.PreviousDurability = SelectedItem->GetCurrentDurability();
	const float MaxDurability = FMath::Max(0.0f, SelectedItem->GetMaxDurability());
	Result.NewDurability = FMath::Clamp(Result.PreviousDurability + DurabilityDelta, 0.0f, MaxDurability);
	if (FMath::IsNearlyEqual(Result.PreviousDurability, Result.NewDurability))
	{
		Result.Message = FText::FromString(TEXT("Durability was unchanged."));
		return Result;
	}

	SelectedItem->SetDurability(Result.NewDurability);
	Result.bApplied = true;

	if (Result.NewDurability <= 0.0f)
	{
		Result.bItemDepleted = true;
		if (bRemoveWhenDepleted)
		{
			RememberSelectedIndex();
			Slots[SelectedIndex].ItemInstance = nullptr;
			Result.bItemRemoved = true;
		}
	}

	ReevaluateSlotsInternal();
	BroadcastHotbarChanged();
	return Result;
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

void UBeekeeperHotbarComponent::RememberSelectedIndex()
{
	if (IsIndexValid(SelectedIndex))
	{
		LastSelectedIndex = SelectedIndex;
	}
}

int32 UBeekeeperHotbarComponent::ResolveToggleFallbackSelectionIndex() const
{
	if (IsIndexValid(LastSelectedIndex) && IsSlotEnabled(LastSelectedIndex))
	{
		return LastSelectedIndex;
	}

	if (IsIndexValid(0) && IsSlotEnabled(0))
	{
		return 0;
	}

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (IsSlotEnabled(Index))
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

UItemInstance* UBeekeeperHotbarComponent::CreateItemInstance(UItemDefinition* ItemDefinition, int32 StackCount)
{
	return CreateItemInstance(ItemDefinition, StackCount, false, 0.0f);
}

UItemInstance* UBeekeeperHotbarComponent::CreateItemInstance(UItemDefinition* ItemDefinition, int32 StackCount, bool bHasDurabilityOverride, float DurabilityOverride)
{
	return ItemStackMoveUtils::CreateItemInstance(this, ItemDefinition, StackCount, bHasDurabilityOverride, DurabilityOverride);
}
