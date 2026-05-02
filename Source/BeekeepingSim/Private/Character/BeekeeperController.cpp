// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BeekeeperController.h"
#include "EnhancedInputSubsystems.h"
#include "BeekeepingSimCameraManager.h"
#include "Character/BeekeeperCharacter.h"
#include "Environment/EnvironmentTimeOfDayActor.h"
#include "Environment/TimeOfDayTypes.h"
#include "EngineUtils.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/StorageBoxComponent.h"
#include "UI/ItemSlotDragDropOperation.h"
#include "UI/TimeOfDayClockWidget.h"

ABeekeeperController::ABeekeeperController()
{
	PlayerCameraManagerClass = ABeekeepingSimCameraManager::StaticClass();
}

void ABeekeeperController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (TimeOfDayClockWidgetClass)
	{
		TimeOfDayClockWidget = CreateWidget<UTimeOfDayClockWidget>(this, TimeOfDayClockWidgetClass);
		if (TimeOfDayClockWidget)
		{
			TimeOfDayClockWidget->AddToViewport();
		}
	}

	BoundTimeOfDayActor = FindTimeOfDayActor();
	if (!BoundTimeOfDayActor)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("%s could not find AEnvironmentTimeOfDayActor for time-of-day clock widget binding."), *GetName());
		return;
	}

	BoundTimeOfDayActor->OnTimeOfDayChanged.AddDynamic(this, &ABeekeeperController::HandleTimeOfDayChanged);
	HandleTimeOfDayChanged(BoundTimeOfDayActor->GetCurrentHour24(), FTimeOfDayVisualState());
}

void ABeekeeperController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundTimeOfDayActor)
	{
		BoundTimeOfDayActor->OnTimeOfDayChanged.RemoveDynamic(this, &ABeekeeperController::HandleTimeOfDayChanged);
		BoundTimeOfDayActor = nullptr;
	}

	if (TimeOfDayClockWidget)
	{
		TimeOfDayClockWidget->RemoveFromParent();
		TimeOfDayClockWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
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

void ABeekeeperController::SetActiveItemSlotDragOperation(UItemSlotDragDropOperation* InOperation)
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

AEnvironmentTimeOfDayActor* ABeekeeperController::FindTimeOfDayActor() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AEnvironmentTimeOfDayActor* FoundActor = nullptr;
	for (TActorIterator<AEnvironmentTimeOfDayActor> It(World); It; ++It)
	{
		if (!FoundActor)
		{
			FoundActor = *It;
		}
		else
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple AEnvironmentTimeOfDayActor instances detected for clock widget. Using first discovered actor: %s"), *GetNameSafe(FoundActor));
			break;
		}
	}

	return FoundActor;
}

void ABeekeeperController::HandleTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)
{
	(void)VisualState;

	if (TimeOfDayClockWidget)
	{
		TimeOfDayClockWidget->SetHour24(Hour24);
	}
}
