#include "Inventory/ItemDefinition.h"

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	if (ItemId.IsValid())
	{
		return ItemId;
	}

	return Super::GetPrimaryAssetId();
}
