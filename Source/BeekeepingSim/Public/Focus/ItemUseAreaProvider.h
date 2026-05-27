#pragma once

#include "CoreMinimal.h"
#include "Focus/CursorItemUseAreaTypes.h"
#include "UObject/Interface.h"
#include "ItemUseAreaProvider.generated.h"

// Deprecated: runtime item-use-area descriptor collection uses UItemUseAreaMeshProviderComponent.
UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UItemUseAreaProvider : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IItemUseAreaProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Use Area")
	void GetItemUseAreaDescriptors(TArray<FItemUseAreaDescriptor>& OutDescriptors) const;
};
