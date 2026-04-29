// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BeekeeperController.h"
#include "EnhancedInputSubsystems.h"
#include "BeekeepingSimCameraManager.h"
#include "Character/BeekeeperCharacter.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/StorageBoxComponent.h"
#include "UI/StorageSlotDragDropOperation.h"

ABeekeeperController::ABeekeeperController()
{
	PlayerCameraManagerClass = ABeekeepingSimCameraManager::StaticClass();
}

void ABeekeeperController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void ABeekeeperController::SetActiveStorageComponent(UStorageBoxComponent* InStorageComponent)
{
	ActiveStorageComponent = InStorageComponent;
}

void ABeekeeperController::ClearActiveStorageComponent()
{
	ActiveStorageComponent = nullptr;
}

UBeekeeperHotbarComponent* ABeekeeperController::GetPlayerHotbarComponent() const
{
	const ABeekeeperCharacter* BeekeeperCharacter = Cast<ABeekeeperCharacter>(GetPawn());
	return BeekeeperCharacter ? BeekeeperCharacter->GetBeekeeperHotbar() : nullptr;
}

void ABeekeeperController::SetActiveItemSlotDragOperation(UStorageSlotDragDropOperation* InOperation)
{
	ActiveItemSlotDragOperation = InOperation;
}

void ABeekeeperController::ClearActiveItemSlotDragOperation()
{
	ActiveItemSlotDragOperation = nullptr;
}

bool ABeekeeperController::AdjustActiveItemSlotDragQuantity(float WheelDelta)
{
	if (!ActiveItemSlotDragOperation || FMath::IsNearlyZero(WheelDelta))
	{
		return false;
	}

	if (ActiveItemSlotDragOperation->DragMode != EItemSlotDragMode::PartialStack)
	{
		return false;
	}

	const int32 PreviousQuantity = ActiveItemSlotDragOperation->MoveQuantity;
	const int32 Delta = WheelDelta > 0.0f ? -1 : 1;
	ActiveItemSlotDragOperation->AdjustMoveQuantity(Delta);
	return PreviousQuantity != ActiveItemSlotDragOperation->MoveQuantity;
}
