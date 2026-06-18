#include "WorldActors/QueenCellSpawnAreaComponent.h"

namespace
{
	enum class EQueenCellSpawnEdge : uint8
	{
		Bottom,
		Left,
		Right,
		Top
	};

	float SanitizeNonNegative(float Value)
	{
		return FMath::IsFinite(Value) ? FMath::Max(0.0f, Value) : 0.0f;
	}
}

UQueenCellSpawnAreaComponent::UQueenCellSpawnAreaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InitBoxExtent(FVector(3.0f, 45.0f, 45.0f));
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool UQueenCellSpawnAreaComponent::CanSampleQueenCellPlacement(const TArray<FQueenCellPlacement>& ExistingPlacements) const
{
	FQueenCellPlacement TestPlacement;
	return TrySampleQueenCellPlacement(ExistingPlacements, TestPlacement);
}

bool UQueenCellSpawnAreaComponent::TrySampleQueenCellPlacement(const TArray<FQueenCellPlacement>& ExistingPlacements, FQueenCellPlacement& OutPlacement) const
{
	float YMin = 0.0f;
	float YMax = 0.0f;
	float ZMin = 0.0f;
	float ZMax = 0.0f;
	float BandWidth = 0.0f;
	if (!HasUsableSurface(YMin, YMax, ZMin, ZMax, BandWidth))
	{
		return false;
	}

	const int32 SafeAttempts = FMath::Max(1, MaxPlacementAttempts);
	for (int32 Attempt = 0; Attempt < SafeAttempts; ++Attempt)
	{
		const EBeehiveCombVisibleFace Face = FMath::RandRange(0, 1) == 0
			? EBeehiveCombVisibleFace::Front
			: EBeehiveCombVisibleFace::Back;

		FVector2D AreaLocalYZ = FVector2D::ZeroVector;
		if (!TrySampleAreaLocalYZ(AreaLocalYZ))
		{
			return false;
		}

		if (IsInsideCenterArea(AreaLocalYZ, YMin, YMax, ZMin, ZMax, BandWidth))
		{
			continue;
		}

		if (!IsFarEnoughFromExisting(ExistingPlacements, Face, AreaLocalYZ))
		{
			continue;
		}

		OutPlacement.QueenCellId = FGuid::NewGuid();
		OutPlacement.Face = Face;
		OutPlacement.AreaLocalYZ = AreaLocalYZ;
		OutPlacement.LocalRotationDegrees = FMath::FRandRange(0.0f, 360.0f);
		OutPlacement.Scale = 1.0f;
		return true;
	}

	return false;
}

bool UQueenCellSpawnAreaComponent::HasUsableSurface(float& OutYMin, float& OutYMax, float& OutZMin, float& OutZMax, float& OutBandWidth) const
{
	const FVector Extent = GetUnscaledBoxExtent();
	const float Inset = SanitizeNonNegative(EdgeInsetCm);
	OutYMin = -Extent.Y + Inset;
	OutYMax = Extent.Y - Inset;
	OutZMin = -Extent.Z + Inset;
	OutZMax = Extent.Z - Inset;

	if (OutYMin >= OutYMax || OutZMin >= OutZMax)
	{
		return false;
	}

	const float UsableHalfWidth = FMath::Min((OutYMax - OutYMin) * 0.5f, (OutZMax - OutZMin) * 0.5f);
	OutBandWidth = FMath::Clamp(SanitizeNonNegative(EdgeBandWidthCm), 0.0f, UsableHalfWidth);
	return OutBandWidth > 0.0f;
}

bool UQueenCellSpawnAreaComponent::TrySampleAreaLocalYZ(FVector2D& OutAreaLocalYZ) const
{
	float YMin = 0.0f;
	float YMax = 0.0f;
	float ZMin = 0.0f;
	float ZMax = 0.0f;
	float BandWidth = 0.0f;
	if (!HasUsableSurface(YMin, YMax, ZMin, ZMax, BandWidth))
	{
		return false;
	}

	const float Weights[] =
	{
		SanitizeNonNegative(BottomEdgeWeight),
		SanitizeNonNegative(LeftEdgeWeight),
		SanitizeNonNegative(RightEdgeWeight),
		SanitizeNonNegative(TopEdgeWeight)
	};

	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return false;
	}

	const float SelectionThreshold = FMath::FRandRange(0.0f, TotalWeight);
	float CumulativeWeight = 0.0f;
	EQueenCellSpawnEdge SelectedEdge = EQueenCellSpawnEdge::Bottom;
	for (int32 EdgeIndex = 0; EdgeIndex < UE_ARRAY_COUNT(Weights); ++EdgeIndex)
	{
		CumulativeWeight += Weights[EdgeIndex];
		if (SelectionThreshold <= CumulativeWeight)
		{
			SelectedEdge = static_cast<EQueenCellSpawnEdge>(EdgeIndex);
			break;
		}
	}

	switch (SelectedEdge)
	{
	case EQueenCellSpawnEdge::Bottom:
		OutAreaLocalYZ = FVector2D(
			FMath::FRandRange(YMin, YMax),
			FMath::FRandRange(ZMin, FMath::Min(ZMin + BandWidth, ZMax)));
		break;
	case EQueenCellSpawnEdge::Left:
		OutAreaLocalYZ = FVector2D(
			FMath::FRandRange(YMin, FMath::Min(YMin + BandWidth, YMax)),
			FMath::FRandRange(ZMin, ZMax));
		break;
	case EQueenCellSpawnEdge::Right:
		OutAreaLocalYZ = FVector2D(
			FMath::FRandRange(FMath::Max(YMax - BandWidth, YMin), YMax),
			FMath::FRandRange(ZMin, ZMax));
		break;
	case EQueenCellSpawnEdge::Top:
		OutAreaLocalYZ = FVector2D(
			FMath::FRandRange(YMin, YMax),
			FMath::FRandRange(FMath::Max(ZMax - BandWidth, ZMin), ZMax));
		break;
	default:
		return false;
	}

	return true;
}

bool UQueenCellSpawnAreaComponent::IsInsideCenterArea(const FVector2D& AreaLocalYZ, float YMin, float YMax, float ZMin, float ZMax, float BandWidth) const
{
	return AreaLocalYZ.X > YMin + BandWidth
		&& AreaLocalYZ.X < YMax - BandWidth
		&& AreaLocalYZ.Y > ZMin + BandWidth
		&& AreaLocalYZ.Y < ZMax - BandWidth;
}

bool UQueenCellSpawnAreaComponent::IsFarEnoughFromExisting(const TArray<FQueenCellPlacement>& ExistingPlacements, EBeehiveCombVisibleFace Face, const FVector2D& AreaLocalYZ) const
{
	const float MinSpacing = SanitizeNonNegative(MinQueenCellSpacingCm);
	const float MinSpacingSquared = FMath::Square(MinSpacing);
	for (const FQueenCellPlacement& ExistingPlacement : ExistingPlacements)
	{
		if (ExistingPlacement.Face != Face)
		{
			continue;
		}

		if (FVector2D::DistSquared(ExistingPlacement.AreaLocalYZ, AreaLocalYZ) < MinSpacingSquared)
		{
			return false;
		}
	}

	return true;
}
