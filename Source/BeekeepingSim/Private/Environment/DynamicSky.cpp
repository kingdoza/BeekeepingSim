#include "Environment/DynamicSky.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Environment/EnvironmentTimeOfDayActor.h"
#include "Environment/GameTimeOfDayActor.h"
#include "Environment/TimeOfDayProvider.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr float MinDuration = 0.001f;
	const FName ParamIsStarVisible(TEXT("IsStarVisible"));
	const FName ParamIsMoonVisible(TEXT("IsMoonVisible"));
}

ADynamicSky::ADynamicSky()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"));
	ExponentialHeightFog->SetupAttachment(Root);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(Root);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);

	SunDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunDirectionalLight"));
	SunDirectionalLight->SetupAttachment(Root);
	SunDirectionalLight->SetAtmosphereSunLight(true);
	SunDirectionalLight->SetAtmosphereSunLightIndex(0);

	MoonDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonDirectionalLight"));
	MoonDirectionalLight->SetupAttachment(Root);
	MoonDirectionalLight->SetAtmosphereSunLight(true);
	MoonDirectionalLight->SetAtmosphereSunLightIndex(1);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(Root);

	SkySphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphereMesh"));
	SkySphereMesh->SetupAttachment(Root);
}

void ADynamicSky::BeginPlay()
{
	Super::BeginPlay();

	RefreshSkySphereMaterial();
	const bool bIsBound = ResolveAndBindTimeSource();
	if (bIsBound && bUseEditorPreviewTime && bStartGameTimeFromPreviewHour)
	{
		if (BoundLegacyTimeOfDayActor)
		{
			BoundLegacyTimeOfDayActor->SetCurrentHour24(PreviewHour24);
			return;
		}
	}

	if (bApplyCurrentTimeOnBeginPlay && bIsBound)
	{
		if (BoundGameTimeOfDayActor)
		{
			ApplySkyState(BoundGameTimeOfDayActor->GetCurrentHour24());
		}
		else if (BoundLegacyTimeOfDayActor)
		{
			ApplySkyState(BoundLegacyTimeOfDayActor->GetCurrentHour24());
		}
	}
}

void ADynamicSky::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindTimeSource();
	Super::EndPlay(EndPlayReason);
}

void ADynamicSky::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshSkySphereMaterial();
	if (ShouldUseEditorPreview())
	{
		ApplyPreviewTime();
	}
}

void ADynamicSky::ApplyPreviewTime()
{
#if WITH_EDITOR
	if (!bUseEditorPreviewTime)
	{
		return;
	}

	ApplySkyState(PreviewHour24);
#endif
}

#if WITH_EDITOR
void ADynamicSky::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!bUseEditorPreviewTime || !bUpdatePreviewOnPropertyChange)
	{
		return;
	}

	const FName ChangedProperty = PropertyChangedEvent.GetPropertyName();
	if (ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, PreviewHour24)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, bUseEditorPreviewTime)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, SunriseHour)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, SunsetHour)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, GapTime)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, OrbitYaw)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, SunLightRayleighScattering)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, NoSunLightRayleighScattering)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, SunLightMultiScattering)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, NoSunLightMultiScattering)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, SkySphereMeshAsset)
		|| ChangedProperty == GET_MEMBER_NAME_CHECKED(ADynamicSky, SkySphereMaterial))
	{
		ApplyPreviewTime();
	}
}
#endif

float ADynamicSky::NormalizeHour24(float Hour24)
{
	const float Wrapped = FMath::Fmod(Hour24, 24.0f);
	return Wrapped < 0.0f ? Wrapped + 24.0f : Wrapped;
}

bool ADynamicSky::IsTimeWithinWrappedRange(float Hour24, float RangeStart, float RangeEnd)
{
	const float NormalizedHour = NormalizeHour24(Hour24);
	const float Start = NormalizeHour24(RangeStart);
	const float End = NormalizeHour24(RangeEnd);

	if (FMath::IsNearlyEqual(Start, End))
	{
		return true;
	}

	if (Start < End)
	{
		return NormalizedHour >= Start && NormalizedHour <= End;
	}

	return NormalizedHour >= Start || NormalizedHour <= End;
}

