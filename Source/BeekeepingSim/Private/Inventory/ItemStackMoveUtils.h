#pragma once

#include "CoreMinimal.h"
#include "UI/StorageSlotDragDropTypes.h"

class UItemDefinition;
class UItemInstance;

namespace ItemStackMoveUtils
{
int32 ResolveMaxStack(const UItemDefinition* Definition);
int32 ClampQuantityToAvailable(const int32 RequestedQuantity, const int32 AvailableQuantity);
bool HasMatchingDefinition(const UItemInstance* ItemInstance, const UItemDefinition* Definition);
int32 GetAvailableStackSpace(const UItemInstance* ItemInstance, const int32 MaxStack);
int32 MergeIntoStack(UItemInstance* TargetItem, const int32 RequestedQuantity, const int32 MaxStack);
UItemInstance* CreateItemInstance(UObject* Outer, UItemDefinition* Definition, const int32 StackCount);
void UpdateRemainingQuantity(FItemSlotMoveResult& Result);
}
