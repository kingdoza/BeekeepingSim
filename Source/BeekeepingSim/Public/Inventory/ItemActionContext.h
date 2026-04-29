#pragma once

#include "CoreMinimal.h"
#include "ItemActionContext.generated.h"

class ABeekeeperCharacter;
class APlayerController;
class UFocusTargetComponent;
class UWorld;

USTRUCT(BlueprintType)
struct FItemActionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<ABeekeeperCharacter> Character = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<UWorld> World = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<UFocusTargetComponent> FocusTarget = nullptr;
};
