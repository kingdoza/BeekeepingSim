#include "WorldActors/PlacedItemRemainingVisualComponent.h"

#include "WorldActors/PlacedItemActor.h"
#include "WorldActors/PlacedItemRemainingComponent.h"

void UPlacedItemRemainingVisualComponent::InitializeRemainingVisual(APlacedItemActor* InPlacedItemActor, UPlacedItemRemainingComponent* InRemainingComponent)
{
	if (RemainingComponent)
	{
		RemainingComponent->OnRemainingRatioChanged.RemoveDynamic(this, &UPlacedItemRemainingVisualComponent::HandleRemainingRatioChanged);
	}

	PlacedItemActor = InPlacedItemActor;
	RemainingComponent = InRemainingComponent;

	if (RemainingComponent)
	{
		RemainingComponent->OnRemainingRatioChanged.AddDynamic(this, &UPlacedItemRemainingVisualComponent::HandleRemainingRatioChanged);
		ApplyRemainingRatio(RemainingComponent->GetRemainingRatio());
		return;
	}

	ApplyRemainingRatio(0.0f);
}

void UPlacedItemRemainingVisualComponent::ApplyRemainingRatio(float Ratio)
{
	ReceiveRemainingRatioChanged(Ratio);
}

void UPlacedItemRemainingVisualComponent::HandleRemainingRatioChanged(float Ratio)
{
	ApplyRemainingRatio(Ratio);
}

void UPlacedItemRemainingVisualComponent::ReceiveRemainingRatioChanged_Implementation(float Ratio)
{
	(void)Ratio;
}

