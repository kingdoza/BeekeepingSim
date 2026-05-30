#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlacedItemRemainingVisualComponent.generated.h"

class APlacedItemActor;
class UPlacedItemRemainingComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UPlacedItemRemainingVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining Visual")
	virtual void InitializeRemainingVisual(APlacedItemActor* InPlacedItemActor, UPlacedItemRemainingComponent* InRemainingComponent);

	UFUNCTION(BlueprintCallable, Category = "Placed Item|Remaining Visual")
	virtual void ApplyRemainingRatio(float Ratio);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Placed Item|Remaining Visual")
	void ReceiveRemainingRatioChanged(float Ratio);

	UFUNCTION()
	void HandleRemainingRatioChanged(float Ratio);

	APlacedItemActor* GetPlacedItemActor() const { return PlacedItemActor.Get(); }
	UPlacedItemRemainingComponent* GetRemainingComponent() const { return RemainingComponent.Get(); }

private:
	UPROPERTY(Transient)
	TObjectPtr<APlacedItemActor> PlacedItemActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPlacedItemRemainingComponent> RemainingComponent = nullptr;
};

