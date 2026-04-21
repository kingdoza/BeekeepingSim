// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BeekeeperMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeekeeperMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UBeekeeperMovementComponent();
	
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Sprinting", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Sprinting", meta = (AllowPrivateAccess = "true"))
	float WalkingSpeed = 300;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Sprinting", meta = (AllowPrivateAccess = "true"))
	float SprintingSpeed = 600;
	
protected:
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void StartSprinting();
	
	void StopSprinting();
	
	void SwitchSprinting();
	
	bool IsForwardAccelerating() const;

	UFUNCTION(BlueprintPure, Category = "Character Movement: Sprinting")
	bool IsSprinting() const { return bIsSprinting; }
};
