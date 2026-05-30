#include "Inventory/StorageBoxComponent.h"

#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/ItemStackMoveUtils.h"

UStorageBoxComponent::UStorageBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.SetNum(DefaultSlotCount);
}

void UStorageBoxComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSlots();
}

void UStorageBoxComponent::InitializeSlots()
{
	const int32 TargetSlotCount = FMath::Max(1, DefaultSlotCount);
	if (Slots.Num() == TargetSlotCount)
	{
		return;
	}

	Slots.SetNum(TargetSlotCount);
	BroadcastStorageChanged();
}

UItemInstance* UStorageBoxComponent::GetItemAt(const int32 Index) const
{
	return IsIndexValid(Index) ? Slots[Index].ItemInstance.Get() : nullptr;
}

bool UStorageBoxComponent::IsIndexValid(const int32 Index) const
{
	return Slots.IsValidIndex(Index);
}

bool UStorageBoxComponent::SetSlotItem(const int32 Index, UItemInstance* ItemInstance)
{
	if (!IsIndexValid(Index))
	{
		return false;
	}

	if (Slots[Index].ItemInstance == ItemInstance)
	{
		return false;
	}

	Slots[Index].ItemInstance = ItemInstance;
	BroadcastStorageChanged();
	return true;
}

bool UStorageBoxComponent::ClearSlot(const int32 Index)
{
	return SetSlotItem(Index, nullptr);
}

bool UStorageBoxComponent::SwapStorageSlots(const int32 FromIndex, const int32 ToIndex)
{
	if (!IsIndexValid(FromIndex) || !IsIndexValid(ToIndex) || FromIndex == ToIndex)
	{
		return false;
	}

	Slots.Swap(FromIndex, ToIndex);
	BroadcastStorageChanged();
	return true;
}

bool UStorageBoxComponent::MoveHotbarItemToStorage(UBeekeeperHotbarComponent* HotbarComponent, const int32 HotbarIndex, const int32 StorageIndex)
{
	if (!IsHotbarIndexValid(HotbarComponent, HotbarIndex) || !IsIndexValid(StorageIndex))
	{
		return false;
	}

	UObject* HotbarItem = GetHotbarItemAt(HotbarComponent, HotbarIndex);
	if (!HotbarItem)
	{
		return false;
	}
	UItemInstance* HotbarItemInstance = Cast<UItemInstance>(HotbarItem);
	if (!HotbarItemInstance)
	{
		return false;
	}

	UItemInstance* StorageItem = Slots[StorageIndex].ItemInstance.Get();
	if (!StorageItem)
	{
		Slots[StorageIndex].ItemInstance = HotbarItemInstance;
		HotbarComponent->SetSlotItem(HotbarIndex, nullptr);
		BroadcastStorageChanged();
		return true;
	}

	return SwapHotbarAndStorage(HotbarComponent, HotbarIndex, StorageIndex);
}

bool UStorageBoxComponent::MoveStorageItemToHotbar(UBeekeeperHotbarComponent* HotbarComponent, const int32 StorageIndex, const int32 HotbarIndex)
{
	if (!IsHotbarIndexValid(HotbarComponent, HotbarIndex) || !IsIndexValid(StorageIndex))
	{
		return false;
	}

	UItemInstance* StorageItem = Slots[StorageIndex].ItemInstance.Get();
	if (!StorageItem)
	{
		return false;
	}

	UObject* HotbarItem = GetHotbarItemAt(HotbarComponent, HotbarIndex);
	if (!HotbarItem)
	{
		Slots[StorageIndex].ItemInstance = nullptr;
		HotbarComponent->SetSlotItem(HotbarIndex, StorageItem);
		BroadcastStorageChanged();
		return true;
	}

	return SwapHotbarAndStorage(HotbarComponent, HotbarIndex, StorageIndex);
}

