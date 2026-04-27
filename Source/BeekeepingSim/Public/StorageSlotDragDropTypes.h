#pragma once

#include "CoreMinimal.h"
#include "StorageSlotDragDropTypes.generated.h"

UENUM(BlueprintType)
enum class EStorageSlotContainerType : uint8
{
	None UMETA(DisplayName = "None"),
	Hotbar UMETA(DisplayName = "Hotbar"),
	Storage UMETA(DisplayName = "Storage")
};
