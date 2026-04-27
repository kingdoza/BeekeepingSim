#include "Public/StorageBoxComponent.h"

#include "Public/BeekeeperHotbarComponent.h"
#include "Public/ItemInstance.h"

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
		UE_LOG(LogTemp, Warning, TEXT("MoveHotbarItemToStorage"));
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
