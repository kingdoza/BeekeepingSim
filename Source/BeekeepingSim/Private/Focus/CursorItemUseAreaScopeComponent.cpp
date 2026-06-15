#include "Focus/CursorItemUseAreaScopeComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Focus/BeekeeperFocusComponent.h"
#include "Focus/CursorPartFocusScopeComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Focus/ItemUseAreaMeshProviderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/ActiveUseDurabilityItemDefinition.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/HoldItemUseAction.h"
#include "Inventory/ItemActionContext.h"
#include "Inventory/ItemActionTypes.h"
#include "Inventory/ItemInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WorldActors/ItemPlacementSlot.h"

UCursorItemUseAreaScopeComponent::UCursorItemUseAreaScopeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCursorItemUseAreaScopeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCursorItemUseAreaScopeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateItemUseAreaScope(true);
	Super::EndPlay(EndPlayReason);
}

void UCursorItemUseAreaScopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsScopeActive)
	{
		return;
	}

	RefreshSelectedItemAndAction();
	UpdateHoveredDescriptorFromCursor();

	if (bIsUseInProgress && CachedHoldAction)
	{
		const FItemActionContext TickContext = BuildItemActionContext(HoveredDescriptorIndex);
		CachedHoldAction->TickUse(TickContext, DeltaTime);

		FItemActionExecutionResult Result;
		FItemActionContext DurabilityContext = BuildItemActionContext(INDEX_NONE);
		bool bIsOverValidUseArea = false;

		if (ActiveDescriptorIndices.Contains(HoveredDescriptorIndex))
		{
			const FItemActionContext EffectContext = BuildItemActionContext(HoveredDescriptorIndex);
			if (CachedHoldAction->CanApplyUseEffect(EffectContext))
			{
				bIsOverValidUseArea = true;
				DurabilityContext = EffectContext;
				Result = CachedHoldAction->ApplyUseEffect(EffectContext, DeltaTime);
			}
		}

		if (!bIsScopeActive || !bIsUseInProgress || !CachedHoldAction)
		{
			return;
		}

		Result.DurabilityDelta += CachedHoldAction->ResolveActiveUseDurabilityDelta(
			DurabilityContext,
			Result,
			DeltaTime,
			bIsOverValidUseArea);

		if (Result.bConsumedItem || !FMath::IsNearlyZero(Result.DurabilityDelta))
		{
			ApplyUseEffectResultToSelectedItem(Result);
		}
	}
}

void UCursorItemUseAreaScopeComponent::ActivateItemUseAreaScope(ABeekeeperCharacter* InteractingCharacter)
{
	OwnerCharacter = InteractingCharacter ? InteractingCharacter : Cast<ABeekeeperCharacter>(GetOwner());
	OwnerHotbarComponent = OwnerCharacter ? OwnerCharacter->GetBeekeeperHotbar() : nullptr;
	OwnerFocusComponent = OwnerCharacter ? OwnerCharacter->GetBeekeeperFocus() : nullptr;
	SiblingPartFocusScopeComponent = GetOwner() ? GetOwner()->FindComponentByClass<UCursorPartFocusScopeComponent>() : nullptr;
	ActiveHostActor = ResolveActiveHostActor();

	if (OwnerHotbarComponent)
	{
		OwnerHotbarComponent->OnHotbarChanged.RemoveDynamic(this, &UCursorItemUseAreaScopeComponent::HandleHotbarChanged);
		OwnerHotbarComponent->OnHotbarChanged.AddDynamic(this, &UCursorItemUseAreaScopeComponent::HandleHotbarChanged);
	}

	RebuildItemUseAreaDescriptors();
	RefreshSelectedItemAndAction();
	bIsScopeActive = (OwnerCharacter != nullptr);
	SetComponentTickEnabled(bIsScopeActive);
	UpdatePartFocusOutlineSuppression();
}

