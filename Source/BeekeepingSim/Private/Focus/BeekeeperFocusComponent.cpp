// Fill out your copyright notice in the Description page of Project Settings.


#include "Focus/BeekeeperFocusComponent.h"

#include "Camera/CameraComponent.h"
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
		return;
	}

	if (CurrentFocusTarget && !IsValid(CurrentFocusTarget))
	{
		CurrentFocusTarget = nullptr;
		BroadcastPreviewPromptState();
	}

	RefreshFocusTarget();
}

void UBeekeeperFocusComponent::ConfirmFocus()
{
	if (bIsFocusEngaged)
	{
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

FFocusPromptData UBeekeeperFocusComponent::GetCurrentPromptData() const
{
	if (bIsFocusEngaged || !IsValid(CurrentFocusTarget))
	{
		return FFocusPromptData();
	}

	return CurrentFocusTarget->GetPromptData();
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
	if (EngagedFocusAction && OwnerCharacter)
	{
		EngagedFocusAction->AbortFocusAction(OwnerCharacter);
	}

	EngagedFocusTarget = nullptr;
	EngagedFocusAction = nullptr;
	bIsFocusEngaged = false;
	BroadcastEngagedFocusRule();
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
