#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemActionContext.generated.h"

class ABeekeeperCharacter;
class APlayerController;
class AActor;
class UFocusTargetComponent;
class UPrimitiveComponent;
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

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<AActor> FocusEngagedHostActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	FName ItemUseAreaId;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	FGameplayTagContainer ItemUseAreaTags;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<UPrimitiveComponent> ItemUseAreaHitComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item Action")
	TObjectPtr<UObject> ItemUseEffectTargetObject = nullptr;
};
