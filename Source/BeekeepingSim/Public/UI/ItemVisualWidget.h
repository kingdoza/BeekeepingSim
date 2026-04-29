#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemVisualWidget.generated.h"

class UItemInstance;
class UTexture2D;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UItemVisualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Visual")
	void SetItemVisualData(UItemInstance* InItemInstance, int32 QuantityOverride = -1);

	UFUNCTION(BlueprintCallable, Category = "Item Visual")
	void ClearItemVisualData();

	UFUNCTION(BlueprintPure, Category = "Item Visual")
	UTexture2D* GetItemIcon() const;

	UFUNCTION(BlueprintPure, Category = "Item Visual")
	FText GetItemDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Item Visual")
	int32 GetDisplayStackCount() const;

	UFUNCTION(BlueprintPure, Category = "Item Visual")
	bool HasItem() const;

	UFUNCTION(BlueprintPure, Category = "Item Visual|Durability")
	bool HasDurability() const;

	UFUNCTION(BlueprintPure, Category = "Item Visual|Durability")
	float GetDurabilityRatio() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Item Visual")
	void OnItemVisualDataChanged();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Visual")
	TObjectPtr<UItemInstance> ItemInstance;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item Visual")
	int32 QuantityOverride = INDEX_NONE;
};
