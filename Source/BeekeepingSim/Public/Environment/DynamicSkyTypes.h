#pragma once

#include "CoreMinimal.h"
#include "DynamicSkyTypes.generated.h"

USTRUCT(BlueprintType)
struct FDynamicSkyState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	float Hour24 = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	FRotator SunWorldRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	FRotator MoonWorldRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	bool bSunLightVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	bool bMoonLightVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	FLinearColor RayleighScattering = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	float MultiScattering = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	float IsStarVisible = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dynamic Sky")
	float IsMoonVisible = 0.0f;
};
