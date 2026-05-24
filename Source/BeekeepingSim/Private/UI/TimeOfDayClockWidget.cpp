#include "UI/TimeOfDayClockWidget.h"

void UTimeOfDayClockWidget::SetHour24(float InHour24)
{
	CurrentHour24 = NormalizeHour24(InHour24);

	const int32 TotalMinutes = Hour24ToTotalMinutes(CurrentHour24);
	if (TotalMinutes == LastDisplayedTotalMinutes)
	{
		return;
	}

	LastDisplayedTotalMinutes = TotalMinutes;

	const int32 Hour = TotalMinutes / 60;
	const int32 Minute = TotalMinutes % 60;
	OnDisplayedTimeChanged(FormatHour24AsHHMM(CurrentHour24), Hour, Minute);
}

FText UTimeOfDayClockWidget::GetFormattedTimeText() const
{
	return FormatHour24AsHHMM(CurrentHour24);
}

FText UTimeOfDayClockWidget::FormatHour24AsHHMM(float Hour24)
{
	const int32 TotalMinutes = Hour24ToTotalMinutes(Hour24);
	const int32 Hour = TotalMinutes / 60;
	const int32 Minute = TotalMinutes % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Hour, Minute));
}

float UTimeOfDayClockWidget::NormalizeHour24(float Hour24)
{
	const float Wrapped = FMath::Fmod(Hour24, 24.0f);
	return Wrapped < 0.0f ? Wrapped + 24.0f : Wrapped;
}

int32 UTimeOfDayClockWidget::Hour24ToTotalMinutes(float Hour24)
{
	const float NormalizedHour = NormalizeHour24(Hour24);
	return FMath::FloorToInt(NormalizedHour * 60.0f) % 1440;
}
