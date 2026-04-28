#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Public/StorageSlotDragDropTypes.h"
#include "StorageSlotDragDropOperation.generated.h"

class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UItemInstance;
class UItemVisualWidget;

UENUM(BlueprintType)
enum class EItemSlotDragMode : uint8
{
	FullStack,
	PartialStack
};

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UStorageSlotDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Storage Drag Drop")
	void InitializeMoveQuantity();

	UFUNCTION(BlueprintCallable, Category = "Storage Drag Drop")
	void AdjustMoveQuantity(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Storage Drag Drop")
	void SetMoveQuantityClamped(int32 NewQuantity);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	EItemSlotDragMode DragMode = EItemSlotDragMode::FullStack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	int32 MoveQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	int32 MaxMoveQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
	TObjectPtr<UItemVisualWidget> DragVisualWidget = nullptr;
};
