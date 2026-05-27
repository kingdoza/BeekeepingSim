#include "WorldActors/BeehiveCombPartFocusActionComponent.h"

#include "Focus/CursorPartFocusScopeComponent.h"
#include "GameplayTagsManager.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"

UBeehiveCombPartFocusActionComponent::UBeehiveCombPartFocusActionComponent()
{
	SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
	FGameplayTagContainer RequiredTags;
	RequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.LidOpen")), false));
	SetRequiredStateTags(RequiredTags);
	SetExclusiveGroup(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.CombLift")), false));
}

bool UBeehiveCombPartFocusActionComponent::CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const
{
	const ABeehiveCombActor* CombActor = ResolveOwnerCombActor();
	const UPlacementSlotRetrievePartFocusActionComponent* RetrieveAction = CombActor ? CombActor->GetPlacementRetrieveActionComponent() : nullptr;
	return RetrieveAction && RetrieveAction->CanRetrievePlacementOccupant(InteractingCharacter);
}

bool UBeehiveCombPartFocusActionComponent::HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	ABeehiveCombActor* CombActor = ResolveOwnerCombActor();
	UPlacementSlotRetrievePartFocusActionComponent* RetrieveAction = CombActor ? CombActor->GetPlacementRetrieveActionComponent() : nullptr;
	if (!CombActor || !RetrieveAction)
	{
		return false;
	}

	UItemInstance* AcquiredItemInstance = nullptr;
	AActor* SlotActor = nullptr;
	if (!RetrieveAction->TryRetrievePlacementOccupant(InteractingCharacter, AcquiredItemInstance, SlotActor))
	{
		return false;
	}

	CombActor->WriteStateToItemInstance(AcquiredItemInstance);

	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return false;
	}

	IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
	return true;
}

bool UBeehiveCombPartFocusActionComponent::CanBeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const
{
	if (!ScopeComponent || !InteractingCharacter || !IsPartActionEngaged())
	{
		return false;
	}

	return ResolveOwnerCombActor() != nullptr;
}

bool UBeehiveCombPartFocusActionComponent::BeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	if (!CanBeginPartFocusDrag(ScopeComponent, InteractingCharacter))
	{
		return false;
	}

	ResetDragInterpretationState();
	SetPartFocusDragInProgress(true);
	return true;
}

void UBeehiveCombPartFocusActionComponent::UpdatePartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, float DeltaTime)
{
	if (!IsPartFocusDragInProgress() || !ScopeComponent)
	{
		return;
	}

	ABeehiveCombActor* CombActor = ResolveOwnerCombActor();
	if (!CombActor)
	{
		return;
	}

	const FVector2D DeltaFromPress = ScopeComponent->GetPartFocusDragDeltaFromPress();
	const FVector2D DeltaSinceLastUpdate = ScopeComponent->GetPartFocusDragDeltaSinceLastUpdate();

	if (ActiveDragMode == EBeehiveCombDragMode::None)
	{
		const float AbsX = FMath::Abs(DeltaFromPress.X);
		const float AbsY = FMath::Abs(DeltaFromPress.Y);
		const float SafeHorizontalDominance = FMath::Max(0.0f, HorizontalDominanceRatio);
		const float SafeVerticalDominance = FMath::Max(0.0f, VerticalDominanceRatio);

		const bool bFlipQualified = AbsX >= FMath::Max(0.0f, CombFlipDragThresholdPixels)
			&& AbsX > (AbsY * SafeHorizontalDominance);
		const bool bShakeQualified = AbsY >= FMath::Max(0.0f, CombShakeStrokeThresholdPixels)
			&& AbsY > (AbsX * SafeVerticalDominance);

		if (bFlipQualified)
		{
			ActiveDragMode = EBeehiveCombDragMode::Flip;
			if (!bFlipExecutedThisDrag)
			{
				const EBeehiveCombFlipDirection FlipDirection = (DeltaFromPress.X >= 0.0f)
					? EBeehiveCombFlipDirection::Right
					: EBeehiveCombFlipDirection::Left;
				CombActor->FlipCombFaceWithDirection(FlipDirection);
				bFlipExecutedThisDrag = true;
			}
			return;
		}

		if (bShakeQualified)
		{
			ActiveDragMode = EBeehiveCombDragMode::Shake;
		}
	}

	if (ActiveDragMode == EBeehiveCombDragMode::Shake)
	{
		UpdateShakeMode(DeltaSinceLastUpdate, CombActor);
	}
}

bool UBeehiveCombPartFocusActionComponent::EndPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, bool bCanceled)
{
	SetPartFocusDragInProgress(false);
	ResetDragInterpretationState();
	return true;
}

void UBeehiveCombPartFocusActionComponent::ResetDragInterpretationState()
{
	ActiveDragMode = EBeehiveCombDragMode::None;
	AccumulatedShakeDistanceInCurrentDirection = 0.0f;
	LastShakeDirectionSign = 0.0f;
	ShakeStrokeCount = 0;
	bFlipExecutedThisDrag = false;
	bShakeExecutedThisDrag = false;
}

ABeehiveCombActor* UBeehiveCombPartFocusActionComponent::ResolveOwnerCombActor() const
{
	return Cast<ABeehiveCombActor>(GetOwner());
}

void UBeehiveCombPartFocusActionComponent::UpdateShakeMode(const FVector2D& DeltaSinceLastUpdate, ABeehiveCombActor* CombActor)
{
	if (!CombActor || bShakeExecutedThisDrag)
	{
		return;
	}

	const float DeltaY = DeltaSinceLastUpdate.Y;
	const float CurrentDirectionSign = (DeltaY > 0.0f) ? 1.0f : ((DeltaY < 0.0f) ? -1.0f : 0.0f);
	if (CurrentDirectionSign == 0.0f)
	{
		return;
	}

	if (LastShakeDirectionSign == 0.0f)
	{
		LastShakeDirectionSign = CurrentDirectionSign;
	}

	if (CurrentDirectionSign != LastShakeDirectionSign)
	{
		if (AccumulatedShakeDistanceInCurrentDirection >= FMath::Max(0.0f, CombShakeStrokeThresholdPixels))
		{
			++ShakeStrokeCount;
			if (ShakeStrokeCount >= FMath::Max(1, RequiredShakeStrokeCount))
			{
				CombActor->ApplyCombShakeByRatioWithStrokeCount(ShakeBeeReductionRatio, ShakeStrokeCount);
				bShakeExecutedThisDrag = true;
				return;
			}
		}

		AccumulatedShakeDistanceInCurrentDirection = 0.0f;
		LastShakeDirectionSign = CurrentDirectionSign;
	}

	AccumulatedShakeDistanceInCurrentDirection += FMath::Abs(DeltaY);
}
