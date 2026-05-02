#include "Environment/EnvironmentTimeOfDayActor.h"

#include "Environment/GameTimeBucketSubsystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY(LogBeekeepingEnvironment);

namespace
{
	const FName ParamSkyZenithColor(TEXT("SkyZenithColor"));
	const FName ParamSkyHorizonColor(TEXT("SkyHorizonColor"));
	const FName ParamSunDirection(TEXT("SunDirection"));
	const FName ParamMoonDirection(TEXT("MoonDirection"));
	const FName ParamSunIntensity(TEXT("SunIntensity"));
	const FName ParamMoonIntensity(TEXT("MoonIntensity"));
	const FName ParamFogDensity(TEXT("FogDensity"));
	const FName ParamAmbientIntensity(TEXT("AmbientIntensity"));
	const float MinDayLengthSeconds = 1.0f;
	const float MinSegmentDuration = 0.001f;

	FString FormatHour24(float Hour)
	{
		const float NormalizedHour = FMath::Fmod(FMath::Fmod(Hour, 24.0f) + 24.0f, 24.0f);
		const int32 TotalSeconds = FMath::FloorToInt((NormalizedHour * 3600.0f) + 0.5f) % 86400;
		const int32 Hours = TotalSeconds / 3600;
		const int32 Minutes = (TotalSeconds % 3600) / 60;
		const int32 Seconds = TotalSeconds % 60;
		return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
	}
}

AEnvironmentTimeOfDayActor::AEnvironmentTimeOfDayActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnvironmentTimeOfDayActor::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (bUseEditorPreviewTime && bStartPlayFromPreviewHour)
	{
		CurrentHour24 = NormalizeHour(PreviewHour24);
	}
#endif

	ApplyCurrentTimeState();

	if (UWorld* World = GetWorld())
	{
		if (UGameTimeBucketSubsystem* BucketSubsystem = World->GetSubsystem<UGameTimeBucketSubsystem>())
		{
			BucketSubsystem->SetTimeOfDayActor(this);
		}
	}
}

void AEnvironmentTimeOfDayActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTimeProgressionEnabled || ShouldUseEditorPreview())
	{
		//UE_LOG(LogBeekeepingEnvironment, Log, TEXT("Current time: %s"), *FormatHour24(CurrentHour24));
		return;
	}

	if (DayLengthSeconds < MinDayLengthSeconds)
	{
		if (!bHasLoggedInvalidDayLength)
		{
			//UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("DayLengthSeconds is invalid (%.3f). Use value >= 1.0."), DayLengthSeconds);
			bHasLoggedInvalidDayLength = true;
		}
		//UE_LOG(LogBeekeepingEnvironment, Log, TEXT("Current time: %s"), *FormatHour24(CurrentHour24));
		return;
	}

	bHasLoggedInvalidDayLength = false;

	const float HoursPerSecond = 24.0f / DayLengthSeconds;
	CurrentHour24 = NormalizeHour(CurrentHour24 + (DeltaTime * HoursPerSecond));
	//UE_LOG(LogBeekeepingEnvironment, Log, TEXT("Current time: %s"), *FormatHour24(CurrentHour24));
	ApplyCurrentTimeState();
}

void AEnvironmentTimeOfDayActor::SetCurrentHour24(float NewHour)
{
	CurrentHour24 = NormalizeHour(NewHour);
	ApplyCurrentTimeState();
}

void AEnvironmentTimeOfDayActor::SetTimeProgressionEnabled(bool bEnabled)
{
	bTimeProgressionEnabled = bEnabled;
}

void AEnvironmentTimeOfDayActor::ApplyPreviewTime()
{
#if WITH_EDITOR
	if (!bUseEditorPreviewTime)
	{
		return;
	}

	const FTimeOfDayVisualState PreviewState = EvaluateVisualState(PreviewHour24);
	ApplyVisualState(PreviewState);
	OnTimeOfDayChanged.Broadcast(PreviewState.Hour24, PreviewState);
#endif
}

FTimeOfDayVisualState AEnvironmentTimeOfDayActor::EvaluateCurrentVisualState() const
{
	if (ShouldUseEditorPreview())
	{
		return EvaluateVisualState(PreviewHour24);
	}

	return EvaluateVisualState(CurrentHour24);
}

#if WITH_EDITOR
void AEnvironmentTimeOfDayActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!bUseEditorPreviewTime || !bUpdatePreviewOnPropertyChange)
	{
		return;
	}

	const FName ChangedPropertyName = PropertyChangedEvent.GetPropertyName();
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, PreviewHour24)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, CurveSet)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, bUseEditorPreviewTime)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, SunLight)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, MoonLight)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, SkyLight)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, HeightFog)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AEnvironmentTimeOfDayActor, SkyParameterCollection))
	{
		ApplyPreviewTime();
	}
}
#endif

float AEnvironmentTimeOfDayActor::NormalizeHour(float Hour) const
{
	const float Wrapped = FMath::Fmod(Hour, 24.0f);
	return (Wrapped < 0.0f) ? Wrapped + 24.0f : Wrapped;
}

