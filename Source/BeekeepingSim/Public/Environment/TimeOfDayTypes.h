#pragma once

#include "CoreMinimal.h"
#include "TimeOfDayTypes.generated.h"

class UCurveFloat;
class UCurveLinearColor;

USTRUCT(BlueprintType)
struct FTimeOfDayCurveSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveLinearColor> SkyZenithColorCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveLinearColor> SkyHorizonColorCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveFloat> SunIntensityCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveFloat> SunTemperatureCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveFloat> MoonIntensityCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveFloat> MoonTemperatureCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveFloat> FogDensityCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves")
	TObjectPtr<UCurveFloat> AmbientIntensityCurve = nullptr;
};

USTRUCT(BlueprintType)
struct FTimeOfDayVisualState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float Hour24 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float NormalizedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	FLinearColor SkyZenithColor = FLinearColor(0.02f, 0.04f, 0.10f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	FLinearColor SkyHorizonColor = FLinearColor(0.06f, 0.08f, 0.14f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float SunIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float SunTemperature = 6500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float MoonIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float MoonTemperature = 8500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float FogDensity = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	float AmbientIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	FRotator SunRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	FRotator MoonRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	bool bSunAboveHorizon = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day")
	bool bMoonLightActive = false;
};
