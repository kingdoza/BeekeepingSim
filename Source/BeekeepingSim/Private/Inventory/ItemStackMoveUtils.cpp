#include "Inventory/ItemStackMoveUtils.h"

#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"

namespace ItemStackMoveUtils
{
int32 ResolveMaxStack(const UItemDefinition* Definition)
{
	return Definition ? FMath::Max(1, Definition->MaxStack) : 1;
}

int32 ClampQuantityToAvailable(const int32 RequestedQuantity, const int32 AvailableQuantity)
{
	return FMath::Min(FMath::Max(0, RequestedQuantity), FMath::Max(0, AvailableQuantity));
}

bool HasMatchingDefinition(const UItemInstance* ItemInstance, const UItemDefinition* Definition)
{
	return ItemInstance && ItemInstance->GetDefinition() == Definition;
}

int32 GetAvailableStackSpace(const UItemInstance* ItemInstance, const int32 MaxStack)
{
	if (!ItemInstance)
	{
		return 0;
	}

	return FMath::Max(0, MaxStack - ItemInstance->GetStackCount());
}

int32 MergeIntoStack(UItemInstance* TargetItem, const int32 RequestedQuantity, const int32 MaxStack)
{
	if (!TargetItem || RequestedQuantity <= 0)
	{
		return 0;
	}

	const int32 AddCount = ClampQuantityToAvailable(RequestedQuantity, GetAvailableStackSpace(TargetItem, MaxStack));
	if (AddCount > 0)
	{
		TargetItem->SetStackCount(TargetItem->GetStackCount() + AddCount);
	}

	return AddCount;
}

UItemInstance* CreateItemInstance(UObject* Outer, UItemDefinition* Definition, const int32 StackCount)
{
	if (!Outer || !Definition || StackCount <= 0)
	{
		return nullptr;
	}

	UItemInstance* NewItemInstance = NewObject<UItemInstance>(Outer);
	if (!NewItemInstance)
	{
		return nullptr;
	}

	NewItemInstance->InitializeFromDefinition(Definition, StackCount);
	return NewItemInstance;
}

void UpdateRemainingQuantity(FItemSlotMoveResult& Result)
{
	Result.RemainingQuantity = FMath::Max(0, Result.RequestedQuantity - Result.MovedQuantity);
}
}
