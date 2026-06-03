#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QueenBeeActor.generated.h"

class USceneComponent;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AQueenBeeActor : public AActor
{
	GENERATED_BODY()

public:
	AQueenBeeActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Queen Bee|Colony")
	float GetBaseEggLayingPower() const { return BaseEggLayingPower; }

	UFUNCTION(BlueprintCallable, Category = "Queen Bee|Disease")
	void SetDiseaseValue(float NewDiseaseValue);

	UFUNCTION(BlueprintPure, Category = "Queen Bee|Disease")
	float GetDiseaseValue() const { return DiseaseValue; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> QueenBeeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Motion", meta = (ClampMin = "0.0"))
	float YawJitterDegreesPerTick = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Colony", meta = (ClampMin = "0.0"))
	float BaseEggLayingPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Disease")
	FName DiseaseMaterialParameterName = TEXT("Disease");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Queen Bee|Disease", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DiseaseValue = 0.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DiseaseMaterialInstances;

private:
	void EnsureDiseaseMaterialInstances();
	void ApplyDiseaseMaterialParameter();
};
