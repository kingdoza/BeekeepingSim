#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BeehiveCombActor.generated.h"

class UNiagaraComponent;
class USceneComponent;
class UStaticMeshComponent;
class UCursorPartFocusActionComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeehiveCombActor : public AActor
{
	GENERATED_BODY()

public:
	ABeehiveCombActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ApplyCombBeeParameters(const FVector2D& InPlaneSize, int32 InSpawnAmount, int32 InTargetBeeCount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetSpawnAmountAndResetTargetBeeCount(const FVector2D& InPlaneSize, int32 InSpawnAmount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetTargetBeeCount(int32 NewTargetBeeCount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ResetTargetBeeCountToSpawnAmount();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceTargetBeeCountByRatio(float Ratio);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceTargetBeeCountByAmount(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetSpawnAmount() const { return SpawnAmount; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetTargetBeeCount() const { return TargetBeeCount; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	UStaticMeshComponent* GetCombMeshComponent() const { return CombMesh; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	UCursorPartFocusActionComponent* GetPartFocusActionComponent() const { return PartFocusAction; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CombMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FrontFaceBeeNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> BackFaceBeeNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCursorPartFocusActionComponent> PartFocusAction;

private:
	void ApplyNiagaraUserParameters();
	void SanitizeState();

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb")
	FVector2D PlaneSize = FVector2D(100.0f, 100.0f);

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 SpawnAmount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 TargetBeeCount = 0;
};
