// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BeekeeperCameraShakeComponent.generated.h"

class ABeekeeperCharacter;
class APlayerCameraManager;
class APlayerController;
class UBeekeeperMovementComponent;
class UCameraShakeBase;

UENUM(BlueprintType)
enum class EBeekeeperMoveState : uint8
{
	Idle,
	Walk,
	Sprint
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeekeeperCameraShakeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBeekeeperCameraShakeComponent();

	UFUNCTION(BlueprintCallable, Category = "Camera Shake")
	void StopAllCameraShakes();

	UFUNCTION(BlueprintCallable, Category = "Camera Shake")
	void SuppressNextLandingShake();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> IdleCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> WalkCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> SprintCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> LandingCameraShakeClass;

	// 추측: 3.0f is a safe default for "almost zero" horizontal speed and should be tuned per project feel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake", meta = (ClampMin = "0.0"))
	float IdleSpeedThreshold = 3.0f;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	EBeekeeperMoveState CalculateMoveState() const;

	void ApplyMoveState(EBeekeeperMoveState NewState);

	void StopCurrentShake();

	void StopMoveShake();

	void PlayLandingShake();

	TSubclassOf<UCameraShakeBase> GetShakeClassForState(EBeekeeperMoveState State) const;

	APlayerController* GetLocalPlayerController() const;

	bool ShouldDisableTickForNonLocal() const;

	bool IsOwnerFalling() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperMovementComponent> OwnerMovementComponent;

	EBeekeeperMoveState AppliedMoveState = EBeekeeperMoveState::Idle;

	bool bHasAppliedMoveState = false;

	bool bWasFalling = false;

	bool bSuppressNextLandingShake = false;

	UPROPERTY(Transient)
	TObjectPtr<APlayerCameraManager> CurrentPlayerCameraManager;

	TSubclassOf<UCameraShakeBase> CurrentShakeClass;
};
