// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BeekeeperController.h"
#include "EnhancedInputSubsystems.h"
#include "BeekeepingSimCameraManager.h"
#include "Public/StorageBoxComponent.h"
#include "Public/StorageSlotDragDropOperation.h"

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

	const int32 Delta = WheelDelta > 0.0f ? 1 : -1;
	ActiveItemSlotDragOperation->AdjustMoveQuantity(Delta);
	return true;
}
