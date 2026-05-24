// Fill out your copyright notice in the Description page of Project Settings.


#include "Focus/BeekeeperFocusComponent.h"

#include "Camera/CameraComponent.h"
#include "Focus/BeekeepingSimFocusSettings.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"
#include "Focus/FocusActionComponent.h"
#include "Focus/FocusTargetComponent.h"

UBeekeeperFocusComponent::UBeekeeperFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBeekeeperFocusComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABeekeeperCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCamera = OwnerCharacter->GetFirstPersonCamera();
	}

	BroadcastPreviewPromptState();
	BroadcastEngagedFocusRule();
	UpdateCrosshairVisibility(false);
}

void UBeekeeperFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShouldDisableTickForNonLocal())
	{
		if (bIsFocusEngaged)
		{
			ClearEngagedFocus();
		}

		if (CurrentFocusTarget)
		{
			ClearPreviewFocus(false);
		}

		UpdateCrosshairVisibility(false);

		SetComponentTickInterval(0.25f);
		return;
	}

	SetComponentTickInterval(0.0f);

	if (bIsFocusEngaged)
	{
		UpdateEngagedFocusState();
	}

	if (bIsFocusPrimaryPointerDown)
	{
		FVector2D CurrentMousePosition = FVector2D::ZeroVector;
		if (TryGetMouseScreenPosition(CurrentMousePosition))
		{
			const float MoveDistance = FVector2D::Distance(CurrentMousePosition, PressedFocusScreenPosition);
			MaxFocusPointerMoveDistanceSincePress = FMath::Max(MaxFocusPointerMoveDistanceSincePress, MoveDistance);
		}

		const UBeekeepingSimFocusSettings* FocusSettings = GetDefault<UBeekeepingSimFocusSettings>();
		const float Threshold = FocusSettings ? FMath::Max(0.0f, FocusSettings->ClickCancelThresholdPixels) : 12.0f;
		if (MaxFocusPointerMoveDistanceSincePress > Threshold)
		{
			bFocusClickCanceledByMovement = true;
		}
	}

	if (!bIsFocusEngaged)
	{
		if (CurrentFocusTarget && !IsValid(CurrentFocusTarget))
		{
			CurrentFocusTarget = nullptr;
			BroadcastPreviewPromptState();
		}

		RefreshFocusTarget();
	}
}

void UBeekeeperFocusComponent::ConfirmFocus()
{
	if (bIsFocusEngaged)
	{
		if (EngagedFocusAction && OwnerCharacter && EngagedFocusAction->HandleConfirmInputWhileEngaged(OwnerCharacter))
		{
			return;
		}

		CancelFocus();
		return;
	}

	if (!IsValid(CurrentFocusTarget) || !OwnerCharacter)
	{
		return;
	}

	UFocusActionComponent* FocusActionComponent = FindFocusActionComponent(CurrentFocusTarget);
	if (!FocusActionComponent || !FocusActionComponent->CanBeginFocusAction(OwnerCharacter))
	{
		return;
	}

	UFocusTargetComponent* PreviewTarget = CurrentFocusTarget;
	ClearPreviewFocus(false);

	EngagedFocusTarget = PreviewTarget;
	EngagedFocusAction = FocusActionComponent;
	bIsFocusEngaged = true;

	if (!EngagedFocusAction->BeginFocusAction(OwnerCharacter))
	{
		ClearEngagedFocus();
		if (IsValid(PreviewTarget))
		{
			SetPreviewFocusTarget(PreviewTarget);
		}
		return;
	}

	// Some focus actions, such as pickups, complete synchronously during BeginFocusAction.
	if (!IsValid(EngagedFocusTarget) || !IsValid(EngagedFocusAction) || !EngagedFocusAction->IsActionEngaged())
	{
		ClearEngagedFocus();
		return;
	}

	EngagedFocusTarget->NotifyFocusConfirm(OwnerCharacter);
	BroadcastEngagedFocusRule();
	RefreshCrosshairVisibilityFromCurrentAction();
}

