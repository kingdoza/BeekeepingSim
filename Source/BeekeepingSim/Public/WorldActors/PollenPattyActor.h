#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PollenPattyActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API APollenPattyActor : public AActor
{
	GENERATED_BODY()

public:
	APollenPattyActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PattyMesh;
};