void UCursorItemUseAreaScopeComponent::DeactivateItemUseAreaScope(bool bCancelActiveUse)
{
	if (bCancelActiveUse)
	{
		EndUseSession(true);
	}

	if (OwnerHotbarComponent)
	{
		OwnerHotbarComponent->OnHotbarChanged.RemoveDynamic(this, &UCursorItemUseAreaScopeComponent::HandleHotbarChanged);
	}

	ClearAllVisualState();
	RestoreOriginalCollisionStates();
	RegisteredDescriptors.Reset();
	ActiveDescriptorIndices.Reset();
	DynamicMaterials.Reset();
	HoveredDescriptorIndex = INDEX_NONE;
	ClearHoveredItemUseAreaHit();
	CachedSelectedItemInstance = nullptr;
	CachedHoldAction = nullptr;
	ActiveHostActor = nullptr;
	UpdatePartFocusOutlineSuppression();
	SiblingPartFocusScopeComponent = nullptr;
	bIsScopeActive = false;
	SetComponentTickEnabled(false);
}

void UCursorItemUseAreaScopeComponent::RebuildItemUseAreaDescriptors()
{
	ClearAllVisualState();
	RestoreOriginalCollisionStates();
	DynamicMaterials.Reset();
	RegisteredDescriptors.Reset();
	ActiveDescriptorIndices.Reset();
	HoveredDescriptorIndex = INDEX_NONE;
	ClearHoveredItemUseAreaHit();

	AActor* HostActor = ResolveActiveHostActor();
	ActiveHostActor = HostActor;
	if (!HostActor)
	{
		return;
	}

	RebuildDescriptorsFromItemUseAreaMeshProviders(HostActor);
	RefreshActiveUseAreas();
}

void UCursorItemUseAreaScopeComponent::RegisterItemUseAreaDescriptor(const FItemUseAreaDescriptor& Descriptor)
{
	if (Descriptor.AreaId.IsNone())
	{
		return;
	}

	if (!Descriptor.OwnerActor)
	{
		return;
	}

	if (!Descriptor.HitComponent && Descriptor.VisualComponents.Num() <= 0)
	{
		return;
	}

	RegisteredDescriptors.Add(Descriptor);
}

bool UCursorItemUseAreaScopeComponent::HandleItemUsePressed()
{
	if (!bIsScopeActive)
	{
		return false;
	}

	RefreshSelectedItemAndAction();
	UpdateHoveredDescriptorFromCursor();
	if (!CachedSelectedItemInstance || !CachedHoldAction)
	{
		return false;
	}

	const FItemActionContext Context = BuildItemActionContext(HoveredDescriptorIndex);
	if (!CachedHoldAction->CanBeginUse(Context))
	{
		return false;
	}

	bIsUseInProgress = CachedHoldAction->BeginUse(Context);
	return true;
}

bool UCursorItemUseAreaScopeComponent::HandleItemUseReleased()
{
	if (!bIsUseInProgress)
	{
		return false;
	}

	EndUseSession(false);
	return true;
}

bool UCursorItemUseAreaScopeComponent::HandleItemUseCanceled()
{
	if (!bIsUseInProgress)
	{
		return false;
	}

	EndUseSession(true);
	return true;
}

void UCursorItemUseAreaScopeComponent::HandleHotbarChanged()
{
	RefreshSelectedItemAndAction();
}

AActor* UCursorItemUseAreaScopeComponent::ResolveActiveHostActor() const
{
	if (OwnerFocusComponent && OwnerFocusComponent->IsFocusEngaged() && OwnerFocusComponent->GetEngagedFocusTarget())
	{
		return OwnerFocusComponent->GetEngagedFocusTarget()->GetOwner();
	}

	return GetOwner();
}

void UCursorItemUseAreaScopeComponent::RebuildDescriptorsFromItemUseAreaMeshProviders(AActor* HostActor)
{
	if (!HostActor)
	{
		return;
	}

	TInlineComponentArray<UItemUseAreaMeshProviderComponent*> Providers(HostActor);
	for (UItemUseAreaMeshProviderComponent* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}

		TArray<FItemUseAreaDescriptor> ProviderDescriptors;
		Provider->BuildItemUseAreaDescriptors(ProviderDescriptors);
		for (const FItemUseAreaDescriptor& Descriptor : ProviderDescriptors)
		{
			RegisterItemUseAreaDescriptor(Descriptor);
		}
	}
}

void UCursorItemUseAreaScopeComponent::RefreshSelectedItemAndAction()
{
	UItemInstance* NewSelectedItem = OwnerHotbarComponent ? OwnerHotbarComponent->GetSelectedItemInstance() : nullptr;
	UHoldItemUseAction* NewHoldAction = NewSelectedItem ? NewSelectedItem->FindHoldItemUseAction() : nullptr;

	if (CachedSelectedItemInstance == NewSelectedItem && CachedHoldAction == NewHoldAction)
	{
		return;
	}

	if (bIsUseInProgress)
	{
		EndUseSession(true);
	}

	CachedSelectedItemInstance = NewSelectedItem;
	CachedHoldAction = NewHoldAction;
	RefreshActiveUseAreas();
	UpdatePartFocusOutlineSuppression();
}

void UCursorItemUseAreaScopeComponent::RefreshActiveUseAreas()
{
	ActiveDescriptorIndices.Reset();
	SetHoveredDescriptorIndex(INDEX_NONE);
	ClearHoveredItemUseAreaHit();

	if (!CachedSelectedItemInstance || !CachedHoldAction)
	{
		ApplyVisualStateForAllDescriptors();
		return;
	}

	for (int32 Index = 0; Index < RegisteredDescriptors.Num(); ++Index)
	{
		if (DoesDescriptorMatchActionQuery(RegisteredDescriptors[Index], CachedHoldAction))
		{
			ActiveDescriptorIndices.Add(Index);
		}
	}

	ApplyVisualStateForAllDescriptors();
	UpdateHoveredDescriptorFromCursor();
}

void UCursorItemUseAreaScopeComponent::UpdateHoveredDescriptorFromCursor()
{
	const FResolvedItemUseAreaHit ResolvedHit = ResolveHoveredActiveDescriptor();
	if (ResolvedHit.bHasHit && RegisteredDescriptors.IsValidIndex(ResolvedHit.DescriptorIndex))
	{
		HoveredItemUseAreaHit = ResolvedHit.HitResult;
		bHasHoveredItemUseAreaHit = true;
	}
	else
	{
		ClearHoveredItemUseAreaHit();
	}

	SetHoveredDescriptorIndex(ResolvedHit.DescriptorIndex);
}

UCursorItemUseAreaScopeComponent::FResolvedItemUseAreaHit UCursorItemUseAreaScopeComponent::ResolveHoveredActiveDescriptor() const
{
	FResolvedItemUseAreaHit Result;
	if (!OwnerCharacter || ActiveDescriptorIndices.Num() <= 0)
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
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CursorItemUseAreaTrace), true, OwnerCharacter);
	if (!GetWorld() || !GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CursorTraceChannel, QueryParams))
	{
		return Result;
	}

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	if (!HitComponent)
	{
		return Result;
	}

	for (int32 DescriptorIndex : ActiveDescriptorIndices)
	{
		if (!RegisteredDescriptors.IsValidIndex(DescriptorIndex))
		{
			continue;
		}

		if (RegisteredDescriptors[DescriptorIndex].HitComponent == HitComponent)
		{
			Result.DescriptorIndex = DescriptorIndex;
			Result.HitResult = HitResult;
			Result.bHasHit = true;
			return Result;
		}
	}

	return Result;
}

