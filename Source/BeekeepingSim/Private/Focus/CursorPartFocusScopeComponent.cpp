#include "Focus/CursorPartFocusScopeComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Focus/BeekeepingSimFocusSettings.h"
#include "Focus/BeekeeperFocusComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "GameFramework/PlayerController.h"

UCursorPartFocusScopeComponent::UCursorPartFocusScopeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCursorPartFocusScopeComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ABeekeeperCharacter>(GetOwner());
	OwnerFocusComponent = OwnerCharacter ? OwnerCharacter->GetBeekeeperFocus() : nullptr;
}

void UCursorPartFocusScopeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivatePartFocusScope(true);
	Super::EndPlay(EndPlayReason);
}

void UCursorPartFocusScopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsScopeActive)
	{
		return;
	}

	UpdateHoveredPartFromCursor();
	RemoveInactiveActions();
}

void UCursorPartFocusScopeComponent::ActivatePartFocusScope(ABeekeeperCharacter* InteractingCharacter)
{
	OwnerCharacter = InteractingCharacter ? InteractingCharacter : Cast<ABeekeeperCharacter>(GetOwner());
	OwnerFocusComponent = OwnerCharacter ? OwnerCharacter->GetBeekeeperFocus() : nullptr;
	bIsScopeActive = OwnerCharacter != nullptr;
	SetComponentTickEnabled(bIsScopeActive);
	UpdateHoveredPartFromCursor();
}

void UCursorPartFocusScopeComponent::DeactivatePartFocusScope(bool bAbortActiveActions)
{
	if (bAbortActiveActions)
	{
		while (CancelTopActionCascade(true))
		{
		}
	}

	SetHoveredPartIndex(INDEX_NONE);
	bHoverOutlineSuppressed = false;
	bIsScopeActive = false;
	SetComponentTickEnabled(false);
	BroadcastPartPrompt();
}

bool UCursorPartFocusScopeComponent::HandleConfirmInput()
{
	return HandlePartFocusClickInput();
}

bool UCursorPartFocusScopeComponent::HandlePartFocusClickInput()
{
	if (!bIsScopeActive)
	{
		return false;
	}

	UpdateHoveredPartFromCursor();
	if (RegisteredParts.IsValidIndex(HoveredPartIndex))
	{
		const FCursorPartFocusPartDescriptor& Descriptor = RegisteredParts[HoveredPartIndex];
		if (IsDescriptorPreviewAllowed(Descriptor))
		{
			if (Descriptor.EngageMode == ECursorPartFocusEngageMode::PreviewOnly)
			{
				return true;
			}

			UCursorPartFocusActionComponent* Action = Descriptor.ActionHandler;
			if (!Action || !OwnerCharacter)
			{
				return true;
			}

			if (Action->IsPartActionEngaged())
			{
				TSet<TObjectPtr<UCursorPartFocusActionComponent>> Visited;
				CancelActionCascade(Action, false, Visited);
				RemoveInactiveActions();
				return true;
			}

			return BeginPartActionForDescriptor(Descriptor);
		}
	}

	if (HandleEdgeCancelClick())
	{
		if (!HandleCancelInput())
		{
			RequestHostFocusCancel();
		}
		return true;
	}

	return true;
}

bool UCursorPartFocusScopeComponent::HandleCancelInput()
{
	if (!bIsScopeActive)
	{
		return false;
	}

	if (CancelTopActionCascade(false))
	{
		RemoveInactiveActions();
		return true;
	}

	return false;
}

void UCursorPartFocusScopeComponent::SetHoverOutlineSuppressed(bool bSuppressed)
{
	if (bHoverOutlineSuppressed == bSuppressed)
	{
		return;
	}

	bHoverOutlineSuppressed = bSuppressed;
	ApplyOutlineForPart(HoveredPartIndex, !bHoverOutlineSuppressed);
	BroadcastPartPrompt();
}

