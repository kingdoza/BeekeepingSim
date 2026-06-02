#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "ActiveUseDurabilityItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EActiveUseDurabilityDrainPolicy : uint8
{
	WhenUseEffectSucceeded,
	WhileOverValidUseArea,
	WhileUseSessionActive
};

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UActiveUseDurabilityItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Active Use Durability", meta = (ClampMin = "0.0"))
	float DurabilityDrainPerSecond = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Active Use Durability")
	EActiveUseDurabilityDrainPolicy DrainPolicy = EActiveUseDurabilityDrainPolicy::WhenUseEffectSucceeded;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Active Use Durability")
	bool bRemoveItemWhenDepleted = true;

	// Kept only so early saved assets can still load their old value. DrainPolicy is the source of truth.
	UPROPERTY()
	bool bRequireEffectSucceeded = true;
};