float ADynamicSky::CalculateOrbitElapsedHours(float Hour24, float OrbitStartHour, float OrbitDurationHours, float GapHours)
{
	const float CurrentHour = NormalizeHour24(Hour24);
	const float Start = NormalizeHour24(OrbitStartHour);
	const float ElapsedSinceStart = FMath::Fmod((CurrentHour - Start + 24.0f), 24.0f);
	const float ClampedDuration = FMath::Max(OrbitDurationHours, MinDuration);
	const float ClampedGap = FMath::Max(0.0f, GapHours);

	if (ElapsedSinceStart >= 24.0f - ClampedGap)
	{
		return ElapsedSinceStart - 24.0f;
	}
	if (ElapsedSinceStart <= ClampedDuration + ClampedGap)
	{
		return ElapsedSinceStart;
	}

	return ElapsedSinceStart <= 12.0f ? ElapsedSinceStart : ElapsedSinceStart - 24.0f;
}

float ADynamicSky::EvaluateSunLightBlendAlpha(float Hour24) const
{
	const float CurrentHour = NormalizeHour24(Hour24);
	const float Sunrise = NormalizeHour24(SunriseHour);
	const float Sunset = NormalizeHour24(SunsetHour);
	const float ClampedGap = FMath::Min(GapTime, 11.999f);

	if (ClampedGap <= KINDA_SMALL_NUMBER)
	{
		return IsTimeWithinWrappedRange(CurrentHour, Sunrise, Sunset) ? 1.0f : 0.0f;
	}

	const float SunriseStart = NormalizeHour24(Sunrise - ClampedGap);
	const float SunriseEnd = NormalizeHour24(Sunrise + ClampedGap);
	if (IsTimeWithinWrappedRange(CurrentHour, SunriseStart, SunriseEnd))
	{
		const float Span = ClampedGap * 2.0f;
		const float Elapsed = FMath::Fmod((CurrentHour - SunriseStart + 24.0f), 24.0f);
		return FMath::Clamp(Elapsed / Span, 0.0f, 1.0f);
	}

	const float SunsetStart = NormalizeHour24(Sunset - ClampedGap);
	const float SunsetEnd = NormalizeHour24(Sunset + ClampedGap);
	if (IsTimeWithinWrappedRange(CurrentHour, SunsetStart, SunsetEnd))
	{
		const float Span = ClampedGap * 2.0f;
		const float Elapsed = FMath::Fmod((CurrentHour - SunsetStart + 24.0f), 24.0f);
		return FMath::Clamp(1.0f - (Elapsed / Span), 0.0f, 1.0f);
	}

	return IsTimeWithinWrappedRange(CurrentHour, Sunrise - ClampedGap, Sunset + ClampedGap) ? 1.0f : 0.0f;
}

FLinearColor ADynamicSky::EvaluateRayleighScattering(float Hour24) const
{
	const float Alpha = FMath::Clamp(EvaluateSunLightBlendAlpha(Hour24), 0.0f, 1.0f);
	return FMath::Lerp(NoSunLightRayleighScattering, SunLightRayleighScattering, Alpha);
}

float ADynamicSky::EvaluateMultiScattering(float Hour24) const
{
	const float Alpha = FMath::Clamp(EvaluateSunLightBlendAlpha(Hour24), 0.0f, 1.0f);
	return FMath::Lerp(NoSunLightMultiScattering, SunLightMultiScattering, Alpha);
}

