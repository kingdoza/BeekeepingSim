#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoneyContainerActor.generated.h"

class UHoneyContainerItemDefinition;
class UHoneyContainerRetrievePartFocusActionComponent;
class UHoneyNozzlePartFocusActionComponent;
class UItemDefinition;
class UItemInstance;
class UMaterialInstanceDynamic;
class UPlacementOccupantComponent;
class UNiagaraComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AHoneyContainerActor : public AActor
{
	GENERATED_BODY()

public:
	AHoneyContainerActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Honey Container|Placement")
	void ApplyStateFromItemInstance(const UItemInstance* SourceItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Honey Container|Placement")
	void WriteHoneyContainerStateToItemInstance(UItemInstance* TargetItemInstance) const;

	UFUNCTION(BlueprintPure, Category = "Honey Container")
	float GetCurrentVolumeMl() const { return CurrentVolumeMl; }

	UFUNCTION(BlueprintPure, Category = "Honey Container")
	float GetMaxVolumeMl() const { return MaxVolumeMl; }

	UFUNCTION(BlueprintPure, Category = "Honey Container")
	float GetFreeVolumeMl() const;

	UFUNCTION(BlueprintPure, Category = "Honey Container")
	float GetHoneyDensity() const { return HoneyDensity; }

	UFUNCTION(BlueprintPure, Category = "Honey Container")
	float GetHoneyRipeness() const { return HoneyRipeness; }

	UFUNCTION(BlueprintPure, Category = "Honey Container")
	float GetFillRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Honey Container")
	void RemoveHoneyVolume(float VolumeMl);

	UFUNCTION(BlueprintCallable, Category = "Honey Container")
	void AddHoneyVolume(float VolumeMl, float IncomingDensity, float IncomingRipeness);

	UFUNCTION(BlueprintPure, Category = "Honey Container|Nozzle")
	USceneComponent* GetNozzleOriginComponent() const { return NozzleOrigin; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Pour")
	USceneComponent* GetPourTargetComponent() const { return PourTarget; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Stream")
	UNiagaraComponent* GetHoneyStreamNiagaraComponent() const { return HoneyStreamNiagara; }

	UFUNCTION(BlueprintCallable, Category = "Honey Container|Visual")
	void RefreshHoneyVisualState();

	UFUNCTION(BlueprintPure, Category = "Honey Container|Components")
	UStaticMeshComponent* GetContainerMeshComponent() const { return ContainerMesh; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Components")
	UStaticMeshComponent* GetHoneyVisualMeshComponent() const { return HoneyVisualMesh; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Components")
	UStaticMeshComponent* GetNozzleHitComponent() const { return NozzleHitComponent; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Placement")
	UPlacementOccupantComponent* GetPlacementOccupantComponent() const { return PlacementOccupant; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Placement")
	UHoneyContainerRetrievePartFocusActionComponent* GetRetrieveActionComponent() const { return RetrieveAction; }

	UFUNCTION(BlueprintPure, Category = "Honey Container|Nozzle")
	UHoneyNozzlePartFocusActionComponent* GetNozzleActionComponent() const { return NozzleAction; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ContainerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HoneyVisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> NozzleHitComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> NozzleOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PourTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> HoneyStreamNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlacementOccupantComponent> PlacementOccupant;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHoneyContainerRetrievePartFocusActionComponent> RetrieveAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHoneyNozzlePartFocusActionComponent> NozzleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Container|Visual")
	FName HoneyDensityMaterialParameterName = TEXT("HoneyDensity");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Container|Visual")
	FName HoneyRipenessMaterialParameterName = TEXT("HoneyRipeness");

private:
	void ApplyDefinitionDefaults(const UHoneyContainerItemDefinition* HoneyContainerDefinition);
	void SanitizeHoneyState();
	void CaptureHoneyVisualFullScaleIfNeeded();
	void EnsureHoneyVisualMaterialInstance();
	UItemDefinition* ResolveReturnItemDefinition() const;
	const UHoneyContainerItemDefinition* ResolveHoneyContainerDefinition() const;
	float NormalizeHoneyRipeness(float Density, float Ripeness) const;

	UPROPERTY(VisibleAnywhere, Category = "Honey Container", meta = (ClampMin = "0.0"))
	float MaxVolumeMl = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Honey Container", meta = (ClampMin = "0.0"))
	float CurrentVolumeMl = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoneyDensity = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoneyRipeness = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HoneyVisualMaterialInstance;

	FVector HoneyVisualFullRelativeScale = FVector::OneVector;
	bool bHasCapturedHoneyVisualFullScale = false;
};
