#pragma once

#include "CoreMinimal.h"
#include "Focus/ItemUseAreaActivationProvider.h"
#include "GameFramework/Actor.h"
#include "BeeSwarmClusterActor.generated.h"

class AQueenBeeActor;
class UAnchoredFocusCursorActionComponent;
class UChildActorComponent;
class UCursorItemUseAreaScopeComponent;
class UFocusTargetComponent;
class UItemUseAreaMeshComponent;
class UItemUseAreaMeshProviderComponent;
class UNiagaraComponent;
class USceneComponent;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ABeeSwarmClusterActor : public AActor, public IItemUseAreaActivationProvider
{
	GENERATED_BODY()

public:
	ABeeSwarmClusterActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster")
	void InitializeSwarmCluster(float InAliveRadius, int32 InSpawnAmount, float InSphereRadius);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Niagara")
	void ApplyClusterNiagaraParameters();

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster")
	float DecreaseAliveRadius(float DeltaRadius);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster")
	void SetAliveRadius(float NewAliveRadius);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Capture")
	float CaptureBees(float RequestedBeeAmount);

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Capture")
	void SetCapturedBeeAmount(float NewCapturedBeeAmount);

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster")
	float GetAliveRadius() const { return AliveRadius; }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster")
	float GetInitialAliveRadius() const { return InitialAliveRadius; }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster")
	float GetSphereRadius() const { return SphereRadius; }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster")
	int32 GetSpawnAmount() const { return SpawnAmount; }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Capture")
	float GetCapturedBeeAmount() const { return FMath::Max(0.0f, CapturedBeeAmount); }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Capture")
	int32 GetCapturedBeeCountRounded() const;

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Capture")
	float GetRemainingBeeAmount() const;

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Capture")
	int32 GetRemainingBeeCountRounded() const;

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Capture")
	float GetTotalBeeAmount() const;

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Capture")
	float CalculateAliveRadiusFromRemainingBees() const;

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Capture")
	void RefreshAliveRadiusFromBeeAmounts();

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster")
	bool IsCaptured() const { return bCaptured; }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Components")
	USceneComponent* GetClusterCenterComponent() const { return ClusterCenter; }

	UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Queen Bee")
	AQueenBeeActor* GetQueenBeeActor() const;

	UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Item Use Area")
	void RebuildItemUseAreaDescriptors();

	virtual bool IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> ClusterCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ClusterNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> QueenBeeChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshComponent> CaptureUseAreaMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FocusAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> CharacterAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFocusTargetComponent> FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAnchoredFocusCursorActionComponent> FocusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCursorItemUseAreaScopeComponent> ItemUseAreaScope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshProviderComponent> ItemUseAreaMeshProvider;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster", meta = (ClampMin = "0.0"))
	float AliveRadius = 200.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Capture")
	float InitialAliveRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster", meta = (ClampMin = "0"))
	int32 SpawnAmount = 300;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Capture")
	float CapturedBeeAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster", meta = (ClampMin = "0.0"))
	float SphereRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster|Queen Bee")
	TSubclassOf<AQueenBeeActor> SwarmQueenBeeActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster|Queen Bee")
	FVector QueenCenterOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster|Niagara")
	FName AliveRadiusParameterName = TEXT("User.AliveRadius");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster|Niagara")
	FName SpawnAmountParameterName = TEXT("User.SpawnAmount");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster|Niagara")
	FName SphereRadiusParameterName = TEXT("User.SphereRadius");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster")
	bool bCaptured = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Bee Swarm Cluster")
	void ReceiveSwarmClusterInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bee Swarm Cluster")
	void ReceiveAliveRadiusChanged(float NewAliveRadius);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bee Swarm Cluster")
	void ReceiveSwarmCaptured();

private:
	void EnsureQueenBeeChildActorClass();
	void ApplyQueenBeeTransform();
	void ApplyCaptureUseAreaVisualIdleState();
	void SetCaptureUseAreaActive(bool bActive);
	void HandleCapturedIfNeeded();
};