void UBeekeeperFocusComponent::CancelFocus()
{
	if (!bIsFocusEngaged)
	{
		ClearPreviewFocus(true);
		return;
	}

	if (EngagedFocusAction && OwnerCharacter && EngagedFocusAction->HandleCancelInputWhileEngaged(OwnerCharacter))
	{
		return;
	}

	if (!IsValid(EngagedFocusTarget) || !EngagedFocusAction || !OwnerCharacter)
	{
		ClearEngagedFocus();
		return;
	}

	if (EngagedFocusAction->ShouldRestoreCrosshairOnCancelStart())
	{
		UpdateCrosshairVisibility(false);
	}

	if (EngagedFocusAction->CancelFocusAction(OwnerCharacter))
	{
		EngagedFocusTarget->NotifyFocusCancel(OwnerCharacter);
		return;
	}

	RefreshCrosshairVisibilityFromCurrentAction();
}

void UBeekeeperFocusComponent::HandleFocusPrimaryPressedInput()
{
	ResetFocusPrimaryGestureState();

	if (bIsFocusEngaged)
	{
		// Engaged confirm stays release-driven for LMB gesture consistency.
		return;
	}

	bIsFocusPrimaryPointerDown = true;
	PressedFocusTarget = IsValid(CurrentFocusTarget) ? CurrentFocusTarget : nullptr;
	TryGetMouseScreenPosition(PressedFocusScreenPosition);
}

void UBeekeeperFocusComponent::HandleFocusPrimaryReleasedInput()
{
	const bool bWasPointerDown = bIsFocusPrimaryPointerDown;
	const TObjectPtr<UFocusTargetComponent> PressedTarget = PressedFocusTarget;
	float MaxMoveDistance = MaxFocusPointerMoveDistanceSincePress;
	FVector2D ReleaseMousePosition = FVector2D::ZeroVector;
	if (bWasPointerDown && TryGetMouseScreenPosition(ReleaseMousePosition))
	{
		const float ReleaseMoveDistance = FVector2D::Distance(ReleaseMousePosition, PressedFocusScreenPosition);
		MaxMoveDistance = FMath::Max(MaxMoveDistance, ReleaseMoveDistance);
	}

	const UBeekeepingSimFocusSettings* FocusSettings = GetDefault<UBeekeepingSimFocusSettings>();
	const float Threshold = FocusSettings ? FMath::Max(0.0f, FocusSettings->ClickCancelThresholdPixels) : 12.0f;
	const bool bCanceledByMovement = bFocusClickCanceledByMovement || (MaxMoveDistance > Threshold);
	ResetFocusPrimaryGestureState();

	if (!bWasPointerDown || bCanceledByMovement || bIsFocusEngaged)
	{
		return;
	}

	if (!IsValid(PressedTarget) || !IsValid(CurrentFocusTarget))
	{
		return;
	}

	if (PressedTarget != CurrentFocusTarget)
	{
		return;
	}

	ConfirmFocus();
}

bool UBeekeeperFocusComponent::HandlePartFocusClickInput()
{
	return HandlePartFocusPointerPressedInput();
}

bool UBeekeeperFocusComponent::HandlePartFocusClickReleasedInput()
{
	return HandlePartFocusPointerReleasedInput();
}

bool UBeekeeperFocusComponent::HandlePartFocusPointerPressedInput()
{
	if (!bIsFocusEngaged || !EngagedFocusAction || !OwnerCharacter)
	{
		return false;
	}

	return EngagedFocusAction->HandlePartFocusPointerPressedInputWhileEngaged(OwnerCharacter);
}

bool UBeekeeperFocusComponent::HandlePartFocusPointerReleasedInput()
{
	if (!bIsFocusEngaged || !EngagedFocusAction || !OwnerCharacter)
	{
		return false;
	}

	return EngagedFocusAction->HandlePartFocusPointerReleasedInputWhileEngaged(OwnerCharacter);
}

