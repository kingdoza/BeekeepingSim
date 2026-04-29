#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/ItemSlotDragDropTypes.h"
#include "ItemSlotDragDropLibrary.generated.h"

class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UItemSlotDragDropOperation;

UCLASS()
class BEEKEEPINGSIM_API UItemSlotDragDropLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	static bool HandleItemSlotDrop(
		UItemSlotDragDropOperation* Operation,
		EItemSlotContainerType TargetType,
		int32 TargetIndex,
		UBeekeeperHotbarComponent* TargetHotbarComponent,
		UStorageBoxComponent* TargetStorageComponent);
};
