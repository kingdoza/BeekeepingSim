#include "Environment/GameTimeOfDayActor.h"

#include "EngineUtils.h"
#include "Environment/DynamicSky.h"
#include "Environment/EnvironmentTimeOfDayActor.h"
#include "Environment/GameTimeBucketSubsystem.h"
#include "Engine/World.h"

namespace
{
	constexpr float MinDayLengthSeconds = 1.0f;
}

AGameTimeOfDayActor::AGameTimeOfDayActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGameTimeOfDayActor::BeginPlay()
{
	Super::BeginPlay();

	float StartHour24 = bStartPlayFromPreviewHour ? PreviewStartHour24 : InitialHour24;
	float DynamicSkyPreviewHour24 = 0.0f;
	if (ResolveDynamicSkyPreviewStartHour(DynamicSkyPreviewHour24))
	{
		StartHour24 = DynamicSkyPreviewHour24;
	}
	CurrentHour24 = NormalizeHour24(StartHour24);

	if (UWorld* World = GetWorld())
	{
		if (UGameTimeBucketSubsystem* BucketSubsystem = World->GetSubsystem<UGameTimeBucketSubsystem>())
		{
			BucketSubsystem->SetTimeOfDayProvider(this);
		}
	}

	BroadcastCurrentHour();
}

void AGameTimeOfDayActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTimeProgressionEnabled)
	{
		return;
	}

	if (DayLengthSeconds < MinDayLengthSeconds)
	{
		if (!bHasLoggedInvalidDayLength)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("DayLengthSeconds is invalid (%.3f) on '%s'. Use value >= 1.0."), DayLengthSeconds, *GetName());
			bHasLoggedInvalidDayLength = true;
		}
		return;
	}

	bHasLoggedInvalidDayLength = false;
	const float HoursPerSecond = 24.0f / DayLengthSeconds;
	CurrentHour24 = NormalizeHour24(CurrentHour24 + (DeltaTime * HoursPerSecond));
	BroadcastCurrentHour();
}

float AGameTimeOfDayActor::GetCurrentHour24_Implementation() const
{
	return CurrentHour24;
}

void AGameTimeOfDayActor::SetCurrentHour24(float NewHour)
{
	CurrentHour24 = NormalizeHour24(NewHour);
	BroadcastCurrentHour();
}

void AGameTimeOfDayActor::SetTimeProgressionEnabled(bool bEnabled)
{
	bTimeProgressionEnabled = bEnabled;
}

void AGameTimeOfDayActor::SetDayLengthSeconds(float NewDayLengthSeconds)
{
	DayLengthSeconds = NewDayLengthSeconds;
}

float AGameTimeOfDayActor::NormalizeHour24(float Hour24)
{
	const float Wrapped = FMath::Fmod(Hour24, 24.0f);
	return Wrapped < 0.0f ? Wrapped + 24.0f : Wrapped;
}

void AGameTimeOfDayActor::BroadcastCurrentHour()
{
	OnGameTimeOfDayChanged.Broadcast(CurrentHour24);
}

bool AGameTimeOfDayActor::ResolveDynamicSkyPreviewStartHour(float& OutHour24) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const ADynamicSky* FirstPreviewSky = nullptr;
	int32 PreviewSkyCount = 0;
	for (TActorIterator<ADynamicSky> It(World); It; ++It)
	{
		const ADynamicSky* DynamicSky = *It;
		if (!IsValid(DynamicSky) || !DynamicSky->ShouldStartGameTimeFromPreviewHour())
		{
			continue;
		}

		++PreviewSkyCount;
		if (!FirstPreviewSky)
		{
			FirstPreviewSky = DynamicSky;
		}
	}

	if (!FirstPreviewSky)
	{
		return false;
	}

	if (PreviewSkyCount > 1)
	{
		UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple ADynamicSky actors request preview start hour. AGameTimeOfDayActor '%s' uses first discovered actor: %s"), *GetName(), *GetNameSafe(FirstPreviewSky));
	}

	OutHour24 = FirstPreviewSky->GetPreviewHour24();
	return true;
}
