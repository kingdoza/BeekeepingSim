#pragma once

#include "CoreMinimal.h"
#include "Environment/TimeOfDayProvider.h"
#include "Environment/TimeOfDayTypes.h"
#include "GameFramework/Actor.h"
#include "EnvironmentTimeOfDayActor.generated.h"

class ADirectionalLight;
class ASkyLight;
class AExponentialHeightFog;
class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;
class UCurveFloat;
class UCurveLinearColor;
class UExponentialHeightFogComponent;
class USkyLightComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogBeekeepingEnvironment, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChangedSignature, float, Hour24, const FTimeOfDayVisualState&, VisualState);

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AEnvironmentTimeOfDayActor : public AActor, public ITimeOfDayProvider
{
	GENERATED_BODY()

public:
	AEnvironmentTimeOfDayActor();

	virtual void Tick(float DeltaTime) override;
	virtual float GetCurrentHour24_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	void SetCurrentHour24(float NewHour);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Time Of Day")
	float GetCurrentHour24() const;

	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	void SetTimeProgressionEnabled(bool bEnabled);

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Time Of Day|Preview")
	void ApplyPreviewTime();

	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	FTimeOfDayVisualState EvaluateCurrentVisualState() const;

	UPROPERTY(BlueprintAssignable, Category = "Time Of Day")
	FOnTimeOfDayChangedSignature OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Time Of Day")
	FOnGameTimeOfDayChangedSignature OnGameTimeOfDayChanged;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	float NormalizeHour(float Hour) const;
	float GetNormalizedTime(float Hour) const;
	bool IsHourInDaylight(float Hour) const;
	FRotator CalculateCelestialRotation(float Hour, bool& bOutAboveHorizon) const;
	FTimeOfDayVisualState EvaluateVisualState(float Hour) const;
	void ApplyVisualState(const FTimeOfDayVisualState& State);
	void ApplyCurrentTimeState();
	float EvaluateCurveFloatOrFallback(const UCurveFloat* Curve, float Time, float FallbackValue) const;
	FLinearColor EvaluateCurveColorOrFallback(const UCurveLinearColor* Curve, float Time, const FLinearColor& FallbackValue) const;
	void LogMissingReferencesOnce();
	bool ShouldUseEditorPreview() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "24.0", UIMin = "0.0", UIMax = "24.0"))
	float CurrentHour24 = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float DayLengthSeconds = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	bool bTimeProgressionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	float SunriseHour = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	float SunsetHour = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	float MaxSunAltitudeDegrees = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	float CelestialYawOffsetDegrees = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Scene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ADirectionalLight> SunLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Scene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ADirectionalLight> MoonLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Scene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASkyLight> SkyLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Scene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AExponentialHeightFog> HeightFog = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Scene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialParameterCollection> SkyParameterCollection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Scene", meta = (AllowPrivateAccess = "true"))
	bool bRecaptureSkyLightOnApply = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Curves", meta = (AllowPrivateAccess = "true"))
	FTimeOfDayCurveSet CurveSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true"))
	FLinearColor FallbackSkyZenithColor = FLinearColor(0.02f, 0.04f, 0.10f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true"))
	FLinearColor FallbackSkyHorizonColor = FLinearColor(0.06f, 0.08f, 0.14f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FallbackSunIntensity = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true"))
	float FallbackSunTemperature = 6500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FallbackMoonIntensity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true"))
	float FallbackMoonTemperature = 8500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FallbackFogDensity = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Fallback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FallbackAmbientIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Preview", meta = (AllowPrivateAccess = "true"))
	bool bUseEditorPreviewTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Preview", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "24.0", UIMin = "0.0", UIMax = "24.0", EditCondition = "bUseEditorPreviewTime", EditConditionHides))
	float PreviewHour24 = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Preview", meta = (AllowPrivateAccess = "true", EditCondition = "bUseEditorPreviewTime"))
	bool bUpdatePreviewOnPropertyChange = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Preview", meta = (AllowPrivateAccess = "true", EditCondition = "bUseEditorPreviewTime"))
	bool bStartPlayFromPreviewHour = true;

	bool bHasLoggedMissingSunLight = false;
	bool bHasLoggedMissingMoonLight = false;
	bool bHasLoggedMissingSkyLight = false;
	bool bHasLoggedMissingHeightFog = false;
	bool bHasLoggedInvalidDayLength = false;
	bool bHasLoggedMissingParameterCollection = false;
	bool bHasLoggedMissingParameterCollectionInstance = false;
};
