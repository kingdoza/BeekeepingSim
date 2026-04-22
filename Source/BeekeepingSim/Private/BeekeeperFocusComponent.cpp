// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BeekeeperFocusComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Public/BeekeeperCharacter.h"
#include "Public/FocusTargetComponent.h"

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

	BroadcastFocusState();
}

void UBeekeeperFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentFocusTarget && !IsValid(CurrentFocusTarget))
	{
		CurrentFocusTarget = nullptr;
		BroadcastFocusState();
	}

	if (ShouldDisableTickForNonLocal())
	{
		if (CurrentFocusTarget)
		{
			ClearFocusTarget(false);
		}

		SetComponentTickInterval(0.25f);
		return;
	}

	SetComponentTickInterval(0.0f);
	RefreshFocusTarget();
}

void UBeekeeperFocusComponent::ConfirmFocus()
{
	if (!IsValid(CurrentFocusTarget) || !OwnerCharacter)
	{
		return;
	}

	UFocusTargetComponent* FocusTarget = CurrentFocusTarget;
	const bool bShouldClearFocusOnConfirm = FocusTarget->ShouldClearFocusOnConfirm();

	FocusTarget->NotifyFocusConfirm(OwnerCharacter);
	if (bShouldClearFocusOnConfirm && CurrentFocusTarget == FocusTarget)
	{
		ClearFocusTarget(false);
	}
}

void UBeekeeperFocusComponent::CancelFocus()
{
	ClearFocusTarget(true);
}

FFocusPromptData UBeekeeperFocusComponent::GetCurrentPromptData() const
{
	if (!IsValid(CurrentFocusTarget))
	{
		return FFocusPromptData();
	}

	return CurrentFocusTarget->GetPromptData();
}

bool UBeekeeperFocusComponent::EvaluateItemAllowed(const FGameplayTagContainer& ItemTags, FGameplayTag AllItemsTag) const
{
	if (!CurrentFocusTarget)
	{
		return true;
	}

	const FGameplayTagContainer& AllowedItemTags = CurrentFocusTarget->GetItemRule().AllowedItemTags;
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

	SetFocusTarget(NewFocusTarget);
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

void UBeekeeperFocusComponent::SetFocusTarget(UFocusTargetComponent* NewFocusTarget)
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

	BroadcastFocusState();
}

void UBeekeeperFocusComponent::ClearFocusTarget(bool bNotifyCancel)
{
	if (!IsValid(CurrentFocusTarget))
	{
		CurrentFocusTarget = nullptr;
		BroadcastFocusState();
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

	BroadcastFocusState();
}

void UBeekeeperFocusComponent::BroadcastFocusState()
{
	OnFocusPromptChanged.Broadcast(GetCurrentPromptData());

	if (!IsValid(CurrentFocusTarget))
	{
		OnFocusRuleChanged.Broadcast(false, FFocusItemRule());
		return;
	}

	OnFocusRuleChanged.Broadcast(true, CurrentFocusTarget->GetItemRule());
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
