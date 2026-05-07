#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QueenBeeActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AQueenBeeActor : public AActor
{
	GENERATED_BODY()

public:
	AQueenBeeActor();

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> QueenBeeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Motion", meta = (ClampMin = "0.0"))
	float YawJitterDegreesPerTick = 1.0f;
};
