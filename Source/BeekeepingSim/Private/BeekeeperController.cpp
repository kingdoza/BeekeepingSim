// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BeekeeperController.h"
#include "EnhancedInputSubsystems.h"
#include "BeekeepingSimCameraManager.h"

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