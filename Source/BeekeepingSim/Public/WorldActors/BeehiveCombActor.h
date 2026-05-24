#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BeehiveCombActor.generated.h"

class UNiagaraComponent;
class UMaterialInstanceDynamic;
class USceneComponent;
class UStaticMeshComponent;
class UCursorPartFocusActionComponent;

UENUM(BlueprintType)
enum class EBeehiveCombVisibleFace : uint8
{
	Front,
	Back
};

UENUM(BlueprintType)
enum class EBeehiveCombFlipDirection : uint8
{
	Left,
	Right
};

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

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Bee")
	USceneComponent* GetQueenFrontAttachPoint() const { return QueenFrontAttachPoint; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Bee")
	USceneComponent* GetQueenBackAttachPoint() const { return QueenBackAttachPoint; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Bee")
	USceneComponent* GetQueenAttachPoint(bool bFrontFace) const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Honey")
	void AddHoneyAmount(float DeltaHoney);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Honey")
	void SetCurrentHoney(float NewHoneyAmount);

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey")
	float GetCurrentHoney() const { return CurrentHoney; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey")
	float GetHoneyFillRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void FlipCombFace();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void FlipCombFaceWithDirection(EBeehiveCombFlipDirection FlipDirection);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetVisibleCombFace(EBeehiveCombVisibleFace NewFace);

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	EBeehiveCombVisibleFace GetVisibleCombFace() const { return VisibleCombFace; }

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ApplyCombShakeByRatio(float ReductionRatio);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ApplyCombShakeByRatioWithStrokeCount(float ReductionRatio, int32 StrokeCount);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> CombPivotRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CombMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FrontFaceBeeNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> BackFaceBeeNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCursorPartFocusActionComponent> PartFocusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> QueenFrontAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> QueenBackAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrontHoneyPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BackHoneyPlane;

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb", meta = (DisplayName = "Receive Comb Flipped"))
	void ReceiveCombFlipped(EBeehiveCombVisibleFace NewVisibleFace);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb", meta = (DisplayName = "Receive Comb Flipped With Direction"))
	void ReceiveCombFlippedWithDirection(EBeehiveCombVisibleFace NewVisibleFace, EBeehiveCombFlipDirection FlipDirection);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb", meta = (DisplayName = "Receive Comb Shaken"))
	void ReceiveCombShaken(int32 StrokeCount, float ReductionRatio);

private:
	void ApplyNiagaraUserParameters();
	void RestartBeeNiagaraSystems();
	void SanitizeState();
	void SanitizeHoneyState();
	void ApplyHoneyVisualState();
	void EnsureHoneyMaterialInstances();

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb")
	FVector2D PlaneSize = FVector2D(100.0f, 100.0f);

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 SpawnAmount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 TargetBeeCount = 0;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey", meta = (ClampMin = "0.0"))
	float MaxHoneyPerComb = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Honey", meta = (ClampMin = "0.0"))
	float CurrentHoney = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
	FVector FrontHoneyEmptyRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
	FVector FrontHoneyFullRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
	FVector BackHoneyEmptyRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
	FVector BackHoneyFullRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
	FName HoneyMaterialParameterName = TEXT("HoneyAmount");

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FrontHoneyMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackHoneyMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb")
	EBeehiveCombVisibleFace VisibleCombFace = EBeehiveCombVisibleFace::Front;
};
