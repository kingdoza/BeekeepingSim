// Fill out your copyright notice in the Description page of Project Settings.


#include "Focus/AnchoredFocusActionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/BeekeeperCameraShakeComponent.h"
#include "Character/BeekeeperCharacter.h"

UAnchoredFocusActionComponent::UAnchoredFocusActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UAnchoredFocusActionComponent::CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
	return UFocusActionComponent::CanBeginFocusAction(InteractingCharacter)
		&& ResolveAnchorByTag(FocusAnchorTag) != nullptr
		&& ResolveAnchorByTag(CharacterAnchorTag) != nullptr;
}

bool UAnchoredFocusActionComponent::BeginFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!UFocusActionComponent::BeginFocusAction(InteractingCharacter))
	{
		return false;
	}

	FocusAnchorComponent = ResolveAnchorByTag(FocusAnchorTag);
	CharacterAnchorComponent = ResolveAnchorByTag(CharacterAnchorTag);
	if (!FocusAnchorComponent || !CharacterAnchorComponent || !InteractingCharacter)
	{
		bIsActionEngaged = false;
		return false;
	}

	CurrentInteractingCharacter = InteractingCharacter;
	CurrentInteractingCharacter->SetFocusInteractionInputLocked(true);
	CurrentInteractingCharacter->BeginFocusCameraOverride();

	if (UBeekeeperCameraShakeComponent* CameraShakeComponent = CurrentInteractingCharacter->GetBeekeeperCameraShake())
	{
		CameraShakeComponent->StopAllCameraShakes();
		const bool bCausedForcedLanding = CurrentInteractingCharacter->MoveToCharacterAnchor(CharacterAnchorComponent->GetComponentTransform());
		if (bCausedForcedLanding)
		{
			CameraShakeComponent->SuppressNextLandingShake();
		}
	}
	else
	{
		CurrentInteractingCharacter->MoveToCharacterAnchor(CharacterAnchorComponent->GetComponentTransform());
	}

	ActionState = EAnchoredFocusActionState::BlendingToFocus;
	SetComponentTickEnabled(true);
	OnFocusEngagedStarted(CurrentInteractingCharacter);
	return true;
}

bool UAnchoredFocusActionComponent::CancelFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	if (!InteractingCharacter || !bIsActionEngaged || !CurrentInteractingCharacter || CurrentInteractingCharacter != InteractingCharacter)
	{
		return false;
	}

	ActionState = EAnchoredFocusActionState::BlendingBack;
	SetComponentTickEnabled(true);
	OnFocusCancelStarted(CurrentInteractingCharacter);
	return true;
}

void UAnchoredFocusActionComponent::AbortFocusAction(ABeekeeperCharacter* InteractingCharacter)
{
	ABeekeeperCharacter* CharacterToRestore = CurrentInteractingCharacter ? CurrentInteractingCharacter.Get() : InteractingCharacter;
	OnFocusActionAborted(CharacterToRestore);
	if (CharacterToRestore)
	{
		CharacterToRestore->EndFocusCameraOverride();
		CharacterToRestore->SetFocusInteractionInputLocked(false);
	}

	CurrentInteractingCharacter = nullptr;
	ActionState = EAnchoredFocusActionState::Idle;
	bIsActionEngaged = false;
	SetComponentTickEnabled(false);
}

void UAnchoredFocusActionComponent::BeginPlay()
{
	Super::BeginPlay();
	FocusAnchorComponent = ResolveAnchorByTag(FocusAnchorTag);
	CharacterAnchorComponent = ResolveAnchorByTag(CharacterAnchorTag);
}

void UAnchoredFocusActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentInteractingCharacter)
	{
		ActionState = EAnchoredFocusActionState::Idle;
		bIsActionEngaged = false;
		SetComponentTickEnabled(false);
		return;
	}

	switch (ActionState)
	{
	case EAnchoredFocusActionState::BlendingToFocus:
		if (FocusAnchorComponent && UpdateCameraBlend(DeltaTime, FocusAnchorComponent->GetComponentTransform(), FocusBlendInterpSpeed))
		{
			ActionState = EAnchoredFocusActionState::Focused;
		}
		break;
	case EAnchoredFocusActionState::Focused:
		if (FocusAnchorComponent)
		{
			CurrentInteractingCharacter->UpdateFocusCameraOverride(FocusAnchorComponent->GetComponentLocation(), FocusAnchorComponent->GetComponentRotation());
		}
		break;
	case EAnchoredFocusActionState::BlendingBack:
		if (UpdateCameraBlend(DeltaTime, CurrentInteractingCharacter->GetDefaultFocusCameraWorldTransform(), ReturnBlendInterpSpeed))
		{
			const FTransform DefaultCameraTransform = CurrentInteractingCharacter->GetDefaultFocusCameraWorldTransform();
			CurrentInteractingCharacter->SyncControlRotationTo(DefaultCameraTransform.GetRotation().Rotator());
			CurrentInteractingCharacter->EndFocusCameraOverride();
			OnFocusReturnCompleted(CurrentInteractingCharacter);
			CurrentInteractingCharacter->SetFocusInteractionInputLocked(false);
			CurrentInteractingCharacter = nullptr;
			ActionState = EAnchoredFocusActionState::Idle;
			bIsActionEngaged = false;
			SetComponentTickEnabled(false);
		}
		break;
	case EAnchoredFocusActionState::Idle:
	default:
		SetComponentTickEnabled(false);
		break;
	}
}

USceneComponent* UAnchoredFocusActionComponent::ResolveAnchorByTag(const FName& AnchorTag) const
{
	if (!GetOwner() || AnchorTag.IsNone())
	{
		return nullptr;
	}

	TArray<USceneComponent*> SceneComponents;
	GetOwner()->GetComponents<USceneComponent>(SceneComponents);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (!SceneComponent)
		{
			continue;
		}

		if (SceneComponent->ComponentHasTag(AnchorTag))
		{
			return SceneComponent;
		}
	}

	return nullptr;
}

bool UAnchoredFocusActionComponent::UpdateCameraBlend(float DeltaTime, const FTransform& TargetTransform, float InterpSpeed)
{
	if (!CurrentInteractingCharacter || !CurrentInteractingCharacter->GetFirstPersonCamera())
	{
		return false;
	}

	UCameraComponent* CameraComponent = CurrentInteractingCharacter->GetFirstPersonCamera();
	const FVector CurrentLocation = CameraComponent->GetComponentLocation();
	const FRotator CurrentRotation = CameraComponent->GetComponentRotation();
	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetTransform.GetLocation(), DeltaTime, InterpSpeed);
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetTransform.GetRotation().Rotator(), DeltaTime, InterpSpeed);

	CurrentInteractingCharacter->UpdateFocusCameraOverride(NewLocation, NewRotation);

	const bool bLocationReached = FVector::DistSquared(NewLocation, TargetTransform.GetLocation()) <= FMath::Square(BlendCompletionDistance);
	const bool bRotationReached = FMath::Abs(UKismetMathLibrary::NormalizedDeltaRotator(NewRotation, TargetTransform.GetRotation().Rotator()).Yaw) <= BlendCompletionAngle
		&& FMath::Abs(UKismetMathLibrary::NormalizedDeltaRotator(NewRotation, TargetTransform.GetRotation().Rotator()).Pitch) <= BlendCompletionAngle
		&& FMath::Abs(UKismetMathLibrary::NormalizedDeltaRotator(NewRotation, TargetTransform.GetRotation().Rotator()).Roll) <= BlendCompletionAngle;

	return bLocationReached && bRotationReached;
}

void UAnchoredFocusActionComponent::OnFocusEngagedStarted(ABeekeeperCharacter* InteractingCharacter)
{
}

void UAnchoredFocusActionComponent::OnFocusCancelStarted(ABeekeeperCharacter* InteractingCharacter)
{
}

void UAnchoredFocusActionComponent::OnFocusReturnCompleted(ABeekeeperCharacter* InteractingCharacter)
{
}

void UAnchoredFocusActionComponent::OnFocusActionAborted(ABeekeeperCharacter* InteractingCharacter)
{
}
