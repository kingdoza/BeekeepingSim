#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldActors/BeeSwarmTypes.h"
#include "BeehiveDualSwarmActor.generated.h"

class UNiagaraComponent;
class USceneComponent;
class USplineComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogBeekeepingBeeSwarm, Log, All);

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeehiveDualSwarmActor : public AActor
{
	GENERATED_BODY()

public:
	ABeehiveDualSwarmActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	void ApplyDualSwarmParameters(const FBeehiveDualSwarmNiagaraParameters& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	void ApplySwarmSpline(USplineComponent* InSwarmSpline);

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Bee Swarm")
	void ApplySplineBindings();

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	float GetSplineLength() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> OutgoingNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> IngoingNiagara;

private:
	TWeakObjectPtr<USplineComponent> BoundSwarmSpline;
};
