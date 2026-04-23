// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BeekeeperCharacter.h"
#include "Public/BeekeeperCameraShakeComponent.h"
#include "Public/BeekeeperFocusComponent.h"
#include "Public/BeekeeperMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"


ABeekeeperCharacter::ABeekeeperCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBeekeeperMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	
	BeekeeperMovement = Cast<UBeekeeperMovementComponent>(GetCharacterMovement());
	BeekeeperCameraShake = CreateDefaultSubobject<UBeekeeperCameraShakeComponent>(TEXT("BeekeeperCameraShake"));
	BeekeeperFocus = CreateDefaultSubobject<UBeekeeperFocusComponent>(TEXT("BeekeeperFocus"));
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("head"));
	FirstPersonCamera->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->CastShadow = false;
	GetMesh()->bVisibleInReflectionCaptures = false;
	GetMesh()->bVisibleInRayTracing = false;
	GetMesh()->bHiddenInGame = true;
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetMesh()->MarkRenderStateDirty();
	
	GetCapsuleComponent()->SetCapsuleSize(34, 96);
	
	GetCharacterMovement()->BrakingDecelerationFalling = 1500;
	GetCharacterMovement()->AirControl = 0.5f;
}

// Called when the game starts or when spawned
void ABeekeeperCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FirstPersonCamera)
	{
		DefaultFirstPersonCameraRelativeLocation = FirstPersonCamera->GetRelativeLocation();
		DefaultFirstPersonCameraRelativeRotation = FirstPersonCamera->GetRelativeRotation();
	}
}

// Called every frame
void ABeekeeperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABeekeeperCharacter::MoveInput(const FInputActionValue& Value)
{
	if (bIsFocusInteractionInputLocked)
	{
		return;
	}

	FVector2D MovementValue = Value.Get<FVector2D>();
	FVector2D ScaledMovementValue = MovementValue * MoveSpeedScale;
	DoMove(ScaledMovementValue.X, ScaledMovementValue.Y);
}

void ABeekeeperCharacter::LookInput(const FInputActionValue& Value)
{
	if (bIsFocusInteractionInputLocked)
	{
		return;
	}

	FVector2D LookAxisValue = Value.Get<FVector2D>();
	FVector2D ScaledLookAxisValue = LookAxisValue * LookSpeedScale;
	DoLook(ScaledLookAxisValue.X, ScaledLookAxisValue.Y);
}

void ABeekeeperCharacter::DoMove(float Right, float Forward)
{
	if (bIsFocusInteractionInputLocked)
	{
		return;
	}

	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}
	
void ABeekeeperCharacter::DoLook(float Yaw, float Pitch)
{
	if (bIsFocusInteractionInputLocked)
	{
		return;
	}

	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

// Called to bind functionality to input
void ABeekeeperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABeekeeperCharacter::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABeekeeperCharacter::LookInput);
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABeekeeperCharacter::DoJumpEnd);
		
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::SprintStartInput);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABeekeeperCharacter::SprintReleaseInput);

		if (FocusConfirmAction)
		{
			EnhancedInputComponent->BindAction(FocusConfirmAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::FocusConfirmInput);
		}

		if (FocusCancelAction)
		{
			EnhancedInputComponent->BindAction(FocusCancelAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::FocusCancelInput);
		}
	}
}

// UProfessorMovementComponent* AProfessorCharacter::GetCharacterMovement() const
// {
// 	return ProfessorMovement;
// }

void ABeekeeperCharacter::SprintStartInput()
{
	if (bIsFocusInteractionInputLocked || !BeekeeperMovement)
	{
		return;
	}

	if (bIsSprintToggle)
	{
		BeekeeperMovement->SwitchSprinting();
	}
	else
	{
		BeekeeperMovement->StartSprinting();
	}
}

void ABeekeeperCharacter::SprintReleaseInput()
{
	if (bIsFocusInteractionInputLocked || bIsSprintToggle || !BeekeeperMovement)
	{
		return;
	}

	BeekeeperMovement->StopSprinting();
}

void ABeekeeperCharacter::FocusConfirmInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->ConfirmFocus();
}

void ABeekeeperCharacter::FocusCancelInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->CancelFocus();
}

void ABeekeeperCharacter::DoJumpStart()
{
	if (bIsFocusInteractionInputLocked)
	{
		return;
	}

	Jump();
}

void ABeekeeperCharacter::DoJumpEnd()
{
	if (bIsFocusInteractionInputLocked)
	{
		return;
	}

	StopJumping();
}

void ABeekeeperCharacter::SetFocusInteractionInputLocked(bool bLocked)
{
	bIsFocusInteractionInputLocked = bLocked;
}

void ABeekeeperCharacter::BeginFocusCameraOverride()
{
	if (!FirstPersonCamera || bIsFocusCameraOverrideActive)
	{
		return;
	}

	bStoredUsePawnControlRotation = FirstPersonCamera->bUsePawnControlRotation;
	FirstPersonCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FirstPersonCamera->bUsePawnControlRotation = false;
	bIsFocusCameraOverrideActive = true;
}

void ABeekeeperCharacter::UpdateFocusCameraOverride(const FVector& WorldLocation, const FRotator& WorldRotation)
{
	if (!FirstPersonCamera)
	{
		return;
	}

	FirstPersonCamera->SetWorldLocationAndRotation(WorldLocation, WorldRotation);
}

void ABeekeeperCharacter::EndFocusCameraOverride()
{
	if (!FirstPersonCamera || !bIsFocusCameraOverrideActive)
	{
		return;
	}

	if (GetMesh())
	{
		FirstPersonCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FirstPersonCameraAttachSocketName);
	}

	FirstPersonCamera->SetRelativeLocationAndRotation(DefaultFirstPersonCameraRelativeLocation, DefaultFirstPersonCameraRelativeRotation);
	FirstPersonCamera->bUsePawnControlRotation = bStoredUsePawnControlRotation;
	bIsFocusCameraOverrideActive = false;
}

FTransform ABeekeeperCharacter::GetDefaultFocusCameraWorldTransform() const
{
	if (!FirstPersonCamera)
	{
		return FTransform::Identity;
	}

	if (!GetMesh())
	{
		return FirstPersonCamera->GetComponentTransform();
	}

	const FTransform SocketTransform = GetMesh()->GetSocketTransform(FirstPersonCameraAttachSocketName);
	const FTransform RelativeTransform(DefaultFirstPersonCameraRelativeRotation, DefaultFirstPersonCameraRelativeLocation);
	return RelativeTransform * SocketTransform;
}

void ABeekeeperCharacter::SyncControlRotationTo(const FRotator& NewControlRotation)
{
	if (!GetController())
	{
		return;
	}

	GetController()->SetControlRotation(NewControlRotation);
}

bool ABeekeeperCharacter::MoveToCharacterAnchor(const FTransform& AnchorTransform)
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	const bool bWasFalling = CharacterMovementComponent && CharacterMovementComponent->IsFalling();

	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->StopMovementImmediately();
	}

	SetActorLocationAndRotation(AnchorTransform.GetLocation(), AnchorTransform.GetRotation().Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
	SyncControlRotationTo(AnchorTransform.GetRotation().Rotator());

	if (!CharacterMovementComponent)
	{
		return false;
	}

	FFindFloorResult FloorResult;
	CharacterMovementComponent->FindFloor(AnchorTransform.GetLocation(), FloorResult, false);
	const bool bHasWalkableFloor = FloorResult.IsWalkableFloor();
	CharacterMovementComponent->SetMovementMode(bHasWalkableFloor ? MOVE_Walking : MOVE_Falling);
	return bWasFalling && bHasWalkableFloor;
}
