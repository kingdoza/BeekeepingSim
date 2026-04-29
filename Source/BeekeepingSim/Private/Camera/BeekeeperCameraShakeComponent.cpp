// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/BeekeeperCameraShakeComponent.h"

#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"
#include "Character/BeekeeperMovementComponent.h"

UBeekeeperCameraShakeComponent::UBeekeeperCameraShakeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBeekeeperCameraShakeComponent::SuppressNextLandingShake()
{
	bSuppressNextLandingShake = true;
}

void UBeekeeperCameraShakeComponent::StopAllCameraShakes()
{
	APlayerController* LocalPlayerController = GetLocalPlayerController();
	if (!LocalPlayerController || !LocalPlayerController->PlayerCameraManager)
	{
		StopCurrentShake();
		return;
	}

	LocalPlayerController->PlayerCameraManager->StopAllCameraShakes(true);
	CurrentPlayerCameraManager = nullptr;
	CurrentShakeClass = nullptr;
	bHasAppliedMoveState = false;
}

void UBeekeeperCameraShakeComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABeekeeperCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComponent = Cast<UBeekeeperMovementComponent>(OwnerCharacter->GetCharacterMovement());
	}

	bWasFalling = IsOwnerFalling();

	if (ShouldDisableTickForNonLocal())
	{
		SetComponentTickInterval(0.25f);
		return;
	}

	SetComponentTickInterval(0.0f);

	if (!bWasFalling)
	{
		ApplyMoveState(CalculateMoveState());
	}
}

void UBeekeeperCameraShakeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCurrentShake();
	Super::EndPlay(EndPlayReason);
}

void UBeekeeperCameraShakeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter && OwnerCharacter->IsFocusInteractionInputLocked())
	{
		StopAllCameraShakes();
		return;
	}

	if (ShouldDisableTickForNonLocal())
	{
		StopCurrentShake();
		SetComponentTickInterval(0.25f);
		return;
	}

	SetComponentTickInterval(0.0f);

	const bool bIsFalling = IsOwnerFalling();
	if (bIsFalling)
	{
		StopMoveShake();
		bWasFalling = true;
		return;
	}

	const bool bHasLandedThisFrame = bWasFalling && !bIsFalling;
	if (bHasLandedThisFrame)
	{
		if (bSuppressNextLandingShake)
		{
			bSuppressNextLandingShake = false;
		}
		else
		{
			PlayLandingShake();
		}
	}

	const EBeekeeperMoveState NewState = CalculateMoveState();
	if (!bHasAppliedMoveState || NewState != AppliedMoveState)
	{
		ApplyMoveState(NewState);
	}

	bWasFalling = bIsFalling;
}

EBeekeeperMoveState UBeekeeperCameraShakeComponent::CalculateMoveState() const
{
	if (!OwnerCharacter)
	{
		return EBeekeeperMoveState::Idle;
	}

	const FVector Velocity = OwnerCharacter->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
	const float ThresholdSq = FMath::Square(IdleSpeedThreshold);
	if (HorizontalVelocity.SizeSquared() <= ThresholdSq)
	{
		return EBeekeeperMoveState::Idle;
	}

	if (OwnerMovementComponent && OwnerMovementComponent->IsSprinting())
	{
		return EBeekeeperMoveState::Sprint;
	}

	return EBeekeeperMoveState::Walk;
}

void UBeekeeperCameraShakeComponent::ApplyMoveState(EBeekeeperMoveState NewState)
{
	StopMoveShake();

	APlayerController* LocalPlayerController = GetLocalPlayerController();
	if (!LocalPlayerController || !LocalPlayerController->PlayerCameraManager)
	{
		return;
	}

	TSubclassOf<UCameraShakeBase> ShakeClass = GetShakeClassForState(NewState);
	AppliedMoveState = NewState;
	bHasAppliedMoveState = true;
	if (!ShakeClass)
	{
		return;
	}

	CurrentPlayerCameraManager = LocalPlayerController->PlayerCameraManager;
	CurrentPlayerCameraManager->StartCameraShake(ShakeClass);
	CurrentShakeClass = ShakeClass;
}

void UBeekeeperCameraShakeComponent::StopCurrentShake()
{
	StopMoveShake();
}

void UBeekeeperCameraShakeComponent::StopMoveShake()
{
	if (CurrentShakeClass && CurrentPlayerCameraManager)
	{
		CurrentPlayerCameraManager->StopAllInstancesOfCameraShake(CurrentShakeClass, true);
	}

	CurrentPlayerCameraManager = nullptr;
	CurrentShakeClass = nullptr;
	bHasAppliedMoveState = false;
}

void UBeekeeperCameraShakeComponent::PlayLandingShake()
{
	APlayerController* LocalPlayerController = GetLocalPlayerController();
	if (!LocalPlayerController || !LocalPlayerController->PlayerCameraManager || !LandingCameraShakeClass)
	{
		return;
	}

	LocalPlayerController->PlayerCameraManager->StartCameraShake(LandingCameraShakeClass);
}

TSubclassOf<UCameraShakeBase> UBeekeeperCameraShakeComponent::GetShakeClassForState(EBeekeeperMoveState State) const
{
	switch (State)
	{
	case EBeekeeperMoveState::Idle:
		return IdleCameraShakeClass;
	case EBeekeeperMoveState::Walk:
		return WalkCameraShakeClass;
	case EBeekeeperMoveState::Sprint:
		return SprintCameraShakeClass;
	default:
		return nullptr;
	}
}

APlayerController* UBeekeeperCameraShakeComponent::GetLocalPlayerController() const
{
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return nullptr;
	}

	return PlayerController;
}

bool UBeekeeperCameraShakeComponent::ShouldDisableTickForNonLocal() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	return OwnerCharacter->GetController() != nullptr && !OwnerCharacter->IsLocallyControlled();
}

bool UBeekeeperCameraShakeComponent::IsOwnerFalling() const
{
	if (!OwnerMovementComponent)
	{
		return false;
	}

	return OwnerMovementComponent->IsFalling();
}
