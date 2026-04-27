#pragma once

#include "CoreMinimal.h"
#include "HotbarPresentationTypes.generated.h"

UENUM(BlueprintType)
enum class EHotbarPresentationMode : uint8
{
	None,
	InHand,
	OnCursor
};
