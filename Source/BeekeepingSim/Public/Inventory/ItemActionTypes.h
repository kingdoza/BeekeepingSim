#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemActionTypes.generated.h"

class UItemAction;

USTRUCT(BlueprintType)
struct FItemActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	TSubclassOf<UItemAction> ActionClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	FGameplayTag ActionTag;
};

USTRUCT(BlueprintType)
struct FItemActionExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	bool bSucceeded = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	bool bConsumedItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	int32 StackDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	float DurabilityDelta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	FText Message;
};
