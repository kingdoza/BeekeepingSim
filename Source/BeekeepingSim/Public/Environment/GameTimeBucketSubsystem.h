#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Environment/GameTimeBucketTypes.h"
#include "GameTimeBucketSubsystem.generated.h"

class AEnvironmentTimeOfDayActor;
class AGameTimeOfDayActor;
struct FTimeOfDayVisualState;

UCLASS()
class BEEKEEPINGSIM_API UGameTimeBucketSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
	void SetTimeOfDayActor(AEnvironmentTimeOfDayActor* InTimeOfDayActor);

	UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
	void SetTimeOfDayProvider(AActor* InTimeOfDayProviderActor);

	UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
	void RegisterListener(AActor* ListenerActor);

	UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
	void UnregisterListener(AActor* ListenerActor);

	UFUNCTION(BlueprintCallable, Category = "Game Time Bucket")
	void RefreshListeners();

private:
	struct FRegisteredGameTimeBucketSubscription
	{
		TWeakObjectPtr<AActor> Listener;
		FGameTimeBucketSubscription Subscription;
		int32 LastBucketIndex = INDEX_NONE;
		int32 LastAbsoluteBucketStartMinute = INDEX_NONE;
		bool bHasApplied = false;
	};

	UFUNCTION()
	void HandleTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState);
	UFUNCTION()
	void HandleGameTimeOfDayChanged(float Hour24);

	bool EnsureTimeProviderBound();
	AActor* FindCanonicalTimeProviderActor(UWorld* World, bool bLogIfMultiple) const;
	void BindTimeProviderActor(AActor* InActor);
	void UnbindTimeProviderActor();
	void RemoveListenerEntries(AActor* ListenerActor);
	void RemoveInvalidEntries();
	void ProcessSubscriptionsForCurrentTime(float Hour24, bool bWrappedDay);
	void ProcessSubscription(FRegisteredGameTimeBucketSubscription& Entry, float Hour24, int32 AbsoluteMinute, int32 MinuteOfDay, bool bWrappedDay);
	void DispatchEvent(FRegisteredGameTimeBucketSubscription& Entry, int32 AbsoluteBucketStartMinute, float Hour24, bool bWrappedDay, bool bInitialApply, bool bCatchUp);
	void BuildEvent(FGameTimeBucketEvent& OutEvent, const FRegisteredGameTimeBucketSubscription& Entry, int32 AbsoluteBucketStartMinute, float Hour24, bool bWrappedDay, bool bInitialApply, bool bCatchUp) const;
	static int32 ClampBucketMinutes(int32 BucketMinutes);
	static float NormalizeHour24(float Hour24);
	static int32 HourToMinuteOfDay(float Hour24);
	static float MinuteOfDayToHour24(int32 MinuteOfDay);

	UPROPERTY()
	TObjectPtr<AActor> TimeOfDayProviderActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AGameTimeOfDayActor> BoundGameTimeActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AEnvironmentTimeOfDayActor> BoundLegacyEnvironmentActor = nullptr;

	TArray<FRegisteredGameTimeBucketSubscription> RegisteredSubscriptions;
	int32 CurrentDayOffset = 0;
	int32 LastObservedMinuteOfDay = INDEX_NONE;
	static constexpr int32 MaxCatchUpEventsPerSubscription = 512;
};