bool UStorageBoxComponent::SwapHotbarAndStorage(UBeekeeperHotbarComponent* HotbarComponent, const int32 HotbarIndex, const int32 StorageIndex)
{
	if (!IsHotbarIndexValid(HotbarComponent, HotbarIndex) || !IsIndexValid(StorageIndex))
	{
		return false;
	}

	UObject* HotbarItem = GetHotbarItemAt(HotbarComponent, HotbarIndex);
	UItemInstance* HotbarItemInstance = Cast<UItemInstance>(HotbarItem);
	UItemInstance* StorageItem = Slots[StorageIndex].ItemInstance.Get();
	if (HotbarItem && !HotbarItemInstance)
	{
		return false;
	}

	if (!HotbarItem && !StorageItem)
	{
		return false;
	}

	Slots[StorageIndex].ItemInstance = HotbarItemInstance;
	HotbarComponent->SetSlotItem(HotbarIndex, StorageItem);
	BroadcastStorageChanged();
	return true;
}

FItemSlotMoveResult UStorageBoxComponent::MovePartialStorageToStorage(const int32 FromIndex, const int32 ToIndex, const int32 Quantity)
{
	FItemSlotMoveResult Result;
	Result.RequestedQuantity = FMath::Max(0, Quantity);
	Result.RemainingQuantity = Result.RequestedQuantity;

	if (!IsIndexValid(FromIndex) || !IsIndexValid(ToIndex) || FromIndex == ToIndex || Result.RequestedQuantity <= 0)
	{
		return Result;
	}

	UItemInstance* SourceItem = Slots[FromIndex].ItemInstance.Get();
	if (!SourceItem || !SourceItem->GetDefinition())
	{
		return Result;
	}

	UItemDefinition* Definition = SourceItem->GetDefinition();
	const int32 MaxStack = ItemStackMoveUtils::ResolveMaxStack(Definition);
	int32 QuantityToMove = ItemStackMoveUtils::ClampQuantityToAvailable(Result.RequestedQuantity, SourceItem->GetStackCount());
	UItemInstance* TargetItem = Slots[ToIndex].ItemInstance.Get();

	if (!TargetItem)
	{
		const int32 CreateCount = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
		UItemInstance* NewItem = CreateStorageItemInstance(Definition, CreateCount, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
		if (!NewItem)
		{
			return Result;
		}
		Slots[ToIndex].ItemInstance = NewItem;
		Result.MovedQuantity += CreateCount;
		QuantityToMove -= CreateCount;
	}
	else
	{
		if (!ItemStackMoveUtils::CanMergeItemStacks(TargetItem, SourceItem))
		{
			Result.Message = FText::FromString(TEXT("Partial move failed: target has a different item type."));
			return Result;
		}

		const int32 MergeCount = ItemStackMoveUtils::MergeIntoStack(TargetItem, QuantityToMove, MaxStack);
		if (MergeCount > 0)
		{
			Result.MovedQuantity += MergeCount;
			QuantityToMove -= MergeCount;
		}

		while (QuantityToMove > 0)
		{
			const int32 EmptyIndex = FindFirstEmptyStorageSlot();
			if (EmptyIndex == INDEX_NONE || EmptyIndex == ToIndex)
			{
				break;
			}

			const int32 CreateCount = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
			UItemInstance* NewItem = CreateStorageItemInstance(Definition, CreateCount, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
			if (!NewItem)
			{
				break;
			}

			Slots[EmptyIndex].ItemInstance = NewItem;
			Result.MovedQuantity += CreateCount;
			QuantityToMove -= CreateCount;
		}
	}

	if (Result.MovedQuantity > 0)
	{
		SourceItem->SetStackCount(SourceItem->GetStackCount() - Result.MovedQuantity);
		if (SourceItem->GetStackCount() <= 0)
		{
			Slots[FromIndex].ItemInstance = nullptr;
		}
		BroadcastStorageChanged();
		Result.bSuccess = true;
	}

	ItemStackMoveUtils::UpdateRemainingQuantity(Result);
	return Result;
}

FItemSlotMoveResult UStorageBoxComponent::MovePartialStorageToHotbar(
	UBeekeeperHotbarComponent* HotbarComponent,
	const int32 StorageIndex,
	const int32 HotbarIndex,
	const int32 Quantity)
{
	FItemSlotMoveResult Result;
	Result.RequestedQuantity = FMath::Max(0, Quantity);
	Result.RemainingQuantity = Result.RequestedQuantity;

	if (!IsIndexValid(StorageIndex) || !IsHotbarIndexValid(HotbarComponent, HotbarIndex) || Result.RequestedQuantity <= 0)
	{
		return Result;
	}

	UItemInstance* SourceItem = Slots[StorageIndex].ItemInstance.Get();
	if (!SourceItem || !SourceItem->GetDefinition())
	{
		return Result;
	}

	UItemDefinition* Definition = SourceItem->GetDefinition();
	const int32 MaxStack = ItemStackMoveUtils::ResolveMaxStack(Definition);
	int32 QuantityToMove = ItemStackMoveUtils::ClampQuantityToAvailable(Result.RequestedQuantity, SourceItem->GetStackCount());

	UItemInstance* TargetItem = Cast<UItemInstance>(GetHotbarItemAt(HotbarComponent, HotbarIndex));
	if (!TargetItem)
	{
		const int32 CreateCount = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
		UItemInstance* NewItem = ItemStackMoveUtils::CreateItemInstance(HotbarComponent, Definition, CreateCount, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
		if (!NewItem)
		{
			return Result;
		}
		HotbarComponent->SetSlotItem(HotbarIndex, NewItem);
		Result.MovedQuantity += CreateCount;
		QuantityToMove -= CreateCount;
	}
	else
	{
		if (!ItemStackMoveUtils::CanMergeItemStacks(TargetItem, SourceItem))
		{
			Result.Message = FText::FromString(TEXT("Partial move failed: target has a different item type."));
			return Result;
		}

		const int32 MergeCount = ItemStackMoveUtils::MergeIntoStack(TargetItem, QuantityToMove, MaxStack);
		if (MergeCount > 0)
		{
			Result.MovedQuantity += MergeCount;
			QuantityToMove -= MergeCount;
		}

		if (QuantityToMove > 0)
		{
			const TArray<FHotbarSlotData>& HotbarSlots = HotbarComponent->GetSlots();
			for (int32 Index = 0; Index < HotbarSlots.Num() && QuantityToMove > 0; ++Index)
			{
				if (HotbarSlots[Index].ItemInstance)
				{
					continue;
				}

				const int32 CreateCount = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
				UItemInstance* NewItem = ItemStackMoveUtils::CreateItemInstance(HotbarComponent, Definition, CreateCount, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
				if (!NewItem)
				{
					break;
				}
				HotbarComponent->SetSlotItem(Index, NewItem);
				Result.MovedQuantity += CreateCount;
				QuantityToMove -= CreateCount;
			}
		}
	}

	if (Result.MovedQuantity > 0)
	{
		SourceItem->SetStackCount(SourceItem->GetStackCount() - Result.MovedQuantity);
		if (SourceItem->GetStackCount() <= 0)
		{
			Slots[StorageIndex].ItemInstance = nullptr;
		}

		HotbarComponent->NotifyHotbarItemsChanged();
		BroadcastStorageChanged();
		Result.bSuccess = true;
	}

	ItemStackMoveUtils::UpdateRemainingQuantity(Result);
	return Result;
}

FItemSlotMoveResult UStorageBoxComponent::MovePartialHotbarToStorage(
	UBeekeeperHotbarComponent* HotbarComponent,
	const int32 HotbarIndex,
	const int32 StorageIndex,
	const int32 Quantity)
{
	FItemSlotMoveResult Result;
	Result.RequestedQuantity = FMath::Max(0, Quantity);
	Result.RemainingQuantity = Result.RequestedQuantity;

	if (!IsHotbarIndexValid(HotbarComponent, HotbarIndex) || !IsIndexValid(StorageIndex) || Result.RequestedQuantity <= 0)
	{
		return Result;
	}

	UItemInstance* SourceItem = Cast<UItemInstance>(GetHotbarItemAt(HotbarComponent, HotbarIndex));
	if (!SourceItem || !SourceItem->GetDefinition())
	{
		return Result;
	}

	UItemDefinition* Definition = SourceItem->GetDefinition();
	const int32 MaxStack = ItemStackMoveUtils::ResolveMaxStack(Definition);
	int32 QuantityToMove = ItemStackMoveUtils::ClampQuantityToAvailable(Result.RequestedQuantity, SourceItem->GetStackCount());

	UItemInstance* TargetItem = Slots[StorageIndex].ItemInstance.Get();
	if (!TargetItem)
	{
		const int32 CreateCount = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
		UItemInstance* NewItem = CreateStorageItemInstance(Definition, CreateCount, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
		if (!NewItem)
		{
			return Result;
		}

		Slots[StorageIndex].ItemInstance = NewItem;
		Result.MovedQuantity += CreateCount;
		QuantityToMove -= CreateCount;
	}
	else
	{
		if (!ItemStackMoveUtils::CanMergeItemStacks(TargetItem, SourceItem))
		{
			Result.Message = FText::FromString(TEXT("Partial move failed: target has a different item type."));
			return Result;
		}

		const int32 MergeCount = ItemStackMoveUtils::MergeIntoStack(TargetItem, QuantityToMove, MaxStack);
		if (MergeCount > 0)
		{
			Result.MovedQuantity += MergeCount;
			QuantityToMove -= MergeCount;
		}

		while (QuantityToMove > 0)
		{
			const int32 EmptyIndex = FindFirstEmptyStorageSlot();
			if (EmptyIndex == INDEX_NONE || EmptyIndex == StorageIndex)
			{
				break;
			}

			const int32 CreateCount = ItemStackMoveUtils::ClampQuantityToAvailable(QuantityToMove, MaxStack);
			UItemInstance* NewItem = CreateStorageItemInstance(Definition, CreateCount, SourceItem->HasDurability(), SourceItem->GetCurrentDurability());
			if (!NewItem)
			{
				break;
			}

			Slots[EmptyIndex].ItemInstance = NewItem;
			Result.MovedQuantity += CreateCount;
			QuantityToMove -= CreateCount;
		}
	}

	if (Result.MovedQuantity > 0)
	{
		SourceItem->SetStackCount(SourceItem->GetStackCount() - Result.MovedQuantity);
		if (SourceItem->GetStackCount() <= 0)
		{
			HotbarComponent->SetSlotItem(HotbarIndex, nullptr);
		}
		else
		{
			HotbarComponent->NotifyHotbarItemsChanged();
		}

		BroadcastStorageChanged();
		Result.bSuccess = true;
	}

	ItemStackMoveUtils::UpdateRemainingQuantity(Result);
	return Result;
}

void UStorageBoxComponent::BroadcastStorageChanged()
{
	OnStorageChanged.Broadcast();
}

bool UStorageBoxComponent::IsHotbarIndexValid(const UBeekeeperHotbarComponent* HotbarComponent, const int32 Index) const
{
	if (!HotbarComponent)
	{
		return false;
	}

	const TArray<FHotbarSlotData>& HotbarSlots = HotbarComponent->GetSlots();
	return HotbarSlots.IsValidIndex(Index);
}

UObject* UStorageBoxComponent::GetHotbarItemAt(const UBeekeeperHotbarComponent* HotbarComponent, const int32 Index) const
{
	if (!IsHotbarIndexValid(HotbarComponent, Index))
	{
		return nullptr;
	}

	return HotbarComponent->GetSlots()[Index].ItemInstance.Get();
}

int32 UStorageBoxComponent::FindFirstEmptyStorageSlot() const
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

UItemInstance* UStorageBoxComponent::CreateStorageItemInstance(UItemDefinition* ItemDefinition, const int32 StackCount)
{
	return CreateStorageItemInstance(ItemDefinition, StackCount, false, 0.0f);
}

UItemInstance* UStorageBoxComponent::CreateStorageItemInstance(UItemDefinition* ItemDefinition, int32 StackCount, bool bHasDurabilityOverride, float DurabilityOverride)
{
	return ItemStackMoveUtils::CreateItemInstance(this, ItemDefinition, StackCount, bHasDurabilityOverride, DurabilityOverride);
}
