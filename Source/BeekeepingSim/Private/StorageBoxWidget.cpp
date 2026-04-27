#include "Public/StorageBoxWidget.h"

#include "Public/BeekeeperHotbarComponent.h"
#include "Public/StorageBoxComponent.h"

void UStorageBoxWidget::InitializeStorageWidget(UStorageBoxComponent* InStorageComponent, UBeekeeperHotbarComponent* InHotbarComponent)
{
	StorageComponent = InStorageComponent;
	HotbarComponent = InHotbarComponent;
	OnStorageWidgetInitialized();
}

bool UStorageBoxWidget::MoveHotbarItemToStorage(const int32 HotbarIndex, const int32 StorageIndex)
{
	return StorageComponent && HotbarComponent && StorageComponent->MoveHotbarItemToStorage(HotbarComponent, HotbarIndex, StorageIndex);
}

bool UStorageBoxWidget::MoveStorageItemToHotbar(const int32 StorageIndex, const int32 HotbarIndex)
{
	return StorageComponent && HotbarComponent && StorageComponent->MoveStorageItemToHotbar(HotbarComponent, StorageIndex, HotbarIndex);
}

bool UStorageBoxWidget::SwapStorageSlots(const int32 FromStorageIndex, const int32 ToStorageIndex)
{
	return StorageComponent && StorageComponent->SwapStorageSlots(FromStorageIndex, ToStorageIndex);
}

bool UStorageBoxWidget::SwapHotbarAndStorage(const int32 HotbarIndex, const int32 StorageIndex)
{
	return StorageComponent && HotbarComponent && StorageComponent->SwapHotbarAndStorage(HotbarComponent, HotbarIndex, StorageIndex);
}
