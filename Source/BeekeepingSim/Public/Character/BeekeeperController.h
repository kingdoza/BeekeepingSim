// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BeekeeperController.generated.h"

class UInputMappingContext;
class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UItemSlotDragDropOperation;
class UTimeOfDayClockWidget;
class AEnvironmentTimeOfDayActor;
struct FTimeOfDayVisualState;

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
	void SetActiveItemSlotDragOperation(UItemSlotDragDropOperation* InOperation);

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	void ClearActiveItemSlotDragOperation();

	UFUNCTION(BlueprintPure, Category = "Item Slot|Drag Drop")
	UItemSlotDragDropOperation* GetActiveItemSlotDragOperation() const { return ActiveItemSlotDragOperation; }

	UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
	bool AdjustActiveItemSlotDragQuantity(float WheelDelta);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input Mappings")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UStorageBoxComponent> ActiveStorageComponent;

	UPROPERTY(Transient)
	TObjectPtr<UItemSlotDragDropOperation> ActiveItemSlotDragOperation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Time Of Day")
	TSubclassOf<UTimeOfDayClockWidget> TimeOfDayClockWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UTimeOfDayClockWidget> TimeOfDayClockWidget;

	UPROPERTY(Transient)
	TObjectPtr<AEnvironmentTimeOfDayActor> BoundTimeOfDayActor;
	
	virtual void SetupInputComponent() override;

private:
	AEnvironmentTimeOfDayActor* FindTimeOfDayActor() const;

	UFUNCTION()
	void HandleTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState);
};
