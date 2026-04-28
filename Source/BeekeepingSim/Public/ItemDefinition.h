#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemActionTypes.h"
#include "ItemDefinition.generated.h"

class UTexture2D;
class UStaticMesh;
class AItemPresentationActor;

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FPrimaryAssetId ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMesh> WorldMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Presentation")
	TSubclassOf<AItemPresentationActor> HeldPresentationActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Durability")
	bool bUsesDurability = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Durability", meta = (ClampMin = "0.0", EditCondition = "bUsesDurability", EditConditionHides))
	float MaxDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<FItemActionSpec> ActionSpecs;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
