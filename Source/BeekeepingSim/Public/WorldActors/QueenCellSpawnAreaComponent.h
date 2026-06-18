#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "WorldActors/BeehiveCombActor.h"
#include "QueenCellSpawnAreaComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UQueenCellSpawnAreaComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UQueenCellSpawnAreaComponent();

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Cell")
	bool CanSampleQueenCellPlacement(const TArray<FQueenCellPlacement>& ExistingPlacements) const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Queen Cell")
	bool TrySampleQueenCellPlacement(const TArray<FQueenCellPlacement>& ExistingPlacements, FQueenCellPlacement& OutPlacement) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float EdgeBandWidthCm = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float EdgeInsetCm = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float MinQueenCellSpacingCm = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "1"))
	int32 MaxPlacementAttempts = 32;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float BottomEdgeWeight = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float LeftEdgeWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float RightEdgeWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0"))
	float TopEdgeWeight = 0.5f;

private:
	bool HasUsableSurface(float& OutYMin, float& OutYMax, float& OutZMin, float& OutZMax, float& OutBandWidth) const;
	bool TrySampleAreaLocalYZ(FVector2D& OutAreaLocalYZ) const;
	bool IsInsideCenterArea(const FVector2D& AreaLocalYZ, float YMin, float YMax, float ZMin, float ZMax, float BandWidth) const;
	bool IsFarEnoughFromExisting(const TArray<FQueenCellPlacement>& ExistingPlacements, EBeehiveCombVisibleFace Face, const FVector2D& AreaLocalYZ) const;
};
