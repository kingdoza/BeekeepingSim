#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BeekeepingSimFocusSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Beekeeping Sim Focus"))
class BEEKEEPINGSIM_API UBeekeepingSimFocusSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus", meta = (ClampMin = "0.0"))
	float ScreenEdgeCancelRegionThickness = 64.0f;
};