bool UCursorPartFocusScopeComponent::HandlePreviewKeyInput(ECursorPartFocusPreviewInputKey Key)
{
	if (!bIsScopeActive)
	{
		return false;
	}

	UpdateHoveredPartFromCursor();
	if (!RegisteredParts.IsValidIndex(HoveredPartIndex))
	{
		return false;
	}

	const FCursorPartFocusPartDescriptor& Descriptor = RegisteredParts[HoveredPartIndex];
	if (!IsDescriptorPreviewAllowed(Descriptor))
	{
		return true;
	}

	UCursorPartFocusActionComponent* Action = Descriptor.ActionHandler;
	if (!Action || !OwnerCharacter)
	{
		return true;
	}

	if (!Action->CanHandlePreviewKeyAction(Key))
	{
		return true;
	}

	Action->HandlePreviewKeyAction(this, OwnerCharacter, Key);
	return true;
}

void UCursorPartFocusScopeComponent::ClearRegisteredParts()
{
	SetHoveredPartIndex(INDEX_NONE);
	RegisteredParts.Reset();
}

void UCursorPartFocusScopeComponent::RegisterPartDescriptor(const FCursorPartFocusPartDescriptor& Descriptor)
{
	if (Descriptor.PartId.IsNone())
	{
		return;
	}

	FCursorPartFocusPartDescriptor NewDescriptor = Descriptor;
	if (NewDescriptor.ActionHandler)
	{
		NewDescriptor.RequiredStateTags.AppendTags(NewDescriptor.ActionHandler->GetRequiredStateTags());
	}
	ResolveDescriptorOutlineComponents(NewDescriptor);
	RegisteredParts.Add(NewDescriptor);
}

FGameplayTagContainer UCursorPartFocusScopeComponent::GetActiveProvidedStateTags() const
{
	FGameplayTagContainer StateTags;
	for (UCursorPartFocusActionComponent* Action : ActivePartActions)
	{
		if (Action && Action->IsPartActionEngaged())
		{
			StateTags.AppendTags(Action->GetProvidedStateTags());
		}
	}

	return StateTags;
}

UCursorPartFocusScopeComponent::FResolvedPartHit UCursorPartFocusScopeComponent::ResolvePartFromCursorTrace() const
{
	FResolvedPartHit Result;
	if (!OwnerCharacter)
	{
		return Result;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return Result;
	}

	float ScreenX = 0.0f;
	float ScreenY = 0.0f;
	if (!PlayerController->GetMousePosition(ScreenX, ScreenY))
	{
		return Result;
	}

	FVector WorldOrigin = FVector::ZeroVector;
	FVector WorldDirection = FVector::ForwardVector;
	if (!PlayerController->DeprojectScreenPositionToWorld(ScreenX, ScreenY, WorldOrigin, WorldDirection))
	{
		return Result;
	}

	const FVector TraceStart = WorldOrigin;
	const FVector TraceEnd = TraceStart + (WorldDirection * CursorTraceDistance);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CursorPartFocusTrace), true, OwnerCharacter);
	if (!GetWorld() || !GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CursorTraceChannel, QueryParams))
	{
		return Result;
	}

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	AActor* HitActor = HitResult.GetActor();
	if (!HitComponent && !HitActor)
	{
		return Result;
	}

	// Pass 1: explicit hit-component match only.
	for (int32 Index = 0; Index < RegisteredParts.Num(); ++Index)
	{
		const FCursorPartFocusPartDescriptor& Descriptor = RegisteredParts[Index];
		if (!IsDescriptorPreviewAllowed(Descriptor))
		{
			continue;
		}

		if (Descriptor.HitComponent && Descriptor.HitComponent == HitComponent)
		{
			Result.PartIndex = Index;
			Result.HitComponent = HitComponent;
			return Result;
		}
	}

	// Pass 2: owner fallback only for descriptors without explicit hit component.
	for (int32 Index = 0; Index < RegisteredParts.Num(); ++Index)
	{
		const FCursorPartFocusPartDescriptor& Descriptor = RegisteredParts[Index];
		if (!IsDescriptorPreviewAllowed(Descriptor))
		{
			continue;
		}

		if (Descriptor.HitComponent)
		{
			continue;
		}

		if (Descriptor.OwnerActor && Descriptor.OwnerActor == HitActor)
		{
			Result.PartIndex = Index;
			Result.HitComponent = HitComponent;
			return Result;
		}
	}

	return Result;
}

