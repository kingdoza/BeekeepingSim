// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BeekeepingSimCharacter.h"
#include "BeekeeperCharacter.generated.h"

class UBeekeeperMovementComponent;
class UBeekeeperCameraShakeComponent;
class UBeekeeperFocusComponent;
struct FInputActionValue;
class UInputAction;
class UCameraComponent;

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

	void FocusConfirmInput();

	void FocusCancelInput();
	
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
};
