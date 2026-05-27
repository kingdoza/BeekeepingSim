#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemUseAreaActivationProvider.generated.h"

class UItemUseAreaMeshComponent;

UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UItemUseAreaActivationProvider : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IItemUseAreaActivationProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Use Area")
	bool IsItemUseAreaMeshActive(UItemUseAreaMeshComponent* Component, AActor* HostActor) const;
};

