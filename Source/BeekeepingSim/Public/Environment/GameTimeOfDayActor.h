#pragma once

#include "CoreMinimal.h"
#include "Environment/TimeOfDayProvider.h"
#include "GameFramework/Actor.h"
#include "GameTimeOfDayActor.generated.h"

class UGameTimeBucketSubsystem;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AGameTimeOfDayActor : public AActor, public ITimeOfDayProvider
{
	GENERATED_BODY()

public:
	AGameTimeOfDayActor();

	virtual void Tick(float DeltaTime) override;
	virtual float GetCurrentHour24_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	void SetCurrentHour24(float NewHour);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Time Of Day")
	float GetCurrentHour24() const;

	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	void SetTimeProgressionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	void SetDayLengthSeconds(float NewDayLengthSeconds);

	UPROPERTY(BlueprintAssignable, Category = "Time Of Day")
	FOnGameTimeOfDayChangedSignature OnGameTimeOfDayChanged;

protected:
	virtual void BeginPlay() override;

private:
	static float NormalizeHour24(float Hour24);
	void BroadcastCurrentHour();
	bool ResolveDynamicSkyPreviewStartHour(float& OutHour24) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	float InitialHour24 = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	float CurrentHour24 = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float DayLengthSeconds = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Clock", meta = (AllowPrivateAccess = "true"))
	bool bTimeProgressionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Preview", meta = (AllowPrivateAccess = "true"))
	bool bStartPlayFromPreviewHour = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Of Day|Preview", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "24.0"))
	float PreviewStartHour24 = 12.0f;

	bool bHasLoggedInvalidDayLength = false;
};
