// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BeekeepingSimCharacter.h"
#include "BeekeeperCharacter.generated.h"

class UBeekeeperMovementComponent;
class UBeekeeperCameraShakeComponent;
class UBeekeeperFocusComponent;
class UBeekeeperHotbarComponent;
class UBeekeeperHeldItemVisualizerComponent;
class UBeekeeperFlashlightComponent;
struct FInputActionValue;
class UInputAction;
class UCameraComponent;
class USkeletalMeshComponent;

UCLASS()
class BEEKEEPINGSIM_API ABeekeeperCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	ABeekeeperCharacter(const FObjectInitializer& ObjectInitializer);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBeekeeperMovementComponent> BeekeeperMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBeekeeperCameraShakeComponent> BeekeeperCameraShake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBeekeeperFocusComponent> BeekeeperFocus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBeekeeperHotbarComponent> BeekeeperHotbar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBeekeeperHeldItemVisualizerComponent> BeekeeperHeldItemVisualizer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBeekeeperFlashlightComponent> BeekeeperFlashlight;
	
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MouseSensitivity;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> FocusConfirmAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> FocusCancelAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> HotbarSlotAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> HotbarWheelAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> PartFocusClickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> PartFocusRAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> PartFocusFAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> PartFocusCAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> FlashlightToggleAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeedScale = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (ClampMin = "0.0"))
	float LookSpeedScale = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool bIsSprintToggle = true;

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	void MoveInput(const FInputActionValue& Value);
	
	void LookInput(const FInputActionValue& Value);
	
	void SprintStartInput();
	
	void SprintReleaseInput();

	void FocusPrimaryPressedInput();

	void FocusPrimaryReleasedInput();

	void FocusConfirmInput();

	void FocusCancelInput();

	void HotbarSlotInput(const FInputActionValue& Value);

	void HotbarWheelInput(const FInputActionValue& Value);

	void PartFocusClickInput();

	void PartFocusPointerPressedInput();

	void PartFocusPointerReleasedInput();

	void PartFocusClickReleaseInput();

	void PartFocusRInput();

	void PartFocusFInput();

	void PartFocusCInput();

	void FlashlightToggleInput();
	
	UFUNCTION(Blueprintable, Category = "Input")
	void DoMove(float Right, float Forward);
	
	UFUNCTION(Blueprintable, Category = "Input")
	void DoLook(float Yaw, float Pitch);
	
	UFUNCTION(Blueprintable, Category = "Input")
	void DoJumpStart();
	
	UFUNCTION(Blueprintable, Category = "Input")
	void DoJumpEnd();
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:	
	
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	UBeekeeperFocusComponent* GetBeekeeperFocus() const { return BeekeeperFocus; }

	UBeekeeperCameraShakeComponent* GetBeekeeperCameraShake() const { return BeekeeperCameraShake; }

	UBeekeeperHotbarComponent* GetBeekeeperHotbar() const { return BeekeeperHotbar; }

	UBeekeeperHeldItemVisualizerComponent* GetBeekeeperHeldItemVisualizer() const { return BeekeeperHeldItemVisualizer; }

	UBeekeeperFlashlightComponent* GetBeekeeperFlashlight() const { return BeekeeperFlashlight; }

	void SetFocusInteractionInputLocked(bool bLocked);

	bool IsFocusInteractionInputLocked() const { return bIsFocusInteractionInputLocked; }

	void BeginFocusCameraOverride();

	void UpdateFocusCameraOverride(const FVector& WorldLocation, const FRotator& WorldRotation);

	void EndFocusCameraOverride();

	FTransform GetDefaultFocusCameraWorldTransform() const;

	void SyncControlRotationTo(const FRotator& NewControlRotation);

	bool MoveToCharacterAnchor(const FTransform& AnchorTransform);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus", meta = (AllowPrivateAccess = "true"))
	FName FirstPersonCameraAttachSocketName = TEXT("head");

private:
	bool bIsFocusInteractionInputLocked = false;

	bool bIsFocusCameraOverrideActive = false;

	bool bStoredUsePawnControlRotation = true;

	FVector DefaultFirstPersonCameraRelativeLocation = FVector::ZeroVector;

	FRotator DefaultFirstPersonCameraRelativeRotation = FRotator::ZeroRotator;
};