float AEnvironmentTimeOfDayActor::GetNormalizedTime(float Hour) const
{
	return NormalizeHour(Hour) / 24.0f;
}

bool AEnvironmentTimeOfDayActor::IsHourInDaylight(float Hour) const
{
	const float Start = NormalizeHour(SunriseHour);
	const float End = NormalizeHour(SunsetHour);
	const float Current = NormalizeHour(Hour);

	if (FMath::IsNearlyEqual(Start, End))
	{
		return true;
	}

	if (Start < End)
	{
		return Current >= Start && Current <= End;
	}

	return Current >= Start || Current <= End;
}

FRotator AEnvironmentTimeOfDayActor::CalculateCelestialRotation(float Hour, bool& bOutAboveHorizon) const
{
	const float Start = NormalizeHour(SunriseHour);
	const float End = NormalizeHour(SunsetHour);
	const float Current = NormalizeHour(Hour);
	const float DayDuration = FMath::Max(FMath::Fmod((End - Start + 24.0f), 24.0f), MinSegmentDuration);
	const float NightDuration = FMath::Max(24.0f - DayDuration, MinSegmentDuration);

	bOutAboveHorizon = IsHourInDaylight(Current);

	float OrbitAlpha = 0.0f;
	float Altitude = 0.0f;
	float Yaw = CelestialYawOffsetDegrees;

	if (bOutAboveHorizon)
	{
		const float DeltaFromSunrise = FMath::Fmod((Current - Start + 24.0f), 24.0f);
		OrbitAlpha = FMath::Clamp(DeltaFromSunrise / DayDuration, 0.0f, 1.0f);
		Altitude = FMath::Sin(OrbitAlpha * PI) * MaxSunAltitudeDegrees;
		Yaw = FMath::Lerp(-90.0f, 90.0f, OrbitAlpha) + CelestialYawOffsetDegrees;
	}
	else
	{
		const float DeltaFromSunset = FMath::Fmod((Current - End + 24.0f), 24.0f);
		OrbitAlpha = FMath::Clamp(DeltaFromSunset / NightDuration, 0.0f, 1.0f);
		Altitude = -FMath::Sin(OrbitAlpha * PI) * MaxSunAltitudeDegrees;
		Yaw = FMath::Lerp(90.0f, 270.0f, OrbitAlpha) + CelestialYawOffsetDegrees;
	}

	return FRotator(-Altitude, Yaw, 0.0f);
}

FTimeOfDayVisualState AEnvironmentTimeOfDayActor::EvaluateVisualState(float Hour) const
{
	FTimeOfDayVisualState State;
	State.Hour24 = NormalizeHour(Hour);
	State.NormalizedTime = GetNormalizedTime(State.Hour24);
	State.SkyZenithColor = EvaluateCurveColorOrFallback(CurveSet.SkyZenithColorCurve, State.NormalizedTime, FallbackSkyZenithColor);
	State.SkyHorizonColor = EvaluateCurveColorOrFallback(CurveSet.SkyHorizonColorCurve, State.NormalizedTime, FallbackSkyHorizonColor);
	State.SunIntensity = EvaluateCurveFloatOrFallback(CurveSet.SunIntensityCurve, State.NormalizedTime, FallbackSunIntensity);
	State.SunTemperature = EvaluateCurveFloatOrFallback(CurveSet.SunTemperatureCurve, State.NormalizedTime, FallbackSunTemperature);
	State.MoonIntensity = EvaluateCurveFloatOrFallback(CurveSet.MoonIntensityCurve, State.NormalizedTime, FallbackMoonIntensity);
	State.MoonTemperature = EvaluateCurveFloatOrFallback(CurveSet.MoonTemperatureCurve, State.NormalizedTime, FallbackMoonTemperature);
	State.FogDensity = EvaluateCurveFloatOrFallback(CurveSet.FogDensityCurve, State.NormalizedTime, FallbackFogDensity);
	State.AmbientIntensity = EvaluateCurveFloatOrFallback(CurveSet.AmbientIntensityCurve, State.NormalizedTime, FallbackAmbientIntensity);

	State.SunRotation = CalculateCelestialRotation(State.Hour24, State.bSunAboveHorizon);

	const float MoonHour = NormalizeHour(State.Hour24 + 12.0f);
	bool bMoonAboveHorizon = false;
	State.MoonRotation = CalculateCelestialRotation(MoonHour, bMoonAboveHorizon);
	State.bMoonLightActive = !State.bSunAboveHorizon && bMoonAboveHorizon;

	if (!State.bSunAboveHorizon)
	{
		State.SunIntensity = 0.0f;
	}

	if (!State.bMoonLightActive)
	{
		State.MoonIntensity = 0.0f;
	}

	State.SunIntensity = FMath::Max(0.0f, State.SunIntensity);
	State.MoonIntensity = FMath::Max(0.0f, State.MoonIntensity);
	State.FogDensity = FMath::Max(0.0f, State.FogDensity);
	State.AmbientIntensity = FMath::Max(0.0f, State.AmbientIntensity);

	return State;
}

