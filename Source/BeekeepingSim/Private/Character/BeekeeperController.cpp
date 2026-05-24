// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BeekeeperController.h"
#include "EnhancedInputSubsystems.h"
#include "BeekeepingSimCameraManager.h"
#include "Character/BeekeeperCharacter.h"
#include "Environment/EnvironmentTimeOfDayActor.h"
#include "Environment/GameTimeOfDayActor.h"
#include "Environment/TimeOfDayProvider.h"
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

	BoundTimeOfDayProviderActor = FindTimeOfDayProviderActor();
	if (!BoundTimeOfDayProviderActor)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("%s could not find ITimeOfDayProvider actor for time-of-day clock widget binding."), *GetName());
		return;
	}

	BoundGameTimeOfDayActor = Cast<AGameTimeOfDayActor>(BoundTimeOfDayProviderActor);
	BoundLegacyEnvironmentActor = Cast<AEnvironmentTimeOfDayActor>(BoundTimeOfDayProviderActor);

	if (BoundGameTimeOfDayActor)
	{
		BoundGameTimeOfDayActor->OnGameTimeOfDayChanged.AddDynamic(this, &ABeekeeperController::HandleGameTimeOfDayChanged);
		HandleGameTimeOfDayChanged(BoundGameTimeOfDayActor->GetCurrentHour24());
		return;
	}

	if (BoundLegacyEnvironmentActor)
	{
		BoundLegacyEnvironmentActor->OnTimeOfDayChanged.AddDynamic(this, &ABeekeeperController::HandleTimeOfDayChanged);
		HandleTimeOfDayChanged(BoundLegacyEnvironmentActor->GetCurrentHour24(), FTimeOfDayVisualState());
	}
}

void ABeekeeperController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundGameTimeOfDayActor)
	{
		BoundGameTimeOfDayActor->OnGameTimeOfDayChanged.RemoveDynamic(this, &ABeekeeperController::HandleGameTimeOfDayChanged);
		BoundGameTimeOfDayActor = nullptr;
	}

	if (BoundLegacyEnvironmentActor)
	{
		BoundLegacyEnvironmentActor->OnTimeOfDayChanged.RemoveDynamic(this, &ABeekeeperController::HandleTimeOfDayChanged);
		BoundLegacyEnvironmentActor = nullptr;
	}

	BoundTimeOfDayProviderActor = nullptr;

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

AActor* ABeekeeperController::FindTimeOfDayProviderActor() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return FindCanonicalTimeProviderActor(World, true);
}

AActor* ABeekeeperController::FindCanonicalTimeProviderActor(UWorld* World, bool bLogIfMultiple) const
{
	AGameTimeOfDayActor* FirstGameTimeActor = nullptr;
	AEnvironmentTimeOfDayActor* FirstLegacyActor = nullptr;
	int32 GameTimeActorCount = 0;
	int32 LegacyActorCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (!IsValid(Candidate) || !Candidate->GetClass()->ImplementsInterface(UTimeOfDayProvider::StaticClass()))
		{
			continue;
		}

		if (AGameTimeOfDayActor* GameTimeActor = Cast<AGameTimeOfDayActor>(Candidate))
		{
			++GameTimeActorCount;
			if (!FirstGameTimeActor)
			{
				FirstGameTimeActor = GameTimeActor;
			}
			continue;
		}

		if (AEnvironmentTimeOfDayActor* LegacyActor = Cast<AEnvironmentTimeOfDayActor>(Candidate))
		{
			++LegacyActorCount;
			if (!FirstLegacyActor)
			{
				FirstLegacyActor = LegacyActor;
			}
		}
	}

	if (bLogIfMultiple)
	{
		if (GameTimeActorCount > 1)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple AGameTimeOfDayActor instances detected for clock widget. Using first discovered actor: %s"), *GetNameSafe(FirstGameTimeActor));
		}
		if (GameTimeActorCount == 0 && LegacyActorCount > 1)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple legacy AEnvironmentTimeOfDayActor providers detected for clock widget. Using first discovered actor: %s"), *GetNameSafe(FirstLegacyActor));
		}
	}

	return FirstGameTimeActor ? static_cast<AActor*>(FirstGameTimeActor) : static_cast<AActor*>(FirstLegacyActor);
}

void ABeekeeperController::HandleTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)
{
	(void)VisualState;

	if (TimeOfDayClockWidget)
	{
		TimeOfDayClockWidget->SetHour24(Hour24);
	}
}

void ABeekeeperController::HandleGameTimeOfDayChanged(float Hour24)
{
	if (TimeOfDayClockWidget)
	{
		TimeOfDayClockWidget->SetHour24(Hour24);
	}
}
