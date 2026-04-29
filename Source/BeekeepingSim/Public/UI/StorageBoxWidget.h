#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StorageBoxWidget.generated.h"

class UBeekeeperHotbarComponent;
class UStorageBoxComponent;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UStorageBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Storage")
	void InitializeStorageWidget(UStorageBoxComponent* InStorageComponent, UBeekeeperHotbarComponent* InHotbarComponent);

	UFUNCTION(BlueprintPure, Category = "Storage")
	UStorageBoxComponent* GetStorageComponent() const { return StorageComponent; }

	UFUNCTION(BlueprintPure, Category = "Storage")
	UBeekeeperHotbarComponent* GetHotbarComponent() const { return HotbarComponent; }

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool MoveHotbarItemToStorage(int32 HotbarIndex, int32 StorageIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool MoveStorageItemToHotbar(int32 StorageIndex, int32 HotbarIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool SwapStorageSlots(int32 FromStorageIndex, int32 ToStorageIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool SwapHotbarAndStorage(int32 HotbarIndex, int32 StorageIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Storage")
	void OnStorageWidgetInitialized();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Storage")
	TObjectPtr<UStorageBoxComponent> StorageComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Storage")
	TObjectPtr<UBeekeeperHotbarComponent> HotbarComponent;
};
