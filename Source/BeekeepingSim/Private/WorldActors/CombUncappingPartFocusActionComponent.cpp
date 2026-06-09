#include "WorldActors/CombUncappingPartFocusActionComponent.h"

#include "Focus/CursorPartFocusScopeComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"
#include "WorldActors/UncappingTable.h"
#include "WorldActors/UncappingTableCombSlot.h"

UCombUncappingPartFocusActionComponent::UCombUncappingPartFocusActionComponent()
{
	SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
	SetPrimaryPromptActionText(FText::FromString(TEXT("잡기")));
	SetEngagedPrimaryPromptActionText(FText::FromString(TEXT("놓기")));
}

bool UCombUncappingPartFocusActionComponent::CanHandleSecondaryPartFocusAction(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter) const
{
	const ABeehiveCombActor* CombActor = ResolvePlacedCombActor();
	const UPlacementSlotRetrievePartFocusActionComponent* RetrieveAction = CombActor ? CombActor->GetPlacementRetrieveActionComponent() : nullptr;
	return RetrieveAction && RetrieveAction->CanRetrievePlacementOccupantWithInventory(InteractingCharacter);
}

bool UCombUncappingPartFocusActionComponent::HandleSecondaryPartFocusAction(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter)
{
	ABeehiveCombActor* CombActor = ResolvePlacedCombActor();
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
	RebuildOwnerTableDescriptors();
	return true;
}

void UCombUncappingPartFocusActionComponent::AppendPartFocusPromptEntries(
	const FPartFocusPromptBuildContext& Context,
	TArray<FFocusPromptEntry>& OutEntries) const
{
	const ABeehiveCombActor* CombActor = ResolvePlacedCombActor();
	const UPlacementSlotRetrievePartFocusActionComponent* RetrieveAction = CombActor ? CombActor->GetPlacementRetrieveActionComponent() : nullptr;
	if (!RetrieveAction)
	{
		return;
	}

	FFocusPromptEntry Entry;
	Entry.EntryId = FName(TEXT("Retrieve"));
	Entry.KeyText = FText::FromString(TEXT("RMB"));
	Entry.ActionText = FText::FromString(TEXT("획득"));
	Entry.SortPriority = 50;

	FText FailureReason;
	Entry.bEnabled = RetrieveAction->CanRetrievePlacementOccupantWithInventory(Context.InteractingCharacter, &FailureReason);
	if (!Entry.bEnabled)
	{
		Entry.DisabledReason = FailureReason;
	}

	OutEntries.Add(MoveTemp(Entry));
}

bool UCombUncappingPartFocusActionComponent::CanBeginPartFocusDrag(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter) const
{
	return ScopeComponent
		&& InteractingCharacter
		&& IsPartActionEngaged()
		&& ResolvePlacedCombActor() != nullptr;
}

bool UCombUncappingPartFocusActionComponent::BeginPartFocusDrag(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter)
{
	if (!CanBeginPartFocusDrag(ScopeComponent, InteractingCharacter))
	{
		return false;
	}

	ResetDragState();
	SetPartFocusDragInProgress(true);
	return true;
}

void UCombUncappingPartFocusActionComponent::UpdatePartFocusDrag(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter,
	float DeltaTime)
{
	(void)InteractingCharacter;
	(void)DeltaTime;

	if (!IsPartFocusDragInProgress() || !ScopeComponent || bFlipExecutedThisDrag)
	{
		return;
	}

	ABeehiveCombActor* CombActor = ResolvePlacedCombActor();
	if (!CombActor)
	{
		return;
	}

	const FVector2D DeltaFromPress = ScopeComponent->GetPartFocusDragDeltaFromPress();
	const float AbsX = FMath::Abs(DeltaFromPress.X);
	const float AbsY = FMath::Abs(DeltaFromPress.Y);
	const float SafeHorizontalDominance = FMath::Max(0.0f, HorizontalDominanceRatio);

	const bool bFlipQualified = AbsX >= FMath::Max(0.0f, CombFlipDragThresholdPixels)
		&& AbsX > (AbsY * SafeHorizontalDominance);
	if (!bFlipQualified)
	{
		return;
	}

	const EBeehiveCombFlipDirection FlipDirection = (DeltaFromPress.X >= 0.0f)
		? EBeehiveCombFlipDirection::Right
		: EBeehiveCombFlipDirection::Left;
	CombActor->FlipCombFaceWithDirection(FlipDirection);
	bFlipExecutedThisDrag = true;
	RebuildOwnerTableDescriptors();
}

bool UCombUncappingPartFocusActionComponent::EndPartFocusDrag(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter,
	bool bCanceled)
{
	(void)ScopeComponent;
	(void)InteractingCharacter;
	(void)bCanceled;

	SetPartFocusDragInProgress(false);
	ResetDragState();
	return true;
}

void UCombUncappingPartFocusActionComponent::ResetDragState()
{
	bFlipExecutedThisDrag = false;
}

AUncappingTableCombSlot* UCombUncappingPartFocusActionComponent::ResolveOwnerSlot() const
{
	return Cast<AUncappingTableCombSlot>(GetOwner());
}

ABeehiveCombActor* UCombUncappingPartFocusActionComponent::ResolvePlacedCombActor() const
{
	const AUncappingTableCombSlot* Slot = ResolveOwnerSlot();
	return Slot ? Slot->GetPlacedCombActor() : nullptr;
}

void UCombUncappingPartFocusActionComponent::RebuildOwnerTableDescriptors() const
{
	const AUncappingTableCombSlot* Slot = ResolveOwnerSlot();
	AUncappingTable* Table = Slot ? Cast<AUncappingTable>(Slot->GetAttachParentActor()) : nullptr;
	if (!Table)
	{
		return;
	}

	Table->RebuildCursorPartFocusDescriptors();
	Table->RebuildItemUseAreaDescriptors();
}