void UCursorPartFocusScopeComponent::UpdateHoveredPartFromCursor()
{
	const FResolvedPartHit Hit = ResolvePartFromCursorTrace();
	SetHoveredPartIndex(Hit.PartIndex);
}

void UCursorPartFocusScopeComponent::SetHoveredPartIndex(int32 NewPartIndex)
{
	if (HoveredPartIndex == NewPartIndex)
	{
		return;
	}

	ApplyOutlineForPart(HoveredPartIndex, false);
	HoveredPartIndex = NewPartIndex;
	ApplyOutlineForPart(HoveredPartIndex, !bHoverOutlineSuppressed);
	BroadcastPartPrompt();
}

bool UCursorPartFocusScopeComponent::IsDescriptorPreviewAllowed(const FCursorPartFocusPartDescriptor& Descriptor) const
{
	if (!Descriptor.OwnerActor)
	{
		return false;
	}

	if (Descriptor.RequiredStateTags.IsEmpty())
	{
		return true;
	}

	const FGameplayTagContainer ActiveTags = GetActiveProvidedStateTags();
	return ActiveTags.HasAll(BuildEffectiveRequiredStateTags(Descriptor));
}

FGameplayTagContainer UCursorPartFocusScopeComponent::BuildEffectiveRequiredStateTags(const FCursorPartFocusPartDescriptor& Descriptor) const
{
	FGameplayTagContainer EffectiveTags = Descriptor.RequiredStateTags;
	if (Descriptor.ActionHandler)
	{
		EffectiveTags.AppendTags(Descriptor.ActionHandler->GetRequiredStateTags());
	}
	return EffectiveTags;
}

void UCursorPartFocusScopeComponent::ResolveDescriptorOutlineComponents(FCursorPartFocusPartDescriptor& Descriptor) const
{
	if (!Descriptor.OwnerActor)
	{
		return;
	}

	if (Descriptor.OutlineComponentTags.Num() <= 0)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Descriptor.OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		for (const FName& Tag : Descriptor.OutlineComponentTags)
		{
			if (!Tag.IsNone() && PrimitiveComponent->ComponentHasTag(Tag))
			{
				Descriptor.OutlineComponents.AddUnique(PrimitiveComponent);
				break;
			}
		}
	}
}

void UCursorPartFocusScopeComponent::ApplyOutlineForPart(int32 PartIndex, bool bEnabled)
{
	if (!bUseCustomDepthOutline || !RegisteredParts.IsValidIndex(PartIndex))
	{
		return;
	}

	const FCursorPartFocusPartDescriptor& Descriptor = RegisteredParts[PartIndex];
	for (UPrimitiveComponent* PrimitiveComponent : Descriptor.OutlineComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetRenderCustomDepth(bEnabled);
		if (bEnabled)
		{
			PrimitiveComponent->SetCustomDepthStencilValue(CustomDepthStencilValue);
		}
	}
}

void UCursorPartFocusScopeComponent::BroadcastPartPrompt()
{
	FCursorPartFocusPromptData PromptData;
	if (!bHoverOutlineSuppressed && RegisteredParts.IsValidIndex(HoveredPartIndex) && IsDescriptorPreviewAllowed(RegisteredParts[HoveredPartIndex]))
	{
		PromptData = RegisteredParts[HoveredPartIndex].PromptData;
	}

	OnPartFocusPromptChanged.Broadcast(PromptData);

	if (OwnerFocusComponent)
	{
		FFocusPromptData FocusPromptData;
		FocusPromptData.bIsValid = PromptData.bIsValid;
		FocusPromptData.DisplayName = PromptData.DisplayName;
		FocusPromptData.InteractionKeyText = PromptData.InteractionKeyText;
		OwnerFocusComponent->SetEngagedFocusPromptOverride(FocusPromptData);
	}
}

bool UCursorPartFocusScopeComponent::CancelTopActionCascade(bool bAbort)
{
	RemoveInactiveActions();
	if (ActivePartActions.Num() <= 0)
	{
		return false;
	}

	TSet<TObjectPtr<UCursorPartFocusActionComponent>> Visited;
	return CancelActionCascade(ActivePartActions.Last(), bAbort, Visited);
}

