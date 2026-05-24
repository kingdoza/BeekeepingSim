#include "Environment/GameTimeBucketSubsystem.h"

#include "Environment/EnvironmentTimeOfDayActor.h"
#include "Environment/GameTimeOfDayActor.h"
#include "Environment/GameTimeBucketListener.h"
#include "Environment/TimeOfDayProvider.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void UGameTimeBucketSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	RefreshListeners();
	EnsureTimeProviderBound();
}

void UGameTimeBucketSubsystem::Deinitialize()
{
	UnbindTimeProviderActor();
	RegisteredSubscriptions.Reset();
	Super::Deinitialize();
}

void UGameTimeBucketSubsystem::SetTimeOfDayActor(AEnvironmentTimeOfDayActor* InTimeOfDayActor)
{
	SetTimeOfDayProvider(InTimeOfDayActor);
}

void UGameTimeBucketSubsystem::SetTimeOfDayProvider(AActor* InTimeOfDayProviderActor)
{
	// Keep canonical source stable: legacy provider cannot override an already bound game-time actor.
	if (BoundGameTimeActor && InTimeOfDayProviderActor && !InTimeOfDayProviderActor->IsA<AGameTimeOfDayActor>())
	{
		return;
	}

	BindTimeProviderActor(InTimeOfDayProviderActor);
	if (IsValid(TimeOfDayProviderActor) && TimeOfDayProviderActor->GetClass()->ImplementsInterface(UTimeOfDayProvider::StaticClass()))
	{
		const float CurrentHour24 = ITimeOfDayProvider::Execute_GetCurrentHour24(TimeOfDayProviderActor);
		ProcessSubscriptionsForCurrentTime(CurrentHour24, false);
	}
}

void UGameTimeBucketSubsystem::RegisterListener(AActor* ListenerActor)
{
	if (!IsValid(ListenerActor) || !ListenerActor->GetClass()->ImplementsInterface(UGameTimeBucketListener::StaticClass()))
	{
		return;
	}

	RemoveListenerEntries(ListenerActor);

	TArray<FGameTimeBucketSubscription> Subscriptions;
	IGameTimeBucketListener::Execute_GetGameTimeBucketSubscriptions(ListenerActor, Subscriptions);
	for (const FGameTimeBucketSubscription& Subscription : Subscriptions)
	{
		FRegisteredGameTimeBucketSubscription Entry;
		Entry.Listener = ListenerActor;
		Entry.Subscription = Subscription;
		Entry.Subscription.BucketMinutes = ClampBucketMinutes(Entry.Subscription.BucketMinutes);
		RegisteredSubscriptions.Add(MoveTemp(Entry));
	}

	if (IsValid(TimeOfDayProviderActor) && TimeOfDayProviderActor->GetClass()->ImplementsInterface(UTimeOfDayProvider::StaticClass()))
	{
		const float CurrentHour24 = ITimeOfDayProvider::Execute_GetCurrentHour24(TimeOfDayProviderActor);
		ProcessSubscriptionsForCurrentTime(CurrentHour24, false);
	}
}

void UGameTimeBucketSubsystem::UnregisterListener(AActor* ListenerActor)
{
	RemoveListenerEntries(ListenerActor);
}

void UGameTimeBucketSubsystem::RefreshListeners()
{
	RemoveInvalidEntries();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}
		if (!Actor->GetClass()->ImplementsInterface(UGameTimeBucketListener::StaticClass()))
		{
			continue;
		}
		RegisterListener(Actor);
	}
}

void UGameTimeBucketSubsystem::HandleTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)
{
	const int32 MinuteOfDay = HourToMinuteOfDay(Hour24);
	bool bWrappedDay = false;

	if (LastObservedMinuteOfDay != INDEX_NONE && MinuteOfDay < LastObservedMinuteOfDay)
	{
		++CurrentDayOffset;
		bWrappedDay = true;
	}

	LastObservedMinuteOfDay = MinuteOfDay;
	ProcessSubscriptionsForCurrentTime(Hour24, bWrappedDay);
}

bool UGameTimeBucketSubsystem::EnsureTimeProviderBound()
{
	if (IsValid(TimeOfDayProviderActor))
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	AActor* FoundActor = FindCanonicalTimeProviderActor(World, true);

	if (FoundActor)
	{
		BindTimeProviderActor(FoundActor);
		return true;
	}

	return false;
}

AActor* UGameTimeBucketSubsystem::FindCanonicalTimeProviderActor(UWorld* World, bool bLogIfMultiple) const
{
	if (!World)
	{
		return nullptr;
	}

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

	if (bLogIfMultiple)
	{
		if (GameTimeActorCount > 1)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple AGameTimeOfDayActor instances detected for bucket subsystem. Using first discovered actor: %s"), *GetNameSafe(FirstGameTimeActor));
		}
		if (GameTimeActorCount == 0 && LegacyActorCount > 1)
		{
			UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Multiple legacy AEnvironmentTimeOfDayActor providers detected for bucket subsystem. Using first discovered actor: %s"), *GetNameSafe(FirstLegacyActor));
		}
	}

	return FirstGameTimeActor ? static_cast<AActor*>(FirstGameTimeActor) : static_cast<AActor*>(FirstLegacyActor);
}

