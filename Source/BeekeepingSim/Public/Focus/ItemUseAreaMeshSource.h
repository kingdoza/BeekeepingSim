#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemUseAreaMeshSource.generated.h"

class UItemUseAreaMeshComponent;

UINTERFACE()
class BEEKEEPINGSIM_API UItemUseAreaMeshSource : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IItemUseAreaMeshSource
{
	GENERATED_BODY()

public:
	virtual void GetProvidedItemUseAreaMeshes(TArray<UItemUseAreaMeshComponent*>& OutMeshes) const {}
};

