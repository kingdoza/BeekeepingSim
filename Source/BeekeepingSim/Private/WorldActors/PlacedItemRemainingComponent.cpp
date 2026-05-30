#include "WorldActors/PlacedItemRemainingComponent.h"

#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/ItemPlacementSlot.h"

void UPlacedItemRemainingComponent::InitializeFromPlacement(UItemInstance* SourceItemInstance, UItemDefinition* ReturnItemDefinition, AActor* InOwningPlacementSlotActor)
{
	DeactivateRemaining();
	OwningPlacementSlotActor = InOwningPlacementSlotActor;

	if (!ReturnItemDefinition)
	{
		return;
	}

	const FPlacedItemRemainingSpec& RemainingSpec = ReturnItemDefinition->PlacedRemainingSpec;
	if (!RemainingSpec.bUseDurabilityAsPlacedRemaining)
	{
		return;
	}

	if (!ReturnItemDefinition->bUsesDurability || ReturnItemDefinition->MaxDurability <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has invalid placed remaining durability config. Remaining state is disabled."), *GetNameSafe(ReturnItemDefinition));
		return;
	}

	bHasRemaining = true;
	bClearOwningSlotWhenDepleted = RemainingSpec.bClearOwningSlotWhenDepleted;
	MaxAmount = FMath::Max(0.0f, ReturnItemDefinition->MaxDurability);
	CurrentAmount = SourceItemInstance ? SourceItemInstance->GetCurrentDurability() : MaxAmount;
	CurrentAmount = FMath::Clamp(CurrentAmount, 0.0f, MaxAmount);
	BroadcastRemainingRatioChanged();
}

void UPlacedItemRemainingComponent::DeactivateRemaining()
{
	bHasRemaining = false;
	CurrentAmount = 0.0f;
	MaxAmount = 0.0f;
	bClearOwningSlotWhenDepleted = false;
	BroadcastRemainingRatioChanged();
}

float UPlacedItemRemainingComponent::GetRemainingRatio() const
{
	if (!bHasRemaining || MaxAmount <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentAmount / MaxAmount, 0.0f, 1.0f);
}

void UPlacedItemRemainingComponent::SetCurrentAmount(float NewAmount)
{
	ApplyCurrentAmount(NewAmount, true);
}

float UPlacedItemRemainingComponent::ConsumeAmount(float Amount)
{
	if (!bHasRemaining || Amount <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousAmount = CurrentAmount;
	ApplyCurrentAmount(CurrentAmount - Amount, true);
	return FMath::Max(0.0f, PreviousAmount - CurrentAmount);
}

void UPlacedItemRemainingComponent::WriteBackToItemInstance(UItemInstance* TargetItemInstance) const
{
	if (!bHasRemaining || !TargetItemInstance)
	{
		return;
	}

	TargetItemInstance->SetDurability(CurrentAmount);
}

void UPlacedItemRemainingComponent::ApplyCurrentAmount(float NewAmount, bool bHandleDepletion)
{
	if (!bHasRemaining)
	{
		return;
	}

	CurrentAmount = FMath::Clamp(NewAmount, 0.0f, FMath::Max(0.0f, MaxAmount));
	BroadcastRemainingRatioChanged();

	if (bHandleDepletion)
	{
		TryClearOwningSlotWhenDepleted();
	}
}

void UPlacedItemRemainingComponent::TryClearOwningSlotWhenDepleted()
{
	if (!bHasRemaining || !bClearOwningSlotWhenDepleted || CurrentAmount > 0.0f)
	{
		return;
	}

	AActor* SlotActor = OwningPlacementSlotActor.Get();
	if (!SlotActor || !SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		return;
	}

	IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
}

void UPlacedItemRemainingComponent::BroadcastRemainingRatioChanged()
{
	OnRemainingRatioChanged.Broadcast(GetRemainingRatio());
}