bool ADynamicSky::ResolveAndBindTimeSource()
{
	UnbindTimeSource();

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// If explicit source is set, keep it only when it is canonical, otherwise upgrade to game-time actor if present.
	if (TimeOfDaySourceActor && TimeOfDaySourceActor->GetClass()->ImplementsInterface(UTimeOfDayProvider::StaticClass()))
	{
		if (AGameTimeOfDayActor* ExplicitGameTimeActor = Cast<AGameTimeOfDayActor>(TimeOfDaySourceActor))
		{
			BoundGameTimeOfDayActor = ExplicitGameTimeActor;
		}
		else if (bAutoFindTimeOfDaySource)
		{
			for (TActorIterator<AGameTimeOfDayActor> It(World); It; ++It)
			{
				BoundGameTimeOfDayActor = *It;
				TimeOfDaySourceActor = BoundGameTimeOfDayActor;
				break;
			}
			if (!BoundGameTimeOfDayActor)
			{
				BoundLegacyTimeOfDayActor = Cast<AEnvironmentTimeOfDayActor>(TimeOfDaySourceActor);
			}
		}
		else
		{
			BoundLegacyTimeOfDayActor = Cast<AEnvironmentTimeOfDayActor>(TimeOfDaySourceActor);
		}
	}

	if (!BoundGameTimeOfDayActor && !BoundLegacyTimeOfDayActor && bAutoFindTimeOfDaySource)
	{
		AGameTimeOfDayActor* FirstGameTimeActor = nullptr;
		AEnvironmentTimeOfDayActor* FirstLegacyActor = nullptr;
		int32 GameTimeActorCount = 0;
		int32 LegacyActorCount = 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Candidate = *It;
			if (!IsValid(Candidate) || !Candidate->GetClass()->ImplementsInterface(UTimeOfDayProvider::StaticClass()))
			{
				continue;
			}

			if (AGameTimeOfDayActor* GameTimeActor = Cast<AGameTimeOfDayActor>(Candidate))
			{
				++GameTimeActorCount;
				if (!FirstGameTimeActor)
				{
					FirstGameTimeActor = GameTimeActor;
				}
				continue;
			}

			if (AEnvironmentTimeOfDayActor* LegacyActor = Cast<AEnvironmentTimeOfDayActor>(Candidate))
			{
				++LegacyActorCount;
				if (!FirstLegacyActor)
				{
					FirstLegacyActor = LegacyActor;
				}
			}
		}

		if (GameTimeActorCount > 1)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple AGameTimeOfDayActor instances detected. ADynamicSky '%s' uses first discovered actor: %s"), *GetName(), *GetNameSafe(FirstGameTimeActor));
		}
		if (GameTimeActorCount == 0 && LegacyActorCount > 1)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple legacy AEnvironmentTimeOfDayActor providers detected. ADynamicSky '%s' uses first discovered actor: %s"), *GetName(), *GetNameSafe(FirstLegacyActor));
		}

		BoundGameTimeOfDayActor = FirstGameTimeActor;
		BoundLegacyTimeOfDayActor = BoundGameTimeOfDayActor ? nullptr : FirstLegacyActor;
		TimeOfDaySourceActor = BoundGameTimeOfDayActor ? static_cast<AActor*>(BoundGameTimeOfDayActor) : static_cast<AActor*>(BoundLegacyTimeOfDayActor);
	}

	if (BoundGameTimeOfDayActor)
	{
		BoundGameTimeOfDayActor->OnGameTimeOfDayChanged.AddDynamic(this, &ADynamicSky::HandleGameTimeOfDayChanged);
		HandleProviderHour(BoundGameTimeOfDayActor->GetCurrentHour24());
		return true;
	}

	if (BoundLegacyTimeOfDayActor)
	{
		BoundLegacyTimeOfDayActor->OnTimeOfDayChanged.AddDynamic(this, &ADynamicSky::HandleLegacyTimeOfDayChanged);
		HandleProviderHour(BoundLegacyTimeOfDayActor->GetCurrentHour24());
		return true;
	}

	return false;
}

void ADynamicSky::UnbindTimeSource()
{
	if (BoundGameTimeOfDayActor)
	{
		BoundGameTimeOfDayActor->OnGameTimeOfDayChanged.RemoveDynamic(this, &ADynamicSky::HandleGameTimeOfDayChanged);
		BoundGameTimeOfDayActor = nullptr;
	}

	if (BoundLegacyTimeOfDayActor)
	{
		BoundLegacyTimeOfDayActor->OnTimeOfDayChanged.RemoveDynamic(this, &ADynamicSky::HandleLegacyTimeOfDayChanged);
		BoundLegacyTimeOfDayActor = nullptr;
	}
}

void ADynamicSky::HandleProviderHour(float Hour24)
{
	ApplySkyState(Hour24);
}

void ADynamicSky::HandleGameTimeOfDayChanged(float Hour24)
{
	HandleProviderHour(Hour24);
}