bool UCursorPartFocusScopeComponent::CancelActionCascade(UCursorPartFocusActionComponent* Action, bool bAbort, TSet<TObjectPtr<UCursorPartFocusActionComponent>>& Visited)
{
	if (!Action || Visited.Contains(Action))
	{
		return false;
	}

	Visited.Add(Action);

	const FGameplayTagContainer ProvidedTags = Action->GetProvidedStateTags();
	for (int32 Index = ActivePartActions.Num() - 1; Index >= 0; --Index)
	{
		UCursorPartFocusActionComponent* Candidate = ActivePartActions[Index];
		if (!Candidate || Candidate == Action)
		{
			continue;
		}

		if (ProvidedTags.Num() > 0 && Candidate->GetRequiredStateTags().HasAny(ProvidedTags))
		{
			CancelActionCascade(Candidate, bAbort, Visited);
		}
	}

	const bool bCanceled = bAbort ? true : Action->CancelPartFocusAction(this, OwnerCharacter);
	if (bAbort)
	{
		Action->AbortPartFocusAction(this, OwnerCharacter);
	}

	ActivePartActions.Remove(Action);
	return bCanceled || bAbort;
}

void UCursorPartFocusScopeComponent::RemoveInactiveActions()
{
	for (int32 Index = ActivePartActions.Num() - 1; Index >= 0; --Index)
	{
		UCursorPartFocusActionComponent* Action = ActivePartActions[Index];
		if (!Action || !Action->IsPartActionEngaged())
		{
			ActivePartActions.RemoveAt(Index);
		}
	}
}

bool UCursorPartFocusScopeComponent::BeginPartActionForDescriptor(const FCursorPartFocusPartDescriptor& Descriptor)
{
	UCursorPartFocusActionComponent* Action = Descriptor.ActionHandler;
	if (!Action || !OwnerCharacter)
	{
		return true;
	}

	if (!Action->CanBeginPartFocusAction(this, OwnerCharacter))
	{
		return true;
	}

	const FGameplayTagContainer ActiveTags = GetActiveProvidedStateTags();
	const FGameplayTagContainer RequiredTags = BuildEffectiveRequiredStateTags(Descriptor);
	if (!ActiveTags.HasAll(RequiredTags))
	{
		return true;
	}

	const FGameplayTag ExclusiveGroup = Action->GetExclusiveGroup();
	if (ExclusiveGroup.IsValid())
	{
		for (int32 Index = ActivePartActions.Num() - 1; Index >= 0; --Index)
		{
			UCursorPartFocusActionComponent* ExistingAction = ActivePartActions[Index];
			if (!ExistingAction || !ExistingAction->IsPartActionEngaged())
			{
				continue;
			}

			if (ExistingAction->GetExclusiveGroup().MatchesTagExact(ExclusiveGroup))
			{
				TSet<TObjectPtr<UCursorPartFocusActionComponent>> Visited;
				CancelActionCascade(ExistingAction, false, Visited);
			}
		}
	}

	if (!Action->BeginPartFocusAction(this, OwnerCharacter))
	{
		return true;
	}

	if (Action->IsPartActionEngaged())
	{
		ActivePartActions.AddUnique(Action);
	}

	return true;
}

bool UCursorPartFocusScopeComponent::HandleEdgeCancelClick() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
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

	int32 SizeX = 0;
	int32 SizeY = 0;
	PlayerController->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0)
	{
		return false;
	}

	const UBeekeepingSimFocusSettings* FocusSettings = GetDefault<UBeekeepingSimFocusSettings>();
	const float RawThickness = FocusSettings ? FocusSettings->ScreenEdgeCancelRegionThickness : 64.0f;
	const float T = FMath::Max(0.0f, RawThickness);
	return ScreenX <= T || ScreenY <= T || ScreenX >= (static_cast<float>(SizeX) - T) || ScreenY >= (static_cast<float>(SizeY) - T);
}

void UCursorPartFocusScopeComponent::RequestHostFocusCancel() const
{
	if (OwnerFocusComponent)
	{
		OwnerFocusComponent->CancelFocus();
	}
}
