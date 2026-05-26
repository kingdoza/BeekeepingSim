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
	UpdatePartPointerGestureState(DeltaTime);
	UpdatePartDrag(DeltaTime);
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
	if (bPartDragInProgress)
	{
		EndPartDrag(true);
	}

	if (bAbortActiveActions)
	{
		while (CancelTopActionCascade(true))
		{
		}
	}

	SetHoveredPartIndex(INDEX_NONE);
	bHoverOutlineSuppressed = false;
	bIsScopeActive = false;
	ResetPartPointerGestureState();
	SetComponentTickEnabled(false);
	BroadcastPartPrompt();
}

bool UCursorPartFocusScopeComponent::HandleConfirmInput()
{
	return HandlePartFocusClickInput();
}

bool UCursorPartFocusScopeComponent::HandlePartFocusClickInput()
{
	UpdateHoveredPartFromCursor();
	if (RegisteredParts.IsValidIndex(HoveredPartIndex))
	{
		return ExecutePartClickForDescriptor(RegisteredParts[HoveredPartIndex]);
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

bool UCursorPartFocusScopeComponent::HandlePartFocusPointerPressed()
{
	if (!bIsScopeActive)
	{
		return false;
	}

	ResetPartPointerGestureState();
	UpdateHoveredPartFromCursor();
	if (RegisteredParts.IsValidIndex(HoveredPartIndex) && IsDescriptorPreviewAllowed(RegisteredParts[HoveredPartIndex]))
	{
		const FCursorPartFocusPartDescriptor& Descriptor = RegisteredParts[HoveredPartIndex];
		PressedPartIndex = HoveredPartIndex;
		PressedPartId = Descriptor.PartId;
		PressedPartAction = Descriptor.ActionHandler;
		bIsPartPrimaryPointerDown = true;
		bPressedInEdgeCancelRegion = false;
		if (TryGetMouseScreenPosition(PressedPartScreenPosition))
		{
			CurrentPartScreenPosition = PressedPartScreenPosition;
			PreviousPartScreenPosition = PressedPartScreenPosition;
			CachedPartDragDeltaFromPress = FVector2D::ZeroVector;
			CachedPartDragDeltaSinceLastUpdate = FVector2D::ZeroVector;
			bPressedInEdgeCancelRegion = IsMouseInEdgeCancelRegion(PressedPartScreenPosition);
		}
		return true;
	}

	bIsPartPrimaryPointerDown = true;
	TryGetMouseScreenPosition(PressedPartScreenPosition);
	CurrentPartScreenPosition = PressedPartScreenPosition;
	PreviousPartScreenPosition = PressedPartScreenPosition;
	CachedPartDragDeltaFromPress = FVector2D::ZeroVector;
	CachedPartDragDeltaSinceLastUpdate = FVector2D::ZeroVector;
	bPressedInEdgeCancelRegion = IsMouseInEdgeCancelRegion(PressedPartScreenPosition);
	return true;
}

bool UCursorPartFocusScopeComponent::HandlePartFocusPointerReleased()
{
	if (!bIsScopeActive)
	{
		return false;
	}

	const bool bWasPointerDown = bIsPartPrimaryPointerDown;
	bool bDragWasInProgress = bPartDragInProgress;
	const FName CapturedPartId = PressedPartId;
	const TObjectPtr<UCursorPartFocusActionComponent> CapturedPartAction = PressedPartAction;
	const bool bPressedAtEdgeCancel = bPressedInEdgeCancelRegion;
	float MaxMoveDistance = MaxPartPointerMoveDistanceSincePress;

	if (!bWasPointerDown)
	{
		ResetPartPointerGestureState();
		return false;
	}

	FVector2D ReleaseScreenPosition = FVector2D::ZeroVector;
	const bool bHasReleaseMousePosition = TryGetMouseScreenPosition(ReleaseScreenPosition);
	if (bHasReleaseMousePosition)
	{
		PreviousPartScreenPosition = CurrentPartScreenPosition;
		CurrentPartScreenPosition = ReleaseScreenPosition;
		CachedPartDragDeltaFromPress = CurrentPartScreenPosition - PressedPartScreenPosition;
		CachedPartDragDeltaSinceLastUpdate = CurrentPartScreenPosition - PreviousPartScreenPosition;

		const float ReleaseMoveDistance = FVector2D::Distance(ReleaseScreenPosition, PressedPartScreenPosition);
		MaxMoveDistance = FMath::Max(MaxMoveDistance, ReleaseMoveDistance);
	}

	const UBeekeepingSimFocusSettings* FocusSettings = GetDefault<UBeekeepingSimFocusSettings>();
	const float Threshold = FocusSettings ? FMath::Max(0.0f, FocusSettings->ClickCancelThresholdPixels) : 12.0f;
	const bool bExceededThreshold = MaxMoveDistance > Threshold;
	const bool bReleasedAtEdgeCancel = bHasReleaseMousePosition && IsMouseInEdgeCancelRegion(ReleaseScreenPosition);

	if (!bDragWasInProgress && bExceededThreshold)
	{
		bPartClickCanceledByMovement = true;
		if (TryBeginPartDrag())
		{
			bDragWasInProgress = true;
		}
	}

	if (bDragWasInProgress)
	{
		if (PressedPartAction && OwnerCharacter)
		{
			PressedPartAction->UpdatePartFocusDrag(this, OwnerCharacter, 0.0f);
		}

		EndPartDrag(false);
		ResetPartPointerGestureState();
		return true;
	}

	const bool bClickCanceled = bPartClickCanceledByMovement || bExceededThreshold;
	ResetPartPointerGestureState();

	if (bClickCanceled)
	{
		return true;
	}

	UpdateHoveredPartFromCursor();
	if (RegisteredParts.IsValidIndex(HoveredPartIndex))
	{
		const FCursorPartFocusPartDescriptor& ReleaseDescriptor = RegisteredParts[HoveredPartIndex];
		if (IsDescriptorPreviewAllowed(ReleaseDescriptor))
		{
			const bool bPartIdMatched = !CapturedPartId.IsNone() && CapturedPartId == ReleaseDescriptor.PartId;
			const bool bActionMatched = CapturedPartAction == nullptr || CapturedPartAction == ReleaseDescriptor.ActionHandler;
			if (bPartIdMatched && bActionMatched)
			{
				return ExecutePartClickForDescriptor(ReleaseDescriptor);
			}
		}
	}

	if (bPressedAtEdgeCancel && bReleasedAtEdgeCancel)
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

bool UCursorPartFocusScopeComponent::HandleSecondaryInput()
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
		return false;
	}

	UCursorPartFocusActionComponent* Action = Descriptor.ActionHandler;
	if (!Action || !OwnerCharacter)
	{
		return false;
	}

	if (!Action->CanHandleSecondaryPartFocusAction(this, OwnerCharacter))
	{
		return false;
	}

	return Action->HandleSecondaryPartFocusAction(this, OwnerCharacter);
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

	return IsMouseInEdgeCancelRegion(FVector2D(ScreenX, ScreenY));
}

void UCursorPartFocusScopeComponent::RequestHostFocusCancel() const
{
	if (OwnerFocusComponent)
	{
		OwnerFocusComponent->CancelFocus();
	}
}

bool UCursorPartFocusScopeComponent::ExecutePartClickForDescriptor(const FCursorPartFocusPartDescriptor& Descriptor)
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

void UCursorPartFocusScopeComponent::ResetPartPointerGestureState()
{
	PressedPartId = NAME_None;
	PressedPartIndex = INDEX_NONE;
	PressedPartAction = nullptr;
	PressedPartScreenPosition = FVector2D::ZeroVector;
	CurrentPartScreenPosition = FVector2D::ZeroVector;
	PreviousPartScreenPosition = FVector2D::ZeroVector;
	CachedPartDragDeltaFromPress = FVector2D::ZeroVector;
	CachedPartDragDeltaSinceLastUpdate = FVector2D::ZeroVector;
	MaxPartPointerMoveDistanceSincePress = 0.0f;
	bIsPartPrimaryPointerDown = false;
	bPartClickCanceledByMovement = false;
	bPartDragInProgress = false;
	bPressedInEdgeCancelRegion = false;
}

bool UCursorPartFocusScopeComponent::TryGetMouseScreenPosition(FVector2D& OutPosition) const
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

void UCursorPartFocusScopeComponent::UpdatePartPointerGestureState(float DeltaTime)
{
	if (!bIsPartPrimaryPointerDown)
	{
		return;
	}

	FVector2D CurrentMousePosition = FVector2D::ZeroVector;
	if (TryGetMouseScreenPosition(CurrentMousePosition))
	{
		PreviousPartScreenPosition = CurrentPartScreenPosition;
		CurrentPartScreenPosition = CurrentMousePosition;
		CachedPartDragDeltaFromPress = CurrentPartScreenPosition - PressedPartScreenPosition;
		CachedPartDragDeltaSinceLastUpdate = CurrentPartScreenPosition - PreviousPartScreenPosition;

		const float MoveDistance = FVector2D::Distance(CurrentMousePosition, PressedPartScreenPosition);
		MaxPartPointerMoveDistanceSincePress = FMath::Max(MaxPartPointerMoveDistanceSincePress, MoveDistance);
	}

	const UBeekeepingSimFocusSettings* FocusSettings = GetDefault<UBeekeepingSimFocusSettings>();
	const float Threshold = FocusSettings ? FMath::Max(0.0f, FocusSettings->ClickCancelThresholdPixels) : 12.0f;
	if (bPartDragInProgress || MaxPartPointerMoveDistanceSincePress <= Threshold)
	{
		return;
	}

	bPartClickCanceledByMovement = true;
	TryBeginPartDrag();
}

bool UCursorPartFocusScopeComponent::TryBeginPartDrag()
{
	if (bPartDragInProgress || !bIsPartPrimaryPointerDown || !OwnerCharacter || !PressedPartAction)
	{
		return false;
	}

	if (!PressedPartAction->CanBeginPartFocusDrag(this, OwnerCharacter))
	{
		return false;
	}

	if (!PressedPartAction->BeginPartFocusDrag(this, OwnerCharacter))
	{
		PressedPartAction->SetPartFocusDragInProgress(false);
		return false;
	}

	PressedPartAction->SetPartFocusDragInProgress(true);
	bPartDragInProgress = true;
	return true;
}

void UCursorPartFocusScopeComponent::UpdatePartDrag(float DeltaTime)
{
	if (!bPartDragInProgress || !PressedPartAction || !OwnerCharacter)
	{
		return;
	}

	PressedPartAction->UpdatePartFocusDrag(this, OwnerCharacter, DeltaTime);
}

void UCursorPartFocusScopeComponent::EndPartDrag(bool bCanceled)
{
	if (!PressedPartAction || !OwnerCharacter)
	{
		bPartDragInProgress = false;
		return;
	}

	PressedPartAction->EndPartFocusDrag(this, OwnerCharacter, bCanceled);
	PressedPartAction->SetPartFocusDragInProgress(false);
	bPartDragInProgress = false;
}

bool UCursorPartFocusScopeComponent::IsPartFocusDragInProgress() const
{
	return bPartDragInProgress;
}

FVector2D UCursorPartFocusScopeComponent::GetPartFocusDragDeltaFromPress() const
{
	return CachedPartDragDeltaFromPress;
}

FVector2D UCursorPartFocusScopeComponent::GetPartFocusDragDeltaSinceLastUpdate() const
{
	return CachedPartDragDeltaSinceLastUpdate;
}

bool UCursorPartFocusScopeComponent::IsMouseInEdgeCancelRegion(const FVector2D& ScreenPosition) const
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
	return ScreenPosition.X <= T
		|| ScreenPosition.Y <= T
		|| ScreenPosition.X >= (static_cast<float>(SizeX) - T)
		|| ScreenPosition.Y >= (static_cast<float>(SizeY) - T);
}