bool UBeekeeperFocusComponent::HandlePartFocusPreviewKeyInput(ECursorPartFocusPreviewInputKey Key)
{
	if (!bIsFocusEngaged || !EngagedFocusAction || !OwnerCharacter)
	{
		return false;
	}

	return EngagedFocusAction->HandlePartFocusPreviewKeyInputWhileEngaged(OwnerCharacter, Key);
}

FFocusPromptData UBeekeeperFocusComponent::GetCurrentPromptData() const
{
	if (bIsFocusEngaged)
	{
		return bHasEngagedPromptOverride ? EngagedPromptOverride : FFocusPromptData();
	}

	if (!IsValid(CurrentFocusTarget))
	{
		return FFocusPromptData();
	}

	return CurrentFocusTarget->GetPromptData();
}

void UBeekeeperFocusComponent::SetEngagedFocusPromptOverride(const FFocusPromptData& PromptData)
{
	bHasEngagedPromptOverride = PromptData.bIsValid;
	EngagedPromptOverride = PromptData;
	BroadcastPreviewPromptState();
}

bool UBeekeeperFocusComponent::EvaluateItemAllowed(const FGameplayTagContainer& ItemTags, FGameplayTag AllItemsTag) const
{
	if (!bIsFocusEngaged || !EngagedFocusTarget)
	{
		return true;
	}

	const FGameplayTagContainer& AllowedItemTags = EngagedFocusTarget->GetItemRule().AllowedItemTags;
	if (AllowedItemTags.IsEmpty())
	{
		return false;
	}

	if (AllItemsTag.IsValid() && AllowedItemTags.HasTag(AllItemsTag))
	{
		return true;
	}

	return ItemTags.HasAny(AllowedItemTags);
}

void UBeekeeperFocusComponent::RefreshFocusTarget()
{
	UFocusTargetComponent* NewFocusTarget = FindFocusTargetFromTrace();
	if (NewFocusTarget == CurrentFocusTarget)
	{
		return;
	}

	SetPreviewFocusTarget(NewFocusTarget);
}

UFocusTargetComponent* UBeekeeperFocusComponent::FindFocusTargetFromTrace() const
{
	if (!OwnerCharacter || !OwnerCamera || !GetWorld())
	{
		return nullptr;
	}

	const FVector TraceStart = OwnerCamera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (OwnerCamera->GetForwardVector() * FocusTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BeekeeperFocusTrace), false, OwnerCharacter);
	if (!GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, FocusTraceChannel, QueryParams))
	{
		return nullptr;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return nullptr;
	}

	return HitActor->FindComponentByClass<UFocusTargetComponent>();
}

void UBeekeeperFocusComponent::SetPreviewFocusTarget(UFocusTargetComponent* NewFocusTarget)
{
	if (CurrentFocusTarget == NewFocusTarget)
	{
		return;
	}

	if (CurrentFocusTarget)
	{
		CurrentFocusTarget->SetFocused(false);
		if (OwnerCharacter)
		{
			CurrentFocusTarget->NotifyFocusExit(OwnerCharacter);
		}
	}

	CurrentFocusTarget = NewFocusTarget;

	if (CurrentFocusTarget)
	{
		CurrentFocusTarget->SetFocused(true);
		if (OwnerCharacter)
		{
			CurrentFocusTarget->NotifyFocusEnter(OwnerCharacter);
		}
	}

	BroadcastPreviewPromptState();
}

void UBeekeeperFocusComponent::ClearPreviewFocus(bool bNotifyCancel)
{
	if (!IsValid(CurrentFocusTarget))
	{
		CurrentFocusTarget = nullptr;
		BroadcastPreviewPromptState();
		return;
	}

	UFocusTargetComponent* PreviousFocusTarget = CurrentFocusTarget;
	CurrentFocusTarget = nullptr;

	if (bNotifyCancel && OwnerCharacter)
	{
		PreviousFocusTarget->NotifyFocusCancel(OwnerCharacter);
	}

	PreviousFocusTarget->SetFocused(false);
	if (OwnerCharacter)
	{
		PreviousFocusTarget->NotifyFocusExit(OwnerCharacter);
	}

	BroadcastPreviewPromptState();
}