void ADynamicSky::HandleLegacyTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)
{
	(void)VisualState;
	HandleProviderHour(Hour24);
}

FDynamicSkyState ADynamicSky::EvaluateSkyState(float Hour24) const
{
	FDynamicSkyState SkyState;
	SkyState.Hour24 = NormalizeHour24(Hour24);

	const float NormalizedSunrise = NormalizeHour24(SunriseHour);
	const float NormalizedSunset = NormalizeHour24(SunsetHour);
	const float DayDuration = FMath::Max(FMath::Fmod((NormalizedSunset - NormalizedSunrise + 24.0f), 24.0f), MinDuration);
	const float NightDuration = FMath::Max(24.0f - DayDuration, MinDuration);
	const float ClampedGap = FMath::Min(GapTime, 11.999f);
	const float SunElapsedHours = CalculateOrbitElapsedHours(SkyState.Hour24, NormalizedSunrise, DayDuration, ClampedGap);
	const float MoonElapsedHours = CalculateOrbitElapsedHours(SkyState.Hour24, NormalizedSunset, NightDuration, ClampedGap);
	const float SunAlpha = SunElapsedHours / DayDuration;
	const float MoonAlpha = MoonElapsedHours / NightDuration;

	SkyState.SunWorldRotation = FRotator(FMath::Lerp(0.0f, -180.0f, SunAlpha), OrbitYaw, 0.0f);
	SkyState.MoonWorldRotation = FRotator(FMath::Lerp(0.0f, -180.0f, MoonAlpha), OrbitYaw, 0.0f);

	SkyState.bSunLightVisible = IsTimeWithinWrappedRange(SkyState.Hour24, NormalizedSunrise - ClampedGap, NormalizedSunset + ClampedGap);
	SkyState.bMoonLightVisible = !SkyState.bSunLightVisible;

	SkyState.RayleighScattering = EvaluateRayleighScattering(SkyState.Hour24);
	SkyState.MultiScattering = EvaluateMultiScattering(SkyState.Hour24);

	if (SkyState.bSunLightVisible)
	{
		SkyState.IsStarVisible = 0.0f;
	}
	else
	{
		SkyState.IsStarVisible = 1.0f;
	}
	SkyState.IsMoonVisible = 1.0f;

	return SkyState;
}

void ADynamicSky::ApplySkyState(float Hour24)
{
	const FDynamicSkyState SkyState = EvaluateSkyState(Hour24);
	ApplySkyStateInternal(SkyState);
}

void ADynamicSky::ApplySkyStateInternal(const FDynamicSkyState& SkyState)
{
	if (SunDirectionalLight)
	{
		SunDirectionalLight->SetWorldRotation(SkyState.SunWorldRotation);
	}

	if (MoonDirectionalLight)
	{
		MoonDirectionalLight->SetWorldRotation(SkyState.MoonWorldRotation);
	}

	if (SkyAtmosphere)
	{
		SkyAtmosphere->SetRayleighScattering(SkyState.RayleighScattering);
		SkyAtmosphere->SetMultiScatteringFactor(FMath::Max(0.0f, SkyState.MultiScattering));
	}

	if (SkySphereMID)
	{
		SkySphereMID->SetScalarParameterValue(ParamIsStarVisible, SkyState.IsStarVisible);
		SkySphereMID->SetScalarParameterValue(ParamIsMoonVisible, SkyState.IsMoonVisible);
	}
}

void ADynamicSky::RefreshSkySphereMaterial()
{
	if (!SkySphereMesh)
	{
		return;
	}

	if (SkySphereMeshAsset)
	{
		SkySphereMesh->SetStaticMesh(SkySphereMeshAsset);
	}

	if (!SkySphereMaterial)
	{
		SkySphereMID = nullptr;
		return;
	}

	SkySphereMesh->SetMaterial(0, SkySphereMaterial);
	SkySphereMID = UMaterialInstanceDynamic::Create(SkySphereMaterial, this);
	if (SkySphereMID)
	{
		SkySphereMesh->SetMaterial(0, SkySphereMID);
	}
}

bool ADynamicSky::ShouldUseEditorPreview() const
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
