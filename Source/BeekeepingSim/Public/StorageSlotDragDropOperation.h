#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Public/StorageSlotDragDropTypes.h"
#include "StorageSlotDragDropOperation.generated.h"

class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UItemInstance;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UStorageSlotDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	EStorageSlotContainerType SourceType = EStorageSlotContainerType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	int32 SourceIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	TObjectPtr<UBeekeeperHotbarComponent> SourceHotbarComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	TObjectPtr<UStorageBoxComponent> SourceStorageComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	TObjectPtr<UItemInstance> ItemInstance = nullptr;
};
