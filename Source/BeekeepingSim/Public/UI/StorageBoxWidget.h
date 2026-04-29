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
	void InitializeStorageWidget(UStorageBoxComponent* InStorageComponent, UBeekeeperHotbarComponent* InHotbarComponent);

	UFUNCTION(BlueprintPure, Category = "Storage")
	UStorageBoxComponent* GetStorageComponent() const { return StorageComponent; }

	UFUNCTION(BlueprintPure, Category = "Storage")
	UBeekeeperHotbarComponent* GetHotbarComponent() const { return HotbarComponent; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Storage")
	void OnStorageWidgetInitialized();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Storage")
	TObjectPtr<UStorageBoxComponent> StorageComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Storage")
	TObjectPtr<UBeekeeperHotbarComponent> HotbarComponent;
};
