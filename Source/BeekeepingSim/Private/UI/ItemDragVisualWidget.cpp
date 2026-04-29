#include "UI/ItemDragVisualWidget.h"

#include "Engine/Texture2D.h"
#include "Inventory/ItemInstance.h"

void UItemDragVisualWidget::InitializeDragVisual(UItemInstance* InItemInstance)
{
	ItemInstance = InItemInstance;
	OnDragVisualInitialized();
}

UTexture2D* UItemDragVisualWidget::GetItemIcon() const
{
	return ItemInstance ? ItemInstance->GetIcon() : nullptr;
}

FText UItemDragVisualWidget::GetItemDisplayName() const
{
	return ItemInstance ? ItemInstance->GetDisplayName() : FText::GetEmpty();
}

int32 UItemDragVisualWidget::GetItemStackCount() const
{
	return ItemInstance ? ItemInstance->GetStackCount() : 0;
}

bool UItemDragVisualWidget::HasItem() const
{
	return ItemInstance != nullptr;
}
