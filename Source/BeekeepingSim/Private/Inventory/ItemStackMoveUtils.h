#pragma once

#include "CoreMinimal.h"
#include "UI/ItemSlotDragDropTypes.h"

class UItemDefinition;
class UItemInstance;

namespace ItemStackMoveUtils
{
int32 ResolveMaxStack(const UItemDefinition* Definition);
int32 ClampQuantityToAvailable(const int32 RequestedQuantity, const int32 AvailableQuantity);
bool HasMatchingDefinition(const UItemInstance* ItemInstance, const UItemDefinition* Definition);
bool HasCompatibleStackState(const UItemInstance* ExistingItem, const UItemDefinition* Definition, bool bHasDurabilityOverride, float DurabilityOverride);
bool CanMergeItemStacks(const UItemInstance* TargetItem, const UItemInstance* SourceItem);
int32 GetAvailableStackSpace(const UItemInstance* ItemInstance, const int32 MaxStack);
int32 MergeIntoStack(UItemInstance* TargetItem, const int32 RequestedQuantity, const int32 MaxStack);
UItemInstance* CreateItemInstance(UObject* Outer, UItemDefinition* Definition, const int32 StackCount);
UItemInstance* CreateItemInstance(UObject* Outer, UItemDefinition* Definition, const int32 StackCount, bool bHasDurabilityOverride, float DurabilityOverride);
void UpdateRemainingQuantity(FItemSlotMoveResult& Result);
}
