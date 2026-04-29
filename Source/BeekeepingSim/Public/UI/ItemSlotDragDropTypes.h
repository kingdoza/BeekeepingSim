#pragma once

#include "CoreMinimal.h"
#include "ItemSlotDragDropTypes.generated.h"

UENUM(BlueprintType)
enum class EItemSlotContainerType : uint8
{
	None UMETA(DisplayName = "None"),
	Hotbar UMETA(DisplayName = "Hotbar"),
	Storage UMETA(DisplayName = "Storage")
};

USTRUCT(BlueprintType)
struct FItemSlotMoveResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Item Slot")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Item Slot")
	int32 RequestedQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item Slot")
	int32 MovedQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item Slot")
	int32 RemainingQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item Slot")
	FText Message;
};