void UBeekeeperFocusComponent::ClearEngagedFocus()
{
	ResetFocusPrimaryGestureState();

	if (EngagedFocusAction && OwnerCharacter)
	{
		EngagedFocusAction->AbortFocusAction(OwnerCharacter);
	}

	EngagedFocusTarget = nullptr;
	EngagedFocusAction = nullptr;
	bHasEngagedPromptOverride = false;
	EngagedPromptOverride = FFocusPromptData();
	bIsFocusEngaged = false;
	BroadcastEngagedFocusRule();
	BroadcastPreviewPromptState();
	UpdateCrosshairVisibility(false);
}

void UBeekeeperFocusComponent::BroadcastPreviewPromptState()
{
	OnFocusPromptChanged.Broadcast(GetCurrentPromptData());
}

void UBeekeeperFocusComponent::BroadcastEngagedFocusRule()
{
	if (!bIsFocusEngaged || !IsValid(EngagedFocusTarget))
	{
		OnFocusRuleChanged.Broadcast(false, FFocusItemRule());
		return;
	}

	OnFocusRuleChanged.Broadcast(true, EngagedFocusTarget->GetItemRule());
}

void UBeekeeperFocusComponent::UpdateCrosshairVisibility(bool bNewShouldHideCrosshair)
{
	if (bShouldHideCrosshair == bNewShouldHideCrosshair)
	{
		return;
	}

	bShouldHideCrosshair = bNewShouldHideCrosshair;
	OnCrosshairVisibilityChanged.Broadcast(!bShouldHideCrosshair);
}

void UBeekeeperFocusComponent::RefreshCrosshairVisibilityFromCurrentAction()
{
	const bool bNewShouldHideCrosshair = bIsFocusEngaged && EngagedFocusAction && EngagedFocusAction->WantsCrosshairHiddenWhileEngaged();
	UpdateCrosshairVisibility(bNewShouldHideCrosshair);
}

UFocusActionComponent* UBeekeeperFocusComponent::FindFocusActionComponent(const UFocusTargetComponent* FocusTarget) const
{
	if (!FocusTarget || !FocusTarget->GetOwner())
	{
		return nullptr;
	}

	return FocusTarget->GetOwner()->FindComponentByClass<UFocusActionComponent>();
}

void UBeekeeperFocusComponent::UpdateEngagedFocusState()
{
	if (!IsValid(EngagedFocusTarget) || !EngagedFocusAction)
	{
		ClearEngagedFocus();
		return;
	}

	if (EngagedFocusAction->IsActionEngaged())
	{
		return;
	}

	ClearEngagedFocus();
}

bool UBeekeeperFocusComponent::ShouldDisableTickForNonLocal() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	return !PlayerController || !PlayerController->IsLocalController();
}

bool UBeekeeperFocusComponent::TryGetMouseScreenPosition(FVector2D& OutPosition) const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return false;
	}

	float ScreenX = 0.0f;
	float ScreenY = 0.0f;
	if (!PlayerController->GetMousePosition(ScreenX, ScreenY))
	{
		return false;
	}

	OutPosition = FVector2D(ScreenX, ScreenY);
	return true;
}

void UBeekeeperFocusComponent::ResetFocusPrimaryGestureState()
{
	PressedFocusTarget = nullptr;
	PressedFocusScreenPosition = FVector2D::ZeroVector;
	MaxFocusPointerMoveDistanceSincePress = 0.0f;
	bIsFocusPrimaryPointerDown = false;
	bFocusClickCanceledByMovement = false;
}