void AEnvironmentTimeOfDayActor::ApplyVisualState(const FTimeOfDayVisualState& State)
{
	LogMissingReferencesOnce();

	if (SunLight)
	{
		SunLight->SetActorRotation(State.SunRotation);
		if (UDirectionalLightComponent* SunComponent = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent()))
		{
			SunComponent->SetUseTemperature(true);
			SunComponent->SetTemperature(State.SunTemperature);
			SunComponent->SetIntensity(State.bSunAboveHorizon ? State.SunIntensity : 0.0f);
		}
	}

	if (MoonLight)
	{
		MoonLight->SetActorRotation(State.MoonRotation);
		if (UDirectionalLightComponent* MoonComponent = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent()))
		{
			MoonComponent->SetUseTemperature(true);
			MoonComponent->SetTemperature(State.MoonTemperature);
			MoonComponent->SetIntensity(State.bMoonLightActive ? State.MoonIntensity : 0.0f);
		}
	}

	if (SkyLight)
	{
		if (USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent())
		{
			SkyLightComponent->SetIntensity(State.AmbientIntensity);
			if (bRecaptureSkyLightOnApply)
			{
				SkyLightComponent->RecaptureSky();
			}
		}
	}

	if (HeightFog)
	{
		if (UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent())
		{
			FogComponent->SetFogDensity(State.FogDensity);
		}
	}

	if (!SkyParameterCollection)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UMaterialParameterCollectionInstance* ParameterCollectionInstance = World->GetParameterCollectionInstance(SkyParameterCollection);
	if (!ParameterCollectionInstance)
	{
		if (!bHasLoggedMissingParameterCollectionInstance)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Failed to acquire material parameter collection instance for '%s'."), *GetNameSafe(SkyParameterCollection));
			bHasLoggedMissingParameterCollectionInstance = true;
		}
		return;
	}

	const FVector SunDirection = State.SunRotation.Vector();
	const FVector MoonDirection = State.MoonRotation.Vector();

	ParameterCollectionInstance->SetVectorParameterValue(ParamSkyZenithColor, State.SkyZenithColor);
	ParameterCollectionInstance->SetVectorParameterValue(ParamSkyHorizonColor, State.SkyHorizonColor);
	ParameterCollectionInstance->SetVectorParameterValue(ParamSunDirection, FLinearColor(SunDirection.X, SunDirection.Y, SunDirection.Z, 0.0f));
	ParameterCollectionInstance->SetVectorParameterValue(ParamMoonDirection, FLinearColor(MoonDirection.X, MoonDirection.Y, MoonDirection.Z, 0.0f));
	ParameterCollectionInstance->SetScalarParameterValue(ParamSunIntensity, State.SunIntensity);
	ParameterCollectionInstance->SetScalarParameterValue(ParamMoonIntensity, State.MoonIntensity);
	ParameterCollectionInstance->SetScalarParameterValue(ParamFogDensity, State.FogDensity);
	ParameterCollectionInstance->SetScalarParameterValue(ParamAmbientIntensity, State.AmbientIntensity);
}

void AEnvironmentTimeOfDayActor::ApplyCurrentTimeState()
{
	const FTimeOfDayVisualState State = EvaluateCurrentVisualState();
	ApplyVisualState(State);
	OnTimeOfDayChanged.Broadcast(State.Hour24, State);
}

float AEnvironmentTimeOfDayActor::EvaluateCurveFloatOrFallback(const UCurveFloat* Curve, float Time, float FallbackValue) const
{
	if (!Curve)
	{
		return FallbackValue;
	}

	return Curve->GetFloatValue(Time);
}

FLinearColor AEnvironmentTimeOfDayActor::EvaluateCurveColorOrFallback(const UCurveLinearColor* Curve, float Time, const FLinearColor& FallbackValue) const
{
	if (!Curve)
	{
		return FallbackValue;
	}

	return Curve->GetLinearColorValue(Time);
}

void AEnvironmentTimeOfDayActor::LogMissingReferencesOnce()
{
	if (!SunLight && !bHasLoggedMissingSunLight)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("SunLight reference is not set on '%s'."), *GetName());
		bHasLoggedMissingSunLight = true;
	}

	if (!MoonLight && !bHasLoggedMissingMoonLight)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("MoonLight reference is not set on '%s'."), *GetName());
		bHasLoggedMissingMoonLight = true;
	}

	if (!SkyLight && !bHasLoggedMissingSkyLight)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("SkyLight reference is not set on '%s'."), *GetName());
		bHasLoggedMissingSkyLight = true;
	}

	if (!HeightFog && !bHasLoggedMissingHeightFog)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("HeightFog reference is not set on '%s'."), *GetName());
		bHasLoggedMissingHeightFog = true;
	}

	if (!SkyParameterCollection && !bHasLoggedMissingParameterCollection)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("SkyParameterCollection reference is not set on '%s'."), *GetName());
		bHasLoggedMissingParameterCollection = true;
	}
}

bool AEnvironmentTimeOfDayActor::ShouldUseEditorPreview() const
{
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		return bUseEditorPreviewTime && !World->IsGameWorld();
	}

	return false;
#else
	return false;
#endif
}
