// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BeekeeperCharacter.h"
#include "Camera/BeekeeperCameraShakeComponent.h"
#include "Character/BeekeeperController.h"
#include "Focus/BeekeeperFocusComponent.h"
#include "Focus/CursorPartFocusTypes.h"
#include "Character/BeekeeperHeldItemVisualizerComponent.h"
#include "Character/BeekeeperFlashlightComponent.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Character/BeekeeperMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"


ABeekeeperCharacter::ABeekeeperCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBeekeeperMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	BeekeeperMovement = Cast<UBeekeeperMovementComponent>(GetCharacterMovement());
	BeekeeperCameraShake = CreateDefaultSubobject<UBeekeeperCameraShakeComponent>(TEXT("BeekeeperCameraShake"));
	BeekeeperFocus = CreateDefaultSubobject<UBeekeeperFocusComponent>(TEXT("BeekeeperFocus"));
	BeekeeperHotbar = CreateDefaultSubobject<UBeekeeperHotbarComponent>(TEXT("BeekeeperHotbar"));
	BeekeeperHeldItemVisualizer = CreateDefaultSubobject<UBeekeeperHeldItemVisualizerComponent>(TEXT("BeekeeperHeldItemVisualizer"));
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("head"));
	FirstPersonCamera->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	BeekeeperFlashlight = CreateDefaultSubobject<UBeekeeperFlashlightComponent>(TEXT("BeekeeperFlashlight"));
	BeekeeperFlashlight->SetupAttachment(FirstPersonCamera);
	
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

void ABeekeeperCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FirstPersonCamera)
	{
		DefaultFirstPersonCameraRelativeLocation = FirstPersonCamera->GetRelativeLocation();
		DefaultFirstPersonCameraRelativeRotation = FirstPersonCamera->GetRelativeRotation();

		if (BeekeeperFlashlight)
		{
			BeekeeperFlashlight->InitializeFlashlightAttachment(FirstPersonCamera);
		}
	}
}

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
			EnhancedInputComponent->BindAction(FocusConfirmAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::FocusPrimaryPressedInput);
			EnhancedInputComponent->BindAction(FocusConfirmAction, ETriggerEvent::Completed, this, &ABeekeeperCharacter::FocusPrimaryReleasedInput);
		}

		if (FocusCancelAction)
		{
			EnhancedInputComponent->BindAction(FocusCancelAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::FocusCancelInput);
		}

		if (FocusSecondaryAction)
		{
			EnhancedInputComponent->BindAction(FocusSecondaryAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::FocusSecondaryInput);
		}

		if (HotbarSlotAction)
		{
			EnhancedInputComponent->BindAction(HotbarSlotAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::HotbarSlotInput);
		}

		if (HotbarWheelAction)
		{
			EnhancedInputComponent->BindAction(HotbarWheelAction, ETriggerEvent::Triggered, this, &ABeekeeperCharacter::HotbarWheelInput);
		}

		if (HotbarToggleSelectionAction)
		{
			EnhancedInputComponent->BindAction(HotbarToggleSelectionAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::HotbarToggleSelectionInput);
		}

		if (PartFocusClickAction)
		{
			EnhancedInputComponent->BindAction(PartFocusClickAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::PartFocusPointerPressedInput);
			EnhancedInputComponent->BindAction(PartFocusClickAction, ETriggerEvent::Completed, this, &ABeekeeperCharacter::PartFocusPointerReleasedInput);
		}

		if (PartFocusRAction)
		{
			EnhancedInputComponent->BindAction(PartFocusRAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::PartFocusRInput);
		}

		if (PartFocusFAction)
		{
			EnhancedInputComponent->BindAction(PartFocusFAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::PartFocusFInput);
		}

		if (PartFocusCAction)
		{
			EnhancedInputComponent->BindAction(PartFocusCAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::PartFocusCInput);
		}

		if (FlashlightToggleAction)
		{
			EnhancedInputComponent->BindAction(FlashlightToggleAction, ETriggerEvent::Started, this, &ABeekeeperCharacter::FlashlightToggleInput);
		}
	}
}

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

void ABeekeeperCharacter::FocusPrimaryPressedInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandleFocusPrimaryPressedInput();
}

void ABeekeeperCharacter::FocusPrimaryReleasedInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandleFocusPrimaryReleasedInput();
}

void ABeekeeperCharacter::FocusConfirmInput()
{
	FocusPrimaryPressedInput();
}

void ABeekeeperCharacter::FocusCancelInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->CancelFocus();
}

void ABeekeeperCharacter::FocusSecondaryInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandleSecondaryInput();
}

void ABeekeeperCharacter::PartFocusClickInput()
{
	PartFocusPointerPressedInput();
}

void ABeekeeperCharacter::PartFocusPointerPressedInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandlePartFocusPointerPressedInput();
}

void ABeekeeperCharacter::PartFocusPointerReleasedInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandlePartFocusPointerReleasedInput();
}

void ABeekeeperCharacter::PartFocusClickReleaseInput()
{
	PartFocusPointerReleasedInput();
}

void ABeekeeperCharacter::PartFocusRInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandlePartFocusPreviewKeyInput(ECursorPartFocusPreviewInputKey::R);
}

void ABeekeeperCharacter::PartFocusFInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandlePartFocusPreviewKeyInput(ECursorPartFocusPreviewInputKey::F);
}

void ABeekeeperCharacter::PartFocusCInput()
{
	if (!BeekeeperFocus)
	{
		return;
	}

	BeekeeperFocus->HandlePartFocusPreviewKeyInput(ECursorPartFocusPreviewInputKey::C);
}

void ABeekeeperCharacter::FlashlightToggleInput()
{
	if (!BeekeeperFlashlight)
	{
		return;
	}

	BeekeeperFlashlight->ToggleFlashlight();
}

void ABeekeeperCharacter::HotbarSlotInput(const FInputActionValue& Value)
{
	if (!BeekeeperHotbar)
	{
		return;
	}

	const int32 SlotNumber = FMath::RoundToInt(Value.Get<float>());
	if (SlotNumber <= 0)
	{
		return;
	}

	BeekeeperHotbar->HandleSlotInput(SlotNumber - 1);
}

void ABeekeeperCharacter::HotbarWheelInput(const FInputActionValue& Value)
{
	const float WheelValue = Value.Get<float>();
	if (FMath::IsNearlyZero(WheelValue))
	{
		return;
	}

	if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(GetController()))
	{
		if (BeekeeperController->GetActiveItemSlotDragOperation())
		{
			BeekeeperController->AdjustActiveItemSlotDragQuantity(WheelValue);
			return;
		}
	}

	if (!BeekeeperHotbar)
	{
		return;
	}

	BeekeeperHotbar->HandleWheelInput(WheelValue > 0.0f);
}

void ABeekeeperCharacter::HotbarToggleSelectionInput()
{
	if (!BeekeeperHotbar)
	{
		return;
	}

	BeekeeperHotbar->ToggleSelectionFromLastSelectedSlot();
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
