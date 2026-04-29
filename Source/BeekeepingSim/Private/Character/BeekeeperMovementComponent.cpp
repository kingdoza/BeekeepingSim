// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BeekeeperMovementComponent.h"

void UBeekeeperMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Acceleration.GetSafeNormal().IsNearlyZero() && bIsSprinting)
	{
		StopSprinting();
	}
}

UBeekeeperMovementComponent::UBeekeeperMovementComponent()
{
	MaxWalkSpeed = WalkingSpeed;
}

void UBeekeeperMovementComponent::StartSprinting()
{
	if (bIsSprinting || !IsForwardAccelerating() || IsFalling()) return;
	bIsSprinting = true;
	MaxWalkSpeed = SprintingSpeed;
}

void UBeekeeperMovementComponent::StopSprinting()
{
	if (!bIsSprinting) return;
	bIsSprinting = false;
	MaxWalkSpeed = WalkingSpeed;
}

void UBeekeeperMovementComponent::SwitchSprinting()
{
	if (bIsSprinting)
	{
		StopSprinting();
	}
	else
	{
		StartSprinting();
	}
}

bool UBeekeeperMovementComponent::IsForwardAccelerating() const
{
	FVector InputVector = Acceleration.GetSafeNormal();
	if (InputVector.IsNearlyZero()) return false;
	FVector OwnerForwardVector = GetOwner()->GetActorForwardVector();
	float ForwardInfluence = FVector::DotProduct(InputVector, OwnerForwardVector);
	constexpr float InfluenceThreshold = 0.01f;
	return ForwardInfluence > InfluenceThreshold;
}
