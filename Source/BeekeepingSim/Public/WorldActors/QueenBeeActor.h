#pragma once

#include "CoreMinimal.h"
#include "Focus/ItemUseAreaActivationProvider.h"
#include "Inventory/ItemInstance.h"
#include "GameFramework/Actor.h"
#include "QueenBeeActor.generated.h"

class USceneComponent;
class UMaterialInstanceDynamic;
class UItemUseAreaMeshComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AQueenBeeActor : public AActor, public IItemUseAreaActivationProvider
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

	UFUNCTION(BlueprintPure, Category = "Queen Bee|Capture")
	bool IsCaptured() const { return bCaptured; }

	UFUNCTION(BlueprintCallable, Category = "Queen Bee|Capture")
	void SetCaptured(bool bNewCaptured);

	UFUNCTION(BlueprintPure, Category = "Queen Bee|Capture")
	FQueenCageItemState MakeQueenCageItemState() const;

	virtual bool IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> QueenBeeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshComponent> QueenCageUseAreaMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Motion", meta = (ClampMin = "0.0"))
	float YawJitterDegreesPerTick = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Colony", meta = (ClampMin = "0.0"))
	float BaseEggLayingPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Disease")
	FName DiseaseMaterialParameterName = TEXT("Disease");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Queen Bee|Disease", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DiseaseValue = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Queen Bee|Capture")
	bool bCaptured = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DiseaseMaterialInstances;

private:
	void EnsureDiseaseMaterialInstances();
	void ApplyDiseaseMaterialParameter();
	void ApplyQueenCageUseAreaVisualIdleState();
};
