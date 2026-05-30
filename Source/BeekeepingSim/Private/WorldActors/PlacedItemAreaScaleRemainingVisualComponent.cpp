#include "WorldActors/PlacedItemAreaScaleRemainingVisualComponent.h"

#include "Components/StaticMeshComponent.h"
#include "WorldActors/PlacedItemActor.h"

void UPlacedItemAreaScaleRemainingVisualComponent::ApplyRemainingRatio(float Ratio)
{
	const float ClampedRatio = FMath::Clamp(Ratio, 0.0f, 1.0f);
	const float XYScale = FMath::Sqrt(ClampedRatio);

	if (APlacedItemActor* PlacedActor = GetPlacedItemActor())
	{
		if (UStaticMeshComponent* PrimaryPlacedMesh = PlacedActor->GetItemMeshComponent())
		{
			PrimaryPlacedMesh->SetRelativeScale3D(FVector(XYScale, XYScale, 1.0f));
		}
	}

	Super::ApplyRemainingRatio(ClampedRatio);
}
