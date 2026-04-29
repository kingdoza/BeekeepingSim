#include "UI/StorageBoxWidget.h"

#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/StorageBoxComponent.h"

void UStorageBoxWidget::InitializeStorageWidget(UStorageBoxComponent* InStorageComponent, UBeekeeperHotbarComponent* InHotbarComponent)
{
	StorageComponent = InStorageComponent;
	HotbarComponent = InHotbarComponent;
	OnStorageWidgetInitialized();
}
