#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Public/StorageSlotDragDropTypes.h"
#include "ItemSlotDragDropLibrary.generated.h"

class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UStorageSlotDragDropOperation;

UCLASS()
class BEEKEEPINGSIM_API UItemSlotDragDropLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	static bool HandleItemSlotDrop(
		UStorageSlotDragDropOperation* Operation,
		EStorageSlotContainerType TargetType,
		int32 TargetIndex,
		UBeekeeperHotbarComponent* TargetHotbarComponent,
		UStorageBoxComponent* TargetStorageComponent);
};