void UGameTimeBucketSubsystem::BindTimeProviderActor(AActor* InActor)
{
	if (TimeOfDayProviderActor == InActor)
	{
		return;
	}

	UnbindTimeProviderActor();
	TimeOfDayProviderActor = InActor;
	CurrentDayOffset = 0;
	LastObservedMinuteOfDay = INDEX_NONE;

	if (!IsValid(TimeOfDayProviderActor))
	{
		return;
	}

	BoundGameTimeActor = Cast<AGameTimeOfDayActor>(TimeOfDayProviderActor);
	if (BoundGameTimeActor)
	{
		BoundGameTimeActor->OnGameTimeOfDayChanged.AddDynamic(this, &UGameTimeBucketSubsystem::HandleGameTimeOfDayChanged);
		return;
	}

	BoundLegacyEnvironmentActor = Cast<AEnvironmentTimeOfDayActor>(TimeOfDayProviderActor);
	if (BoundLegacyEnvironmentActor)
	{
		BoundLegacyEnvironmentActor->OnTimeOfDayChanged.AddDynamic(this, &UGameTimeBucketSubsystem::HandleTimeOfDayChanged);
	}
}

void UGameTimeBucketSubsystem::UnbindTimeProviderActor()
{
	if (BoundGameTimeActor)
	{
		BoundGameTimeActor->OnGameTimeOfDayChanged.RemoveDynamic(this, &UGameTimeBucketSubsystem::HandleGameTimeOfDayChanged);
		BoundGameTimeActor = nullptr;
	}

	if (BoundLegacyEnvironmentActor)
	{
		BoundLegacyEnvironmentActor->OnTimeOfDayChanged.RemoveDynamic(this, &UGameTimeBucketSubsystem::HandleTimeOfDayChanged);
		BoundLegacyEnvironmentActor = nullptr;
	}
	TimeOfDayProviderActor = nullptr;
}

void UGameTimeBucketSubsystem::HandleGameTimeOfDayChanged(float Hour24)
{
	HandleTimeOfDayChanged(Hour24, FTimeOfDayVisualState());
}

void UGameTimeBucketSubsystem::RemoveListenerEntries(AActor* ListenerActor)
{
	RegisteredSubscriptions.RemoveAll([ListenerActor](const FRegisteredGameTimeBucketSubscription& Entry)
	{
		return Entry.Listener.Get() == ListenerActor;
	});
}

void UGameTimeBucketSubsystem::RemoveInvalidEntries()
{
	RegisteredSubscriptions.RemoveAll([](const FRegisteredGameTimeBucketSubscription& Entry)
	{
		return !Entry.Listener.IsValid();
	});
}

void UGameTimeBucketSubsystem::ProcessSubscriptionsForCurrentTime(float Hour24, bool bWrappedDay)
{
	RemoveInvalidEntries();

	const int32 MinuteOfDay = HourToMinuteOfDay(Hour24);
	const int32 AbsoluteMinute = (CurrentDayOffset * 1440) + MinuteOfDay;
	for (FRegisteredGameTimeBucketSubscription& Entry : RegisteredSubscriptions)
	{
		ProcessSubscription(Entry, Hour24, AbsoluteMinute, MinuteOfDay, bWrappedDay);
	}
}

