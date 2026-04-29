#include "UI/ItemVisualWidget.h"

#include "Engine/Texture2D.h"
#include "Inventory/ItemInstance.h"

void UItemVisualWidget::SetItemVisualData(UItemInstance* InItemInstance, const int32 InQuantityOverride)
{
	ItemInstance = InItemInstance;
	QuantityOverride = InQuantityOverride;
	OnItemVisualDataChanged();
}

void UItemVisualWidget::ClearItemVisualData()
{
	ItemInstance = nullptr;
	QuantityOverride = INDEX_NONE;
	OnItemVisualDataChanged();
}

UTexture2D* UItemVisualWidget::GetItemIcon() const
{
	return ItemInstance ? ItemInstance->GetIcon() : nullptr;
}

FText UItemVisualWidget::GetItemDisplayName() const
{
	return ItemInstance ? ItemInstance->GetDisplayName() : FText::GetEmpty();
}

int32 UItemVisualWidget::GetDisplayStackCount() const
{
	if (QuantityOverride != INDEX_NONE)
	{
		return FMath::Max(0, QuantityOverride);
	}

	return ItemInstance ? ItemInstance->GetStackCount() : 0;
}

bool UItemVisualWidget::HasItem() const
{
	return ItemInstance != nullptr;
}

bool UItemVisualWidget::HasDurability() const
{
	return ItemInstance && ItemInstance->HasDurability();
}

float UItemVisualWidget::GetDurabilityRatio() const
{
	return ItemInstance ? ItemInstance->GetDurabilityRatio() : 0.0f;
}
