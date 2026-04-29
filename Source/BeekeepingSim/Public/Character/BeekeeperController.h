// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BeekeeperController.generated.h"

class UInputMappingContext;
class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UStorageSlotDragDropOperation;

UCLASS()
class BEEKEEPINGSIM_API ABeekeeperController : public APlayerController
{
	GENERATED_BODY()
public:
	ABeekeeperController();

	UFUNCTION(BlueprintCallable, Category = "Storage")
	void SetActiveStorageComponent(UStorageBoxComponent* InStorageComponent);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	void ClearActiveStorageComponent();

	UFUNCTION(BlueprintPure, Category = "Storage")
	UStorageBoxComponent* GetActiveStorageComponent() const { return ActiveStorageComponent; }

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	UBeekeeperHotbarComponent* GetPlayerHotbarComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	void SetActiveItemSlotDragOperation(UStorageSlotDragDropOperation* InOperation);

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	void ClearActiveItemSlotDragOperation();

	UFUNCTION(BlueprintPure, Category = "Item Slot|Drag Drop")
	UStorageSlotDragDropOperation* GetActiveItemSlotDragOperation() const { return ActiveItemSlotDragOperation; }

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	bool AdjustActiveItemSlotDragQuantity(float WheelDelta);
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Input Mappings")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UStorageBoxComponent> ActiveStorageComponent;

	UPROPERTY(Transient)
	TObjectPtr<UStorageSlotDragDropOperation> ActiveItemSlotDragOperation;
	
	virtual void SetupInputComponent() override;
};