void UGameTimeBucketSubsystem::ProcessSubscription(FRegisteredGameTimeBucketSubscription& Entry, float Hour24, int32 AbsoluteMinute, int32 MinuteOfDay, bool bWrappedDay)
{
	AActor* ListenerActor = Entry.Listener.Get();
	if (!ListenerActor)
	{
		return;
	}

	const int32 BucketMinutes = ClampBucketMinutes(Entry.Subscription.BucketMinutes);
	const int32 CurrentAbsoluteBucketStart = (AbsoluteMinute / BucketMinutes) * BucketMinutes;
	const int32 CurrentBucketIndex = MinuteOfDay / BucketMinutes;

	if (!Entry.bHasApplied)
	{
		if (Entry.Subscription.bApplyImmediatelyOnBeginPlay)
		{
			DispatchEvent(Entry, CurrentAbsoluteBucketStart, Hour24, bWrappedDay, true, false);
		}
		Entry.LastAbsoluteBucketStartMinute = CurrentAbsoluteBucketStart;
		Entry.LastBucketIndex = CurrentBucketIndex;
		Entry.bHasApplied = true;
		return;
	}

	if (CurrentAbsoluteBucketStart == Entry.LastAbsoluteBucketStartMinute)
	{
		return;
	}

	if (Entry.Subscription.CatchUpPolicy == EGameTimeBucketCatchUpPolicy::CatchUp && Entry.LastAbsoluteBucketStartMinute != INDEX_NONE)
	{
		int32 CatchUpCount = 0;
		for (int32 NextBucketStart = Entry.LastAbsoluteBucketStartMinute + BucketMinutes; NextBucketStart <= CurrentAbsoluteBucketStart; NextBucketStart += BucketMinutes)
		{
			if (++CatchUpCount > MaxCatchUpEventsPerSubscription)
			{
				UE_LOG(LogBeekeepingEnvironment, Warning, TEXT("Catch-up event limit exceeded for listener '%s'. Falling back to latest bucket."), *GetNameSafe(ListenerActor));
				DispatchEvent(Entry, CurrentAbsoluteBucketStart, Hour24, bWrappedDay, false, false);
				break;
			}

			const int32 CatchUpMinuteOfDay = NextBucketStart % 1440;
			const float CatchUpHour24 = MinuteOfDayToHour24(CatchUpMinuteOfDay);
			const bool bCatchUpWrappedDay = (CatchUpMinuteOfDay == 0);
			DispatchEvent(Entry, NextBucketStart, CatchUpHour24, bCatchUpWrappedDay, false, true);
		}
	}
	else
	{
		DispatchEvent(Entry, CurrentAbsoluteBucketStart, Hour24, bWrappedDay, false, false);
	}

	Entry.LastAbsoluteBucketStartMinute = CurrentAbsoluteBucketStart;
	Entry.LastBucketIndex = CurrentBucketIndex;
	Entry.bHasApplied = true;
}

void UGameTimeBucketSubsystem::DispatchEvent(FRegisteredGameTimeBucketSubscription& Entry, int32 AbsoluteBucketStartMinute, float Hour24, bool bWrappedDay, bool bInitialApply, bool bCatchUp)
{
	AActor* ListenerActor = Entry.Listener.Get();
	if (!ListenerActor)
	{
		return;
	}

	FGameTimeBucketEvent Event;
	BuildEvent(Event, Entry, AbsoluteBucketStartMinute, Hour24, bWrappedDay, bInitialApply, bCatchUp);
	IGameTimeBucketListener::Execute_OnGameTimeBucketEvent(ListenerActor, Event);
}

void UGameTimeBucketSubsystem::BuildEvent(FGameTimeBucketEvent& OutEvent, const FRegisteredGameTimeBucketSubscription& Entry, int32 AbsoluteBucketStartMinute, float Hour24, bool bWrappedDay, bool bInitialApply, bool bCatchUp) const
{
	const int32 BucketMinutes = ClampBucketMinutes(Entry.Subscription.BucketMinutes);
	const int32 MinuteOfDay = ((AbsoluteBucketStartMinute % 1440) + 1440) % 1440;
	const int32 BucketIndex = MinuteOfDay / BucketMinutes;
	const int32 BucketStartMinute = BucketIndex * BucketMinutes;
	const int32 BucketEndMinute = FMath::Min(BucketStartMinute + BucketMinutes, 1440);

	OutEvent.Hour24 = NormalizeHour24(Hour24);
	OutEvent.BucketMinutes = BucketMinutes;
	OutEvent.BucketIndex = BucketIndex;
	OutEvent.BucketStartMinute = BucketStartMinute;
	OutEvent.BucketEndMinute = BucketEndMinute;
	OutEvent.bWrappedDay = bWrappedDay;
	OutEvent.bInitialApply = bInitialApply;
	OutEvent.bCatchUp = bCatchUp;
	OutEvent.SubscriptionTag = Entry.Subscription.SubscriptionTag;
}

int32 UGameTimeBucketSubsystem::ClampBucketMinutes(int32 BucketMinutes)
{
	return FMath::Clamp(BucketMinutes, 1, 1440);
}

float UGameTimeBucketSubsystem::NormalizeHour24(float Hour24)
{
	const float Wrapped = FMath::Fmod(Hour24, 24.0f);
	return Wrapped < 0.0f ? Wrapped + 24.0f : Wrapped;
}

int32 UGameTimeBucketSubsystem::HourToMinuteOfDay(float Hour24)
{
	const float NormalizedHour = NormalizeHour24(Hour24);
	const int32 TotalMinutes = FMath::FloorToInt(NormalizedHour * 60.0f);
	return ((TotalMinutes % 1440) + 1440) % 1440;
}

float UGameTimeBucketSubsystem::MinuteOfDayToHour24(int32 MinuteOfDay)
{
	const int32 NormalizedMinute = ((MinuteOfDay % 1440) + 1440) % 1440;
	return static_cast<float>(NormalizedMinute) / 60.0f;
}
