#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Focus/CursorPartFocusScopeComponent.h"
#include "CursorPartFocusProvider.generated.h"

UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UCursorPartFocusProvider : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API ICursorPartFocusProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cursor Part Focus")
	void GetCursorPartFocusDescriptors(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const;
};

