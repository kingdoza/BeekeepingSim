#pragma once

#include "CoreMinimal.h"
#include "Focus/ItemUseAreaActivationProvider.h"
#include "GameFramework/Actor.h"
#include "BeehiveCombActor.generated.h"

class UNiagaraComponent;
class UMaterialInstanceDynamic;
class USceneComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UItemUseAreaMeshComponent;
class UCursorPartFocusActionComponent;
class UPlacementOccupantComponent;
class UPlacementSlotRetrievePartFocusActionComponent;
class UItemInstance;
class UTexture2D;
class UMaterialInterface;
class UQueenCellSpawnAreaComponent;
class ABeehive;
struct FBeehiveCombItemState;

UENUM(BlueprintType)
enum class EBeehiveCombVisibleFace : uint8
{
	Front,
	Back
};

USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FQueenCellPlacement
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell")
	FGuid QueenCellId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell")
	EBeehiveCombVisibleFace Face = EBeehiveCombVisibleFace::Front;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell")
	FVector2D AreaLocalYZ = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell")
	float LocalRotationDegrees = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell")
	float Scale = 1.0f;
};

USTRUCT()
struct BEEKEEPINGSIM_API FQueenCellRuntimeComponents
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY(Transient)
	TObjectPtr<UItemUseAreaMeshComponent> UseArea;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UFUNCTION(BlueprintPure, Category = "Beehive|Wax Capping")
	UItemUseAreaMeshComponent* GetFrontWaxCappingUseAreaMesh() const { return FrontWaxCappingUseAreaMesh; }

	UFUNCTION(BlueprintPure, Category = "Beehive|Wax Capping")
	UItemUseAreaMeshComponent* GetBackWaxCappingUseAreaMesh() const { return BackWaxCappingUseAreaMesh; }

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

	UFUNCTION(BlueprintCallable, Category = "Beehive|Wax Capping")
	bool ApplyWaxCappingBrush(UPrimitiveComponent* HitComponent, const FVector& WorldImpactPoint, float BrushRadiusCm);

	UFUNCTION(BlueprintCallable, Category = "Beehive|Wax Capping")
	bool TryRegenerateWaxCapping();

	UFUNCTION(BlueprintPure, Category = "Beehive|Wax Capping")
	float GetWaxCappingRemainingRatio(EBeehiveCombVisibleFace Face) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Wax Capping")
	bool IsWaxCappingFaceComplete(EBeehiveCombVisibleFace Face) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Wax Capping")
	bool IsWaxCappingComplete() const;

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

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Cell")
	int32 GetQueenCellCount() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Cell")
	bool HasQueenCells() const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Cell")
	bool CanSpawnQueenCell() const;

	UFUNCTION(BlueprintCallable, Category = "Beehive|Queen Cell")
	bool TrySpawnQueenCell();

	UFUNCTION(BlueprintCallable, Category = "Beehive|Queen Cell")
	bool RemoveQueenCell(const FGuid& QueenCellId);

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Cell")
	bool ResolveQueenCellIdFromUseArea(const UItemUseAreaMeshComponent* UseArea, FGuid& OutQueenCellId) const;

	UFUNCTION(BlueprintPure, Category = "Beehive|Queen Cell")
	UQueenCellSpawnAreaComponent* GetQueenCellSpawnArea() const { return QueenCellSpawnArea; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshComponent> FrontWaxCappingUseAreaMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UItemUseAreaMeshComponent> BackWaxCappingUseAreaMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UQueenCellSpawnAreaComponent> QueenCellSpawnArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Visual")
	TObjectPtr<UStaticMesh> QueenCellVisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Visual")
	TObjectPtr<UMaterialInterface> QueenCellVisualMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
	TObjectPtr<UStaticMesh> QueenCellUseAreaMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
	TObjectPtr<UMaterialInterface> QueenCellUseAreaMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Spawn")
	FTransform QueenCellSpawnRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
	FVector QueenCellUseAreaScaleMultiplier = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0"))
	int32 MaxQueenCellCountPerComb = 4;

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
	void EnsureCappingMaskState();
	void InitializeFullCappingMasks(int32 NewWidth, int32 NewHeight);
	void ResolveCappingMaskDimensions(int32& OutWidth, int32& OutHeight) const;
	FVector2D ResolveCappingPlaneSizeForMask() const;
	bool IsStoredCappingMaskValid(const FBeehiveCombItemState& CombState) const;
	TArray<uint8>& GetMutableWaxCappingMask(EBeehiveCombVisibleFace Face);
	const TArray<uint8>& GetWaxCappingMask(EBeehiveCombVisibleFace Face) const;
	TObjectPtr<UTexture2D>& GetWaxCappingMaskTextureRef(EBeehiveCombVisibleFace Face);
	void EnsureCappingMaskTextures();
	void RefreshCappingMaskTextures();
	void UpdateCappingMaskTexture(EBeehiveCombVisibleFace Face);
	void ApplyWaxCappingMaskMaterialParameters();
	void SyncVisibleCombFacePresentation();
	void ApplyHoneyVisualState();
	void ApplyHoneyCappingVisualState();
	void EnsureHoneyMaterialInstances();
	bool CreateQueenCellRuntimeComponents(const FQueenCellPlacement& Placement);
	void DestroyQueenCellRuntimeComponents(const FGuid& QueenCellId);
	void DestroyAllQueenCellRuntimeComponents();
	FTransform BuildQueenCellSpawnAreaRelativeTransform(const FQueenCellPlacement& Placement) const;
	ABeehive* ResolveOwningHive() const;
	void RequestOwningHiveItemUseAreaRebuild() const;

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

	UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping", meta = (ClampMin = "1"))
	int32 CappingMaskLongSideResolution = 512;

	UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UncappedThreshold = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WaxCappingRegenerationRipenessThreshold = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping")
	FName WaxCappingMaskMaterialParameterName = TEXT("WaxCappingMask");

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
	int32 RuntimeCappingMaskWidth = 0;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
	int32 RuntimeCappingMaskHeight = 0;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
	TArray<uint8> RuntimeFrontWaxCappingMask;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
	TArray<uint8> RuntimeBackWaxCappingMask;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> FrontWaxCappingMaskTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> BackWaxCappingMaskTexture;

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

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Queen Cell")
	TArray<FQueenCellPlacement> QueenCellPlacements;

	UPROPERTY(Transient)
	TMap<FGuid, FQueenCellRuntimeComponents> QueenCellRuntimeComponents;

	TMap<TObjectPtr<UItemUseAreaMeshComponent>, FGuid> QueenCellUseAreaToId;
};
