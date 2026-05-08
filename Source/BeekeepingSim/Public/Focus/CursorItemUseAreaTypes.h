#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CursorItemUseAreaTypes.generated.h"

class AActor;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FItemUseAreaVisualSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	FLinearColor UseAreaColor = FLinearColor(0.05f, 0.8f, 0.05f, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area", meta = (ClampMin = "0.0"))
	float UseAreaOpacity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area", meta = (ClampMin = "0.0"))
	float PulseSpeed = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoverStrength = 1.0f;
};

USTRUCT(BlueprintType)
struct FItemUseAreaDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	FName AreaId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	FGameplayTagContainer AreaTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	TArray<TObjectPtr<UPrimitiveComponent>> VisualComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	TObjectPtr<UObject> EffectTargetObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
	FItemUseAreaVisualSettings VisualSettings;
};
