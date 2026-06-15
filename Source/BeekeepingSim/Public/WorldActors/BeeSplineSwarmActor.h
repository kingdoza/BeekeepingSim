#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldActors/BeeSwarmTypes.h"
#include "BeeSplineSwarmActor.generated.h"

class UNiagaraComponent;
class USceneComponent;
class USplineComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeeSplineSwarmActor : public AActor
{
	GENERATED_BODY()

public:
	ABeeSplineSwarmActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	void SetControlMode(EBeeSplineSwarmControlMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	EBeeSplineSwarmControlMode GetControlMode() const { return ControlMode; }

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	void ApplyExternalSwarmParameters(const FBeeSplineSwarmAppliedParameters& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	void StopExternalSwarmEmission();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Bee Swarm")
	void ApplySplineLengthParameter();

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm")
	float GetSplineLength() const;

	// Legacy wrappers kept for Blueprint compatibility.
	UFUNCTION(BlueprintCallable, Category = "Bee Swarm", meta = (DeprecatedFunction, DeprecationMessage = "Use ApplySplineLengthParameter or ApplyExternalSwarmParameters"))
	void ApplySwarmParameters();

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm", meta = (DeprecatedFunction, DeprecationMessage = "Use ApplySplineLengthParameter or ApplyBeeSwarmHour24 on the owner"))
	void ApplyHour24(float Hour24);

	// Legacy C++ APIs kept temporarily for compatibility; no-op in control-mode workflow.
	void ApplyOverrideSettings(const FBeeSplineSwarmOverrideSettings& OverrideSettings, float Hour24);
	void ClearOverrideSettings(float Hour24);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> SwarmSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> SwarmNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm")
	EBeeSplineSwarmControlMode ControlMode = EBeeSplineSwarmControlMode::Standalone;

private:
	void ApplyExternalSwarmParametersInternal(const FBeeSplineSwarmAppliedParameters& Parameters);

#if WITH_EDITOR
	bool ShouldApplyParametersInEditor() const;
#endif

	bool bWarnedMissingExternalParameters = false;

	bool bHasLastAppliedExternalParameters = false;

	FBeeSplineSwarmAppliedParameters LastAppliedExternalParameters;
};
