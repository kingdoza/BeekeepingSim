#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDragVisualWidget.generated.h"

class UItemInstance;
class UTexture2D;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UItemDragVisualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Drag Visual")
	void InitializeDragVisual(UItemInstance* InItemInstance);

	UFUNCTION(BlueprintPure, Category = "Item Drag Visual")
	UItemInstance* GetItemInstance() const { return ItemInstance; }

	UFUNCTION(BlueprintPure, Category = "Item Drag Visual")
	UTexture2D* GetItemIcon() const;

	UFUNCTION(BlueprintPure, Category = "Item Drag Visual")
	FText GetItemDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Item Drag Visual")
	int32 GetItemStackCount() const;

	UFUNCTION(BlueprintPure, Category = "Item Drag Visual")
	bool HasItem() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Item Drag Visual")
	void OnDragVisualInitialized();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Drag Visual")
	TObjectPtr<UItemInstance> ItemInstance;
};
