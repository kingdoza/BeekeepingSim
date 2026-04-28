#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Public/StorageSlotDragDropOperation.h"
#include "Public/StorageSlotDragDropTypes.h"
#include "ItemSlotWidget.generated.h"

class ABeekeeperController;
class UBeekeeperHotbarComponent;
class UItemInstance;
class UItemVisualWidget;
class UStorageBoxComponent;
class UStorageSlotDragDropOperation;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Slot")
	void InitializeSlotContext(
		EStorageSlotContainerType InContainerType,
		int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Item Slot")
	void RefreshFromData();

	UFUNCTION(BlueprintCallable, Category = "Item Slot")
	void RefreshVisual();

	UFUNCTION(BlueprintCallable, Category = "Item Slot")
	void SetDragSourceVisualHidden(bool bHidden);

	UFUNCTION(BlueprintCallable, Category = "Item Slot")
	void ClearDragState();

	UFUNCTION(BlueprintPure, Category = "Item Slot|Partial Drag Preview")
	bool IsPartialDragPreviewActive() const;

	UFUNCTION(BlueprintPure, Category = "Item Slot|Partial Drag Preview")
	bool ShouldHideItemVisualForPartialDragPreview() const;

	UFUNCTION(BlueprintPure, Category = "Item Slot|Partial Drag Preview")
	int32 GetPartialDragPreviewDisplayStackCount() const;

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Partial Drag Preview")
	void SetPartialDragPreviewState(int32 InOriginalStackCount, int32 InMoveQuantity);

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Partial Drag Preview")
	void UpdatePartialDragPreviewMoveQuantity(int32 InMoveQuantity);

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Partial Drag Preview")
	void ClearPartialDragPreviewState();

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Partial Drag Preview")
	void RefreshPartialDragPreviewFromOperation(UStorageSlotDragDropOperation* Operation);

	// Legacy wrappers for Blueprint compatibility during migration.
	UFUNCTION(BlueprintPure, Category = "Item Slot")
	bool ShouldHideItemVisualForCurrentDrag() const;

	// Legacy wrapper for Blueprint compatibility during migration.
	UFUNCTION(BlueprintPure, Category = "Item Slot")
	int32 GetDragPreviewDisplayStackCount() const;

	// Legacy wrapper for Blueprint compatibility during migration.
	UFUNCTION(BlueprintCallable, Category = "Item Slot")
	void RefreshDragPreviewFromOperation(UStorageSlotDragDropOperation* Operation);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION()
	void HandleDragOperationDropped(UDragDropOperation* InOperation);

	UFUNCTION()
	void HandleDragOperationCancelled(UDragDropOperation* InOperation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Item Slot")
	void OnSlotVisualStateChanged();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
	EStorageSlotContainerType ContainerType = EStorageSlotContainerType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot")
	TObjectPtr<UItemInstance> ItemInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
	bool bIsSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
	bool bIsActivated = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot")
	bool bIsDragSource = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot|Partial Drag Preview")
	int32 PartialDragPreviewOriginalStackCount = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot|Partial Drag Preview")
	int32 PartialDragPreviewMoveQuantity = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot|Partial Drag Preview")
	bool bPartialDragPreviewActive = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Slot")
	EItemSlotDragMode ActiveDragMode = EItemSlotDragMode::FullStack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
	TSubclassOf<UItemVisualWidget> DragVisualWidgetClass;

private:
	ABeekeeperController* ResolveBeekeeperController() const;
	UBeekeeperHotbarComponent* ResolveHotbarComponentForSlot() const;
	UStorageBoxComponent* ResolveStorageComponentForSlot() const;
	bool TryQuickMove();

	UPROPERTY(Transient)
	FKey PendingDragButton = EKeys::Invalid;
};
