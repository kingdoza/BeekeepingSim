#pragma once

#include "CoreMinimal.h"
#include "Focus/ItemUseAreaActivationProvider.h"
#include "GameFramework/Actor.h"
#include "BeehiveCombActor.generated.h"

class UNiagaraComponent;
class UMaterialInstanceDynamic;
class USceneComponent;
class UStaticMeshComponent;
class UItemUseAreaMeshComponent;
class UCursorPartFocusActionComponent;
class UPlacementOccupantComponent;
class UPlacementSlotRetrievePartFocusActionComponent;
class UItemInstance;

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
class BEEKEEPINGSIM_API ABeehiveCombActor : public AActor, public IItemUseAreaActivationProvider
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
	void ApplyCombBeeParameters(const FVector2D& InPlaneSize, int32 InTotalSpawnAmount, int32 InTotalTargetBeeCount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetTotalSpawnAmountAndResetTargetBeeCounts(const FVector2D& InPlaneSize, int32 InTotalSpawnAmount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetTotalSpawnAmountPreservingTargetRatios(const FVector2D& InPlaneSize, int32 InNewTotalSpawnAmount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void SetTotalTargetBeeCount(int32 NewTotalTargetBeeCount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ResetTargetBeeCountsToSpawnAmount();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceAllTargetBeeCountsByRatio(float Ratio);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceAllTargetBeeCountsByAmount(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceVisibleFaceTargetBeeCountByAmount(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
	void ReduceFaceTargetBeeCountByAmount(EBeehiveCombVisibleFace Face, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetTotalSpawnAmount() const { return TotalSpawnAmount; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetTotalTargetBeeCount() const { return FrontFaceTargetBeeCount + BackFaceTargetBeeCount; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetFaceSpawnAmount(EBeehiveCombVisibleFace Face) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetFaceTargetBeeCount(EBeehiveCombVisibleFace Face) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	int32 GetVisibleFaceTargetBeeCount() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	UStaticMeshComponent* GetCombMeshComponent() const { return CombMesh; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Bee Brush")
	UItemUseAreaMeshComponent* GetBeeBrushUseAreaMesh() const { return BeeBrushUseAreaMesh; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
	UCursorPartFocusActionComponent* GetPartFocusActionComponent() const { return PartFocusAction; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Placement")
	UPlacementOccupantComponent* GetPlacementOccupantComponent() const { return PlacementOccupant; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Placement")
	UPlacementSlotRetrievePartFocusActionComponent* GetPlacementRetrieveActionComponent() const { return PlacementRetrieveAction; }

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

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey")
	bool IsHoneyFull() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Ripeness")
	void AddHoneyRipeness(float DeltaRipeness);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Ripeness")
	void SetCurrentHoneyRipeness(float NewHoneyRipeness);

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey Ripeness")
	float GetCurrentHoneyRipeness() const { return CurrentHoneyRipeness; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Honey Ripeness")
	float GetHoneyRipenessRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Disease")
	void SetBeeDiseaseValue(float NewDiseaseValue);

	UFUNCTION(BlueprintPure, Category = "Beehive|Comb Disease")
	float GetBeeDiseaseValue() const { return BeeDiseaseValue; }

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

	UFUNCTION(BlueprintCallable, Category = "Beehive|Placement")
	void ApplyStateFromItemInstance(const UItemInstance* SourceItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Placement")
	void WriteStateToItemInstance(UItemInstance* TargetItemInstance) const;

	virtual bool IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const override;

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
	TObjectPtr<UPlacementOccupantComponent> PlacementOccupant;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlacementSlotRetrievePartFocusActionComponent> PlacementRetrieveAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> QueenFrontAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> QueenBackAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrontHoneyPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BackHoneyPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrontWaxCappingPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BackWaxCappingPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshComponent> BeeBrushUseAreaMesh;

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb", meta = (DisplayName = "Receive Comb Flipped"))
	void ReceiveCombFlipped(EBeehiveCombVisibleFace NewVisibleFace);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb", meta = (DisplayName = "Receive Comb Flipped With Direction"))
	void ReceiveCombFlippedWithDirection(EBeehiveCombVisibleFace NewVisibleFace, EBeehiveCombFlipDirection FlipDirection);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb", meta = (DisplayName = "Receive Comb Shaken"))
	void ReceiveCombShaken(int32 StrokeCount, float ReductionRatio);

private:
	static int32 GetFrontShareFromTotal(int32 Total);
	static int32 GetBackShareFromTotal(int32 Total);
	int32& GetMutableFaceTargetBeeCount(EBeehiveCombVisibleFace Face);
	int32 GetFaceTargetBeeCountInternal(EBeehiveCombVisibleFace Face) const;
	void ApplyNiagaraUserParameters();
	void RestartBeeNiagaraSystems();
	void SanitizeState();
	void SanitizeHoneyState();
	void SanitizeHoneyRipenessState();
	void ApplyHoneyVisualState();
	void ApplyHoneyCappingVisualState();
	void EnsureHoneyMaterialInstances();

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb")
	FVector2D PlaneSize = FVector2D(100.0f, 100.0f);

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 TotalSpawnAmount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 FrontFaceTargetBeeCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb", meta = (ClampMin = "0"))
	int32 BackFaceTargetBeeCount = 0;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey", meta = (ClampMin = "0.0"))
	float MaxHoneyPerComb = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Honey", meta = (ClampMin = "0.0"))
	float CurrentHoney = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey Ripeness", meta = (ClampMin = "0.0"))
	float MaxHoneyRipeness = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Honey Ripeness", meta = (ClampMin = "0.0"))
	float CurrentHoneyRipeness = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb Disease", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BeeDiseaseValue = 0.0f;

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

	UPROPERTY(EditAnywhere, Category = "Beehive|Honey Ripeness")
	FName HoneyRipenessMaterialParameterName = TEXT("HoneyRipeness");

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FrontHoneyMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackHoneyMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FrontWaxCappingMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackWaxCappingMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb")
	EBeehiveCombVisibleFace VisibleCombFace = EBeehiveCombVisibleFace::Front;
};