void UCursorItemUseAreaScopeComponent::SetHoveredDescriptorIndex(int32 NewIndex)
{
	if (HoveredDescriptorIndex == NewIndex)
	{
		return;
	}

	HoveredDescriptorIndex = NewIndex;
	ApplyVisualStateForAllDescriptors();
}

void UCursorItemUseAreaScopeComponent::ClearHoveredItemUseAreaHit()
{
	bHasHoveredItemUseAreaHit = false;
	HoveredItemUseAreaHit = FHitResult();
}

void UCursorItemUseAreaScopeComponent::ApplyVisualStateForDescriptor(int32 DescriptorIndex, bool bDescriptorActive, bool bIsHovered)
{
	if (!RegisteredDescriptors.IsValidIndex(DescriptorIndex))
	{
		return;
	}

	const FItemUseAreaDescriptor& Descriptor = RegisteredDescriptors[DescriptorIndex];
	const float EffectiveOpacity = bDescriptorActive ? Descriptor.VisualSettings.UseAreaOpacity : 0.0f;
	const float HoverStrengthValue = bIsHovered ? Descriptor.VisualSettings.HoverStrength : 0.0f;
	for (UPrimitiveComponent* VisualComponent : Descriptor.VisualComponents)
	{
		if (!VisualComponent)
		{
			continue;
		}

		UMaterialInstanceDynamic* MID = ResolveOrCreateMID(VisualComponent);
		if (!MID)
		{
			continue;
		}

		MID->SetVectorParameterValue(TEXT("UseAreaColor"), Descriptor.VisualSettings.UseAreaColor);
		MID->SetScalarParameterValue(TEXT("UseAreaOpacity"), EffectiveOpacity);
		MID->SetScalarParameterValue(TEXT("PulseSpeed"), Descriptor.VisualSettings.PulseSpeed);
		MID->SetScalarParameterValue(TEXT("HoverStrength"), HoverStrengthValue);
	}
}

void UCursorItemUseAreaScopeComponent::ApplyVisualStateForAllDescriptors()
{
	for (int32 Index = 0; Index < RegisteredDescriptors.Num(); ++Index)
	{
		const bool bDescriptorActive = ActiveDescriptorIndices.Contains(Index);
		const bool bIsHovered = (HoveredDescriptorIndex == Index);
		ApplyVisualStateForDescriptor(Index, bDescriptorActive, bIsHovered);
	}

	ApplyCollisionStateForAllDescriptors();
}

void UCursorItemUseAreaScopeComponent::ClearAllVisualState()
{
	for (int32 Index = 0; Index < RegisteredDescriptors.Num(); ++Index)
	{
		ApplyVisualStateForDescriptor(Index, false, false);
	}
}

