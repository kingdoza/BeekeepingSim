#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimeOfDayClockWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UTimeOfDayClockWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Time Of Day")
	void SetHour24(float InHour24);

	UFUNCTION(BlueprintPure, Category = "Time Of Day")
	float GetCurrentHour24() const { return CurrentHour24; }

	UFUNCTION(BlueprintPure, Category = "Time Of Day")
	FText GetFormattedTimeText() const;

	UFUNCTION(BlueprintPure, Category = "Time Of Day")
	static FText FormatHour24AsHHMM(float Hour24);

	UFUNCTION(BlueprintImplementableEvent, Category = "Time Of Day")
	void OnDisplayedTimeChanged(const FText& NewTimeText, int32 Hour, int32 Minute);

private:
	static float NormalizeHour24(float Hour24);
	static int32 Hour24ToTotalMinutes(float Hour24);

private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Time Of Day", meta = (AllowPrivateAccess = "true"))
	float CurrentHour24 = 12.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Time Of Day", meta = (AllowPrivateAccess = "true"))
	int32 LastDisplayedTotalMinutes = INDEX_NONE;
};