void UCursorItemUseAreaScopeComponent::ApplyCollisionStateForAllDescriptors()
{
	TSet<TWeakObjectPtr<UPrimitiveComponent>> ManagedComponents;
	TSet<TWeakObjectPtr<UPrimitiveComponent>> ActiveHitComponents;

	for (int32 Index = 0; Index < RegisteredDescriptors.Num(); ++Index)
	{
		const FItemUseAreaDescriptor& Descriptor = RegisteredDescriptors[Index];
		if (Descriptor.HitComponent)
		{
			ManagedComponents.Add(Descriptor.HitComponent);
			if (ActiveDescriptorIndices.Contains(Index))
			{
				ActiveHitComponents.Add(Descriptor.HitComponent);
			}
		}

		for (UPrimitiveComponent* VisualComponent : Descriptor.VisualComponents)
		{
			if (VisualComponent)
			{
				ManagedComponents.Add(VisualComponent);
			}
		}
	}

	for (const TWeakObjectPtr<UPrimitiveComponent>& WeakComponent : ManagedComponents)
	{
		UPrimitiveComponent* Component = WeakComponent.Get();
		if (!Component)
		{
			continue;
		}

		CacheOriginalCollisionState(Component);

		if (ActiveHitComponents.Contains(WeakComponent))
		{
			if (Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
			{
				Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			Component->SetCollisionResponseToChannel(CursorTraceChannel, ECR_Block);
		}
		else
		{
			Component->SetCollisionResponseToChannel(CursorTraceChannel, ECR_Ignore);
		}
	}
}

void UCursorItemUseAreaScopeComponent::RestoreOriginalCollisionStates()
{
	for (const TPair<TWeakObjectPtr<UPrimitiveComponent>, FStoredUseAreaCollisionState>& Pair : OriginalCollisionStates)
	{
		UPrimitiveComponent* Component = Pair.Key.Get();
		if (!Component)
		{
			continue;
		}

		Component->SetCollisionEnabled(Pair.Value.CollisionEnabled);
		Component->SetCollisionResponseToChannel(CursorTraceChannel, Pair.Value.CursorTraceResponse);
	}

	OriginalCollisionStates.Reset();
}

void UCursorItemUseAreaScopeComponent::CacheOriginalCollisionState(UPrimitiveComponent* Component)
{
	if (!Component || OriginalCollisionStates.Contains(Component))
	{
		return;
	}

	FStoredUseAreaCollisionState State;
	State.CollisionEnabled = Component->GetCollisionEnabled();
	State.CursorTraceResponse = Component->GetCollisionResponseToChannel(CursorTraceChannel);
	OriginalCollisionStates.Add(Component, State);
}

UMaterialInstanceDynamic* UCursorItemUseAreaScopeComponent::ResolveOrCreateMID(UPrimitiveComponent* Component)
{
	if (!Component)
	{
		return nullptr;
	}

	if (TObjectPtr<UMaterialInstanceDynamic>* FoundMID = DynamicMaterials.Find(Component))
	{
		return FoundMID->Get();
	}

	UMaterialInstanceDynamic* MID = Component->CreateDynamicMaterialInstance(0);
	if (!MID)
	{
		return nullptr;
	}

	DynamicMaterials.Add(Component, MID);
	return MID;
}

bool UCursorItemUseAreaScopeComponent::DoesDescriptorMatchActionQuery(const FItemUseAreaDescriptor& Descriptor, const UHoldItemUseAction* HoldAction) const
{
	if (!HoldAction || Descriptor.AreaTags.IsEmpty())
	{
		return false;
	}

	const FGameplayTagQuery Query = HoldAction->GetUseAreaTagQuery();
	return Query.IsEmpty() || Query.Matches(Descriptor.AreaTags);
}

void UCursorItemUseAreaScopeComponent::EndUseSession(bool bWasCanceled)
{
	if (bIsUseInProgress && CachedHoldAction)
	{
		const FItemActionContext Context = BuildItemActionContext(HoveredDescriptorIndex);
		CachedHoldAction->EndUse(Context, bWasCanceled);
	}

	bIsUseInProgress = false;
}

void UCursorItemUseAreaScopeComponent::UpdatePartFocusOutlineSuppression() const
{
	if (!SiblingPartFocusScopeComponent)
	{
		return;
	}

	const bool bShouldSuppress = bIsScopeActive && CachedSelectedItemInstance != nullptr;
	SiblingPartFocusScopeComponent->SetHoverOutlineSuppressed(bShouldSuppress);
}

FItemActionContext UCursorItemUseAreaScopeComponent::BuildItemActionContext(int32 DescriptorIndex) const
{
	FItemActionContext Context;
	Context.Character = OwnerCharacter;
	Context.PlayerController = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
	Context.World = GetWorld();
	Context.FocusEngagedHostActor = ActiveHostActor;
	Context.FocusTarget = OwnerFocusComponent ? OwnerFocusComponent->GetEngagedFocusTarget() : nullptr;
	Context.SourceItemInstance = CachedSelectedItemInstance;

	if (RegisteredDescriptors.IsValidIndex(DescriptorIndex))
	{
		const FItemUseAreaDescriptor& Descriptor = RegisteredDescriptors[DescriptorIndex];
		Context.ItemUseAreaId = Descriptor.AreaId;
		Context.ItemUseAreaTags = Descriptor.AreaTags;
		Context.ItemUseAreaHitComponent = Descriptor.HitComponent;
		Context.ItemUseEffectTargetObject = Descriptor.EffectTargetObject;

		if (DescriptorIndex == HoveredDescriptorIndex && bHasHoveredItemUseAreaHit)
		{
			Context.bHasItemUseAreaHit = true;
			Context.ItemUseAreaImpactPoint = HoveredItemUseAreaHit.ImpactPoint;
			Context.ItemUseAreaImpactNormal = HoveredItemUseAreaHit.ImpactNormal.IsNearlyZero()
				? HoveredItemUseAreaHit.Normal
				: HoveredItemUseAreaHit.ImpactNormal;
		}
	}

	return Context;
}

void UCursorItemUseAreaScopeComponent::ApplyUseEffectResultToSelectedItem(const FItemActionExecutionResult& Result)
{
	if (!OwnerHotbarComponent)
	{
		return;
	}

	UObject* EffectTargetObject = nullptr;
	if (RegisteredDescriptors.IsValidIndex(HoveredDescriptorIndex))
	{
		EffectTargetObject = RegisteredDescriptors[HoveredDescriptorIndex].EffectTargetObject;
	}

	bool bShouldRefreshSelectionAndDescriptors = false;
	bool bPlacementRollbackExecuted = false;

	if (Result.bConsumedItem)
	{
		const int32 EffectiveStackDelta = (Result.StackDelta != 0) ? Result.StackDelta : -1;
		if (OwnerHotbarComponent->ApplySelectedItemStackDelta(EffectiveStackDelta))
		{
			bShouldRefreshSelectionAndDescriptors = true;
		}
		else if (Result.bSucceeded && EffectTargetObject && EffectTargetObject->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
		{
			IItemPlacementSlot::Execute_ClearPlacedItem(EffectTargetObject);
			bPlacementRollbackExecuted = true;
		}
	}

	if (!FMath::IsNearlyZero(Result.DurabilityDelta))
	{
		UItemInstance* SelectedItemInstance = OwnerHotbarComponent->GetSelectedItemInstance();
		bool bRemoveWhenDepleted = true;
		bool bWillDepleteAfterDelta = false;

		if (SelectedItemInstance)
		{
			if (const UActiveUseDurabilityItemDefinition* ActiveUseDefinition = Cast<UActiveUseDurabilityItemDefinition>(SelectedItemInstance->GetDefinition()))
			{
				bRemoveWhenDepleted = ActiveUseDefinition->bRemoveItemWhenDepleted;
			}

			if (SelectedItemInstance->HasDurability())
			{
				const float PredictedNewDurability = FMath::Clamp(
					SelectedItemInstance->GetCurrentDurability() + Result.DurabilityDelta,
					0.0f,
					FMath::Max(0.0f, SelectedItemInstance->GetMaxDurability()));
				bWillDepleteAfterDelta = (PredictedNewDurability <= 0.0f);
			}
		}

		if (bWillDepleteAfterDelta && bIsUseInProgress)
		{
			EndUseSession(false);
		}

		const FHotbarItemDurabilityMutationResult DurabilityMutationResult =
			OwnerHotbarComponent->ApplySelectedItemDurabilityDelta(Result.DurabilityDelta, bRemoveWhenDepleted);
		if (DurabilityMutationResult.bApplied)
		{
			bShouldRefreshSelectionAndDescriptors = true;
			if (DurabilityMutationResult.bItemDepleted && bIsUseInProgress)
			{
				EndUseSession(false);
			}
		}
	}

	if (bShouldRefreshSelectionAndDescriptors || bPlacementRollbackExecuted)
	{
		RefreshSelectedItemAndAction();
		RebuildItemUseAreaDescriptors();
	}
}
