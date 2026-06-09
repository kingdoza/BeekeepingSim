#include "WorldActors/BeehiveCombActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/ItemInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeehiveCombPartFocusActionComponent.h"
#include "WorldActors/BeehiveCombPlacementOccupantComponent.h"
#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"
#include "WorldActors/UncappingTable.h"

namespace BeehiveCombActorNames
{
	static const FName PlaneSize(TEXT("User.PlaneSize"));
	static const FName SpawnAmount(TEXT("User.SpawnAmount"));
	static const FName TargetBeeCount(TEXT("User.TargetBeeCount"));
	static const FName UncappingTableCombUseAreaTag(TEXT("Item.UseArea.UncappingTable.Comb"));
	// Legacy direct disease path disabled; ABeehive::DiseaseVfxNiagara now represents disease.
	// static const FName Disease(TEXT("User.Disease"));

	static bool IsGameWorldContext(const UObject* WorldContext)
	{
		const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
		return World && World->IsGameWorld();
	}

	static UMaterialInterface* RestoreDynamicMaterialParentInEditor(UPrimitiveComponent* Component, int32 MaterialIndex)
	{
		if (!Component)
		{
			return nullptr;
		}

		UMaterialInterface* CurrentMaterial = Component->GetMaterial(MaterialIndex);
#if WITH_EDITOR
		if (!IsGameWorldContext(Component))
		{
			UMaterialInterface* AuthoredMaterial = CurrentMaterial;
			for (int32 Depth = 0; Depth < 8; ++Depth)
			{
				UMaterialInstanceDynamic* CurrentMID = Cast<UMaterialInstanceDynamic>(AuthoredMaterial);
				if (!CurrentMID || !CurrentMID->Parent || CurrentMID->Parent == AuthoredMaterial)
				{
					break;
				}

				AuthoredMaterial = CurrentMID->Parent;
			}

			if (AuthoredMaterial && AuthoredMaterial != CurrentMaterial)
			{
				Component->SetMaterial(MaterialIndex, AuthoredMaterial);
				CurrentMaterial = AuthoredMaterial;
			}
		}
#endif
		return CurrentMaterial;
	}
}

ABeehiveCombActor::ABeehiveCombActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CombPivotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CombPivotRoot"));
	CombPivotRoot->SetupAttachment(Root);

	CombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CombMesh"));
	CombMesh->SetupAttachment(CombPivotRoot);

	FrontFaceBeeNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FrontFaceBeeNiagara"));
	FrontFaceBeeNiagara->SetupAttachment(CombPivotRoot);

	BackFaceBeeNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BackFaceBeeNiagara"));
	BackFaceBeeNiagara->SetupAttachment(CombPivotRoot);

	PartFocusAction = CreateDefaultSubobject<UBeehiveCombPartFocusActionComponent>(TEXT("PartFocusAction"));
	if (PartFocusAction)
	{
		PartFocusAction->SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
		FGameplayTagContainer RequiredTags;
		RequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.LidOpen")), false));
		PartFocusAction->SetRequiredStateTags(RequiredTags);
		PartFocusAction->SetExclusiveGroup(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.CombLift")), false));
	}

	PlacementOccupant = CreateDefaultSubobject<UBeehiveCombPlacementOccupantComponent>(TEXT("PlacementOccupant"));
	PlacementRetrieveAction = CreateDefaultSubobject<UPlacementSlotRetrievePartFocusActionComponent>(TEXT("PlacementRetrieveAction"));
	if (PlacementRetrieveAction)
	{
		PlacementRetrieveAction->SetEngageMode(ECursorPartFocusEngageMode::PreviewOnly);
	}

	QueenFrontAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("QueenFrontAttachPoint"));
	QueenFrontAttachPoint->SetupAttachment(CombMesh);
	QueenFrontAttachPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	QueenBackAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("QueenBackAttachPoint"));
	QueenBackAttachPoint->SetupAttachment(CombMesh);
	QueenBackAttachPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	QueenBackAttachPoint->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	FrontHoneyPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontHoneyPlane"));
	FrontHoneyPlane->SetupAttachment(CombMesh);

	BackHoneyPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackHoneyPlane"));
	BackHoneyPlane->SetupAttachment(CombMesh);

	FrontWaxCappingPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWaxCappingPlane"));
	FrontWaxCappingPlane->SetupAttachment(CombMesh);
	FrontWaxCappingPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FrontWaxCappingPlane->SetCollisionResponseToAllChannels(ECR_Ignore);
	FrontWaxCappingPlane->SetHiddenInGame(true);

	BackWaxCappingPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackWaxCappingPlane"));
	BackWaxCappingPlane->SetupAttachment(CombMesh);
	BackWaxCappingPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackWaxCappingPlane->SetCollisionResponseToAllChannels(ECR_Ignore);
	BackWaxCappingPlane->SetHiddenInGame(true);

	BeeBrushUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("BeeBrushUseAreaMesh"));
	BeeBrushUseAreaMesh->SetupAttachment(CombMesh);
	BeeBrushUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
	BeeBrushUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeeBrushUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	FGameplayTagContainer WaxCappingUseAreaTags;
	const FGameplayTag UncappingTableCombTag = FGameplayTag::RequestGameplayTag(BeehiveCombActorNames::UncappingTableCombUseAreaTag, false);
	if (UncappingTableCombTag.IsValid())
	{
		WaxCappingUseAreaTags.AddTag(UncappingTableCombTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing gameplay tag '%s' for beehive comb wax capping use-area."),
			*BeehiveCombActorNames::UncappingTableCombUseAreaTag.ToString());
	}

	FrontWaxCappingUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("FrontWaxCappingUseAreaMesh"));
	FrontWaxCappingUseAreaMesh->SetupAttachment(CombMesh);
	FrontWaxCappingUseAreaMesh->SetAreaId(TEXT("UncappingTable.Comb.FrontWaxCapping"));
	FrontWaxCappingUseAreaMesh->SetAreaTags(WaxCappingUseAreaTags);
	FrontWaxCappingUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
	FrontWaxCappingUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FrontWaxCappingUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	BackWaxCappingUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("BackWaxCappingUseAreaMesh"));
	BackWaxCappingUseAreaMesh->SetupAttachment(CombMesh);
	BackWaxCappingUseAreaMesh->SetAreaId(TEXT("UncappingTable.Comb.BackWaxCapping"));
	BackWaxCappingUseAreaMesh->SetAreaTags(WaxCappingUseAreaTags);
	BackWaxCappingUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
	BackWaxCappingUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackWaxCappingUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ABeehiveCombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SanitizeState();
	SanitizeHoneyState();
	SanitizeHoneyRipenessState();
	EnsureCappingMaskState();
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();
}

void ABeehiveCombActor::BeginPlay()
{
	Super::BeginPlay();
	const UWorld* World = GetWorld();
	UE_LOG(LogTemp, Log, TEXT("[HoneyDiagnostic] Comb=%s Phase=CombBeginPlay_BeforeSanitize World=%s IsGameWorld=%d Owner=%s AttachParentActor=%s Honey=%.3f FillRatio=%.3f Ripeness=%.3f RipenessRatio=%.3f"),
		*GetNameSafe(this),
		*GetNameSafe(World),
		World && World->IsGameWorld() ? 1 : 0,
		*GetNameSafe(GetOwner()),
		*GetNameSafe(GetAttachParentActor()),
		CurrentHoney,
		GetHoneyFillRatio(),
		CurrentHoneyRipeness,
		GetHoneyRipenessRatio());

	SanitizeState();
	SanitizeHoneyState();
	SanitizeHoneyRipenessState();
	EnsureCappingMaskState();
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();

	UE_LOG(LogTemp, Log, TEXT("[HoneyDiagnostic] Comb=%s Phase=CombBeginPlay_AfterSanitize Honey=%.3f FillRatio=%.3f Ripeness=%.3f RipenessRatio=%.3f"),
		*GetNameSafe(this),
		CurrentHoney,
		GetHoneyFillRatio(),
		CurrentHoneyRipeness,
		GetHoneyRipenessRatio());
}

int32 ABeehiveCombActor::GetFrontShareFromTotal(int32 Total)
{
	return (FMath::Max(0, Total) + 1) / 2;
}

int32 ABeehiveCombActor::GetBackShareFromTotal(int32 Total)
{
	return FMath::Max(0, Total) / 2;
}

int32 ABeehiveCombActor::GetFaceSpawnAmount(EBeehiveCombVisibleFace Face) const
{
	return Face == EBeehiveCombVisibleFace::Front
		? GetFrontShareFromTotal(TotalSpawnAmount)
		: GetBackShareFromTotal(TotalSpawnAmount);
}

int32& ABeehiveCombActor::GetMutableFaceTargetBeeCount(EBeehiveCombVisibleFace Face)
{
	return Face == EBeehiveCombVisibleFace::Front ? FrontFaceTargetBeeCount : BackFaceTargetBeeCount;
}

int32 ABeehiveCombActor::GetFaceTargetBeeCountInternal(EBeehiveCombVisibleFace Face) const
{
	return Face == EBeehiveCombVisibleFace::Front ? FrontFaceTargetBeeCount : BackFaceTargetBeeCount;
}

int32 ABeehiveCombActor::GetFaceTargetBeeCount(EBeehiveCombVisibleFace Face) const
{
	return GetFaceTargetBeeCountInternal(Face);
}

int32 ABeehiveCombActor::GetVisibleFaceTargetBeeCount() const
{
	return GetFaceTargetBeeCountInternal(VisibleCombFace);
}

void ABeehiveCombActor::ApplyCombBeeParameters(const FVector2D& InPlaneSize, int32 InTotalSpawnAmount, int32 InTotalTargetBeeCount)
{
	const int32 PreviousTotalSpawnAmount = TotalSpawnAmount;
	PlaneSize = InPlaneSize;
	TotalSpawnAmount = InTotalSpawnAmount;

	const int32 SanitizedTotalTarget = FMath::Max(0, InTotalTargetBeeCount);
	FrontFaceTargetBeeCount = GetFrontShareFromTotal(SanitizedTotalTarget);
	BackFaceTargetBeeCount = GetBackShareFromTotal(SanitizedTotalTarget);

	SanitizeState();
	EnsureCappingMaskState();
	ApplyNiagaraUserParameters();
	if (TotalSpawnAmount != PreviousTotalSpawnAmount)
	{
		RestartBeeNiagaraSystems();
	}
}

void ABeehiveCombActor::SetTotalSpawnAmountAndResetTargetBeeCounts(const FVector2D& InPlaneSize, int32 InTotalSpawnAmount)
{
	const int32 PreviousTotalSpawnAmount = TotalSpawnAmount;
	PlaneSize = InPlaneSize;
	TotalSpawnAmount = InTotalSpawnAmount;
	SanitizeState();
	FrontFaceTargetBeeCount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Front);
	BackFaceTargetBeeCount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Back);
	SanitizeState();
	EnsureCappingMaskState();
	ApplyNiagaraUserParameters();
	if (TotalSpawnAmount != PreviousTotalSpawnAmount)
	{
		RestartBeeNiagaraSystems();
	}
}

void ABeehiveCombActor::SetTotalSpawnAmountPreservingTargetRatios(const FVector2D& InPlaneSize, int32 InNewTotalSpawnAmount)
{
	SanitizeState();

	const int32 PreviousTotalSpawnAmount = TotalSpawnAmount;
	const int32 OldFrontSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Front);
	const int32 OldBackSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Back);
	const int32 OldFrontTargetBeeCount = FrontFaceTargetBeeCount;
	const int32 OldBackTargetBeeCount = BackFaceTargetBeeCount;

	PlaneSize = InPlaneSize;
	TotalSpawnAmount = InNewTotalSpawnAmount;

	const int32 NewFrontSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Front);
	const int32 NewBackSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Back);

	if (OldFrontSpawnAmount <= 0)
	{
		FrontFaceTargetBeeCount = NewFrontSpawnAmount;
	}
	else
	{
		const float FrontRatio = static_cast<float>(OldFrontTargetBeeCount) / static_cast<float>(OldFrontSpawnAmount);
		FrontFaceTargetBeeCount = FMath::RoundToInt(static_cast<float>(NewFrontSpawnAmount) * FrontRatio);
	}

	if (OldBackSpawnAmount <= 0)
	{
		BackFaceTargetBeeCount = NewBackSpawnAmount;
	}
	else
	{
		const float BackRatio = static_cast<float>(OldBackTargetBeeCount) / static_cast<float>(OldBackSpawnAmount);
		BackFaceTargetBeeCount = FMath::RoundToInt(static_cast<float>(NewBackSpawnAmount) * BackRatio);
	}

	SanitizeState();
	EnsureCappingMaskState();
	ApplyNiagaraUserParameters();
	if (TotalSpawnAmount != PreviousTotalSpawnAmount)
	{
		RestartBeeNiagaraSystems();
	}
}

void ABeehiveCombActor::SetTotalTargetBeeCount(int32 NewTotalTargetBeeCount)
{
	const int32 ClampedTotalTargetBeeCount = FMath::Clamp(NewTotalTargetBeeCount, 0, FMath::Max(0, TotalSpawnAmount));
	FrontFaceTargetBeeCount = GetFrontShareFromTotal(ClampedTotalTargetBeeCount);
	BackFaceTargetBeeCount = GetBackShareFromTotal(ClampedTotalTargetBeeCount);
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ResetTargetBeeCountsToSpawnAmount()
{
	FrontFaceTargetBeeCount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Front);
	BackFaceTargetBeeCount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Back);
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ReduceAllTargetBeeCountsByRatio(float Ratio)
{
	const float ClampedRatio = FMath::Clamp(Ratio, 0.0f, 1.0f);
	const int32 FrontReduction = FMath::RoundToInt(static_cast<float>(FrontFaceTargetBeeCount) * ClampedRatio);
	const int32 BackReduction = FMath::RoundToInt(static_cast<float>(BackFaceTargetBeeCount) * ClampedRatio);

	FrontFaceTargetBeeCount -= FMath::Max(0, FrontReduction);
	BackFaceTargetBeeCount -= FMath::Max(0, BackReduction);

	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ReduceAllTargetBeeCountsByAmount(int32 Amount)
{
	const int32 ClampedAmount = FMath::Clamp(Amount, 0, GetTotalTargetBeeCount());
	if (ClampedAmount <= 0)
	{
		return;
	}

	const int32 RequestedFrontReduction = GetFrontShareFromTotal(ClampedAmount);
	const int32 RequestedBackReduction = GetBackShareFromTotal(ClampedAmount);

	const int32 FrontReduction = FMath::Min(FrontFaceTargetBeeCount, RequestedFrontReduction);
	const int32 BackReduction = FMath::Min(BackFaceTargetBeeCount, RequestedBackReduction);

	FrontFaceTargetBeeCount -= FrontReduction;
	BackFaceTargetBeeCount -= BackReduction;

	int32 RemainingReduction = ClampedAmount - (FrontReduction + BackReduction);
	if (RemainingReduction > 0)
	{
		const int32 AdditionalFrontReduction = FMath::Min(FrontFaceTargetBeeCount, RemainingReduction);
		FrontFaceTargetBeeCount -= AdditionalFrontReduction;
		RemainingReduction -= AdditionalFrontReduction;
	}

	if (RemainingReduction > 0)
	{
		const int32 AdditionalBackReduction = FMath::Min(BackFaceTargetBeeCount, RemainingReduction);
		BackFaceTargetBeeCount -= AdditionalBackReduction;
	}

	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ReduceVisibleFaceTargetBeeCountByAmount(int32 Amount)
{
	ReduceFaceTargetBeeCountByAmount(VisibleCombFace, Amount);
}

void ABeehiveCombActor::ReduceFaceTargetBeeCountByAmount(EBeehiveCombVisibleFace Face, int32 Amount)
{
	int32& FaceTargetBeeCount = GetMutableFaceTargetBeeCount(Face);
	FaceTargetBeeCount -= FMath::Max(0, Amount);
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ApplyNiagaraUserParameters()
{
	const int32 FrontSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Front);
	const int32 BackSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Back);

	if (FrontFaceBeeNiagara)
	{
		FrontFaceBeeNiagara->SetVariableVec2(BeehiveCombActorNames::PlaneSize, PlaneSize);
		FrontFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::SpawnAmount, FrontSpawnAmount);
		FrontFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::TargetBeeCount, FrontFaceTargetBeeCount);
		// FrontFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, FMath::Clamp(BeeDiseaseValue, 0.0f, 1.0f));
	}

	if (BackFaceBeeNiagara)
	{
		BackFaceBeeNiagara->SetVariableVec2(BeehiveCombActorNames::PlaneSize, PlaneSize);
		BackFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::SpawnAmount, BackSpawnAmount);
		BackFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::TargetBeeCount, BackFaceTargetBeeCount);
		// BackFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, FMath::Clamp(BeeDiseaseValue, 0.0f, 1.0f));
	}
}

void ABeehiveCombActor::RestartBeeNiagaraSystems()
{
	if (FrontFaceBeeNiagara)
	{
		FrontFaceBeeNiagara->ReinitializeSystem();
	}

	if (BackFaceBeeNiagara)
	{
		BackFaceBeeNiagara->ReinitializeSystem();
	}
}

void ABeehiveCombActor::SanitizeState()
{
	TotalSpawnAmount = FMath::Max(0, TotalSpawnAmount);

	const int32 FrontFaceSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Front);
	const int32 BackFaceSpawnAmount = GetFaceSpawnAmount(EBeehiveCombVisibleFace::Back);

	FrontFaceTargetBeeCount = FMath::Clamp(FrontFaceTargetBeeCount, 0, FrontFaceSpawnAmount);
	BackFaceTargetBeeCount = FMath::Clamp(BackFaceTargetBeeCount, 0, BackFaceSpawnAmount);

	int32 Overflow = GetTotalTargetBeeCount() - TotalSpawnAmount;
	if (Overflow > 0)
	{
		const int32 BackReduction = FMath::Min(Overflow, BackFaceTargetBeeCount);
		BackFaceTargetBeeCount -= BackReduction;
		Overflow -= BackReduction;
	}

	if (Overflow > 0)
	{
		const int32 FrontReduction = FMath::Min(Overflow, FrontFaceTargetBeeCount);
		FrontFaceTargetBeeCount -= FrontReduction;
	}
}

USceneComponent* ABeehiveCombActor::GetQueenAttachPoint(bool bFrontFace) const
{
	return bFrontFace ? QueenFrontAttachPoint.Get() : QueenBackAttachPoint.Get();
}

void ABeehiveCombActor::AddHoneyAmount(float DeltaHoney)
{
	CurrentHoney += FMath::Max(0.0f, DeltaHoney);
	SanitizeHoneyState();
	ApplyHoneyVisualState();
}

void ABeehiveCombActor::SetCurrentHoney(float NewHoneyAmount)
{
	CurrentHoney = NewHoneyAmount;
	SanitizeHoneyState();
	ApplyHoneyVisualState();
}

float ABeehiveCombActor::GetHoneyFillRatio() const
{
	const float SafeMaxHoney = FMath::Max(KINDA_SMALL_NUMBER, MaxHoneyPerComb);
	return FMath::Clamp(CurrentHoney / SafeMaxHoney, 0.0f, 1.0f);
}

bool ABeehiveCombActor::IsHoneyFull() const
{
	return GetHoneyFillRatio() >= 1.0f - KINDA_SMALL_NUMBER;
}

bool ABeehiveCombActor::ApplyWaxCappingBrush(UPrimitiveComponent* HitComponent, const FVector& WorldImpactPoint, float BrushRadiusCm)
{
	EBeehiveCombVisibleFace TargetFace = EBeehiveCombVisibleFace::Front;
	if (HitComponent == FrontWaxCappingUseAreaMesh)
	{
		TargetFace = EBeehiveCombVisibleFace::Front;
	}
	else if (HitComponent == BackWaxCappingUseAreaMesh)
	{
		TargetFace = EBeehiveCombVisibleFace::Back;
	}
	else
	{
		return false;
	}

	if (TargetFace != VisibleCombFace || !IsHoneyFull() || IsWaxCappingFaceComplete(TargetFace))
	{
		return false;
	}

	const float SafeBrushRadius = FMath::Max(0.0f, BrushRadiusCm);
	if (SafeBrushRadius <= 0.0f)
	{
		return false;
	}

	EnsureCappingMaskState();
	TArray<uint8>& Mask = GetMutableWaxCappingMask(TargetFace);
	const int32 PixelCount = CappingMaskWidth * CappingMaskHeight;
	if (CappingMaskWidth <= 0 || CappingMaskHeight <= 0 || Mask.Num() != PixelCount)
	{
		return false;
	}

	const UStaticMeshComponent* PaintComponent = (TargetFace == EBeehiveCombVisibleFace::Front)
		? FrontWaxCappingPlane.Get()
		: BackWaxCappingPlane.Get();
	if (!PaintComponent)
	{
		return false;
	}

	const FTransform PaintTransform = PaintComponent->GetComponentTransform();
	const FVector LocalImpactPoint = PaintTransform.InverseTransformPosition(WorldImpactPoint);
	const FVector2D PlaneSizeForMask = ResolveCappingPlaneSizeForMask();
	const float HalfPlaneX = PlaneSizeForMask.X * 0.5f;
	const float HalfPlaneY = PlaneSizeForMask.Y * 0.5f;
	const float BrushRadiusSquared = FMath::Square(SafeBrushRadius);
	const float WorldUnitsPerLocalX = FMath::Max(
		KINDA_SMALL_NUMBER,
		PaintTransform.TransformVector(FVector(1.0f, 0.0f, 0.0f)).Size());
	const float WorldUnitsPerLocalY = FMath::Max(
		KINDA_SMALL_NUMBER,
		PaintTransform.TransformVector(FVector(0.0f, 1.0f, 0.0f)).Size());
	const float LocalBrushRadiusX = SafeBrushRadius / WorldUnitsPerLocalX;
	const float LocalBrushRadiusY = SafeBrushRadius / WorldUnitsPerLocalY;

	const int32 MinX = FMath::Clamp(
		FMath::FloorToInt(((LocalImpactPoint.X - LocalBrushRadiusX + HalfPlaneX) / PlaneSizeForMask.X) * static_cast<float>(CappingMaskWidth)),
		0,
		CappingMaskWidth - 1);
	const int32 MaxX = FMath::Clamp(
		FMath::CeilToInt(((LocalImpactPoint.X + LocalBrushRadiusX + HalfPlaneX) / PlaneSizeForMask.X) * static_cast<float>(CappingMaskWidth)),
		0,
		CappingMaskWidth - 1);
	const int32 MinY = FMath::Clamp(
		FMath::FloorToInt(((LocalImpactPoint.Y - LocalBrushRadiusY + HalfPlaneY) / PlaneSizeForMask.Y) * static_cast<float>(CappingMaskHeight)),
		0,
		CappingMaskHeight - 1);
	const int32 MaxY = FMath::Clamp(
		FMath::CeilToInt(((LocalImpactPoint.Y + LocalBrushRadiusY + HalfPlaneY) / PlaneSizeForMask.Y) * static_cast<float>(CappingMaskHeight)),
		0,
		CappingMaskHeight - 1);

	bool bChanged = false;
	for (int32 Y = MinY; Y <= MaxY; ++Y)
	{
		const float PixelLocalY = ((static_cast<float>(Y) + 0.5f) / static_cast<float>(CappingMaskHeight) - 0.5f) * PlaneSizeForMask.Y;
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const float PixelLocalX = ((static_cast<float>(X) + 0.5f) / static_cast<float>(CappingMaskWidth) - 0.5f) * PlaneSizeForMask.X;
			const FVector WorldDelta = PaintTransform.TransformVector(FVector(
				PixelLocalX - LocalImpactPoint.X,
				PixelLocalY - LocalImpactPoint.Y,
				0.0f));
			if (WorldDelta.SizeSquared() > BrushRadiusSquared)
			{
				continue;
			}

			const int32 PixelIndex = (Y * CappingMaskWidth) + X;
			if (Mask.IsValidIndex(PixelIndex) && Mask[PixelIndex] > 0)
			{
				Mask[PixelIndex] = 0;
				bChanged = true;
			}
		}
	}

	if (bChanged)
	{
		UpdateCappingMaskTexture(TargetFace);
		ApplyWaxCappingMaskMaterialParameters();
		ApplyHoneyCappingVisualState();
	}

	return bChanged;
}

float ABeehiveCombActor::GetWaxCappingRemainingRatio(EBeehiveCombVisibleFace Face) const
{
	const TArray<uint8>& Mask = GetWaxCappingMask(Face);
	const int32 PixelCount = CappingMaskWidth * CappingMaskHeight;
	if (CappingMaskWidth <= 0 || CappingMaskHeight <= 0 || Mask.Num() != PixelCount || PixelCount <= 0)
	{
		return 1.0f;
	}

	int64 RemainingValue = 0;
	for (uint8 MaskValue : Mask)
	{
		RemainingValue += static_cast<int64>(MaskValue);
	}

	const double MaxValue = static_cast<double>(PixelCount) * 255.0;
	return MaxValue > 0.0 ? static_cast<float>(static_cast<double>(RemainingValue) / MaxValue) : 1.0f;
}

bool ABeehiveCombActor::IsWaxCappingFaceComplete(EBeehiveCombVisibleFace Face) const
{
	return GetWaxCappingRemainingRatio(Face) <= FMath::Clamp(UncappedThreshold, 0.0f, 1.0f);
}

bool ABeehiveCombActor::IsWaxCappingComplete() const
{
	return IsWaxCappingFaceComplete(EBeehiveCombVisibleFace::Front)
		&& IsWaxCappingFaceComplete(EBeehiveCombVisibleFace::Back);
}

void ABeehiveCombActor::AddHoneyRipeness(float DeltaRipeness)
{
	CurrentHoneyRipeness += FMath::Max(0.0f, DeltaRipeness);
	SanitizeHoneyRipenessState();
	ApplyHoneyVisualState();
}

void ABeehiveCombActor::SetCurrentHoneyRipeness(float NewHoneyRipeness)
{
	CurrentHoneyRipeness = NewHoneyRipeness;
	SanitizeHoneyRipenessState();
	ApplyHoneyVisualState();
}

float ABeehiveCombActor::GetHoneyRipenessRatio() const
{
	const float SafeMaxRipeness = FMath::Max(KINDA_SMALL_NUMBER, MaxHoneyRipeness);
	return FMath::Clamp(CurrentHoneyRipeness / SafeMaxRipeness, 0.0f, 1.0f);
}

void ABeehiveCombActor::SetBeeDiseaseValue(float NewDiseaseValue)
{
	BeeDiseaseValue = FMath::Clamp(NewDiseaseValue, 0.0f, 1.0f);
	// Disease is now represented by ABeehive::DiseaseVfxNiagara.
	// ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::SanitizeHoneyState()
{
	MaxHoneyPerComb = FMath::Max(KINDA_SMALL_NUMBER, MaxHoneyPerComb);
	CurrentHoney = FMath::Clamp(CurrentHoney, 0.0f, MaxHoneyPerComb);
}

void ABeehiveCombActor::SanitizeHoneyRipenessState()
{
	MaxHoneyRipeness = FMath::Max(KINDA_SMALL_NUMBER, MaxHoneyRipeness);
	CurrentHoneyRipeness = FMath::Clamp(CurrentHoneyRipeness, 0.0f, MaxHoneyRipeness);
}

void ABeehiveCombActor::EnsureCappingMaskState()
{
	CappingMaskLongSideResolution = FMath::Max(1, CappingMaskLongSideResolution);
	UncappedThreshold = FMath::Clamp(UncappedThreshold, 0.0f, 1.0f);

	int32 DesiredWidth = 0;
	int32 DesiredHeight = 0;
	ResolveCappingMaskDimensions(DesiredWidth, DesiredHeight);

	const int32 DesiredPixelCount = DesiredWidth * DesiredHeight;
	const bool bNeedsFullMask = DesiredWidth <= 0
		|| DesiredHeight <= 0
		|| CappingMaskWidth != DesiredWidth
		|| CappingMaskHeight != DesiredHeight
		|| FrontWaxCappingMask.Num() != DesiredPixelCount
		|| BackWaxCappingMask.Num() != DesiredPixelCount;

	if (bNeedsFullMask)
	{
		InitializeFullCappingMasks(DesiredWidth, DesiredHeight);
	}

	EnsureCappingMaskTextures();
}

void ABeehiveCombActor::InitializeFullCappingMasks(int32 NewWidth, int32 NewHeight)
{
	CappingMaskWidth = FMath::Max(1, NewWidth);
	CappingMaskHeight = FMath::Max(1, NewHeight);

	const int32 PixelCount = CappingMaskWidth * CappingMaskHeight;
	FrontWaxCappingMask.Init(255, PixelCount);
	BackWaxCappingMask.Init(255, PixelCount);
}

void ABeehiveCombActor::ResolveCappingMaskDimensions(int32& OutWidth, int32& OutHeight) const
{
	const FVector2D CappingPlaneSize = ResolveCappingPlaneSizeForMask();
	const int32 LongSideResolution = FMath::Max(1, CappingMaskLongSideResolution);

	if (CappingPlaneSize.X >= CappingPlaneSize.Y)
	{
		OutWidth = LongSideResolution;
		OutHeight = FMath::Max(1, FMath::RoundToInt(static_cast<float>(LongSideResolution) * (CappingPlaneSize.Y / CappingPlaneSize.X)));
	}
	else
	{
		OutHeight = LongSideResolution;
		OutWidth = FMath::Max(1, FMath::RoundToInt(static_cast<float>(LongSideResolution) * (CappingPlaneSize.X / CappingPlaneSize.Y)));
	}
}

FVector2D ABeehiveCombActor::ResolveCappingPlaneSizeForMask() const
{
	const float AbsX = FMath::Abs(PlaneSize.X);
	const float AbsY = FMath::Abs(PlaneSize.Y);
	if (AbsX <= KINDA_SMALL_NUMBER || AbsY <= KINDA_SMALL_NUMBER)
	{
		return FVector2D(100.0f, 100.0f);
	}

	return FVector2D(AbsX, AbsY);
}

bool ABeehiveCombActor::IsStoredCappingMaskValid(const FBeehiveCombItemState& CombState) const
{
	int32 DesiredWidth = 0;
	int32 DesiredHeight = 0;
	ResolveCappingMaskDimensions(DesiredWidth, DesiredHeight);

	const int32 DesiredPixelCount = DesiredWidth * DesiredHeight;
	return DesiredWidth > 0
		&& DesiredHeight > 0
		&& CombState.CappingMaskWidth == DesiredWidth
		&& CombState.CappingMaskHeight == DesiredHeight
		&& CombState.FrontWaxCappingMask.Num() == DesiredPixelCount
		&& CombState.BackWaxCappingMask.Num() == DesiredPixelCount;
}

TArray<uint8>& ABeehiveCombActor::GetMutableWaxCappingMask(EBeehiveCombVisibleFace Face)
{
	return Face == EBeehiveCombVisibleFace::Front ? FrontWaxCappingMask : BackWaxCappingMask;
}

const TArray<uint8>& ABeehiveCombActor::GetWaxCappingMask(EBeehiveCombVisibleFace Face) const
{
	return Face == EBeehiveCombVisibleFace::Front ? FrontWaxCappingMask : BackWaxCappingMask;
}

TObjectPtr<UTexture2D>& ABeehiveCombActor::GetWaxCappingMaskTextureRef(EBeehiveCombVisibleFace Face)
{
	return Face == EBeehiveCombVisibleFace::Front ? FrontWaxCappingMaskTexture : BackWaxCappingMaskTexture;
}

void ABeehiveCombActor::EnsureCappingMaskTextures()
{
	if (CappingMaskWidth <= 0 || CappingMaskHeight <= 0)
	{
		return;
	}

	for (const EBeehiveCombVisibleFace Face : { EBeehiveCombVisibleFace::Front, EBeehiveCombVisibleFace::Back })
	{
		TObjectPtr<UTexture2D>& Texture = GetWaxCappingMaskTextureRef(Face);
		if (!Texture || Texture->GetSizeX() != CappingMaskWidth || Texture->GetSizeY() != CappingMaskHeight)
		{
			Texture = UTexture2D::CreateTransient(CappingMaskWidth, CappingMaskHeight, PF_B8G8R8A8);
			if (Texture)
			{
				Texture->SRGB = false;
				Texture->NeverStream = true;
				Texture->Filter = TF_Nearest;
			}
		}

		UpdateCappingMaskTexture(Face);
	}
}

void ABeehiveCombActor::UpdateCappingMaskTexture(EBeehiveCombVisibleFace Face)
{
	TObjectPtr<UTexture2D>& Texture = GetWaxCappingMaskTextureRef(Face);
	const TArray<uint8>& Mask = GetWaxCappingMask(Face);
	const int32 PixelCount = CappingMaskWidth * CappingMaskHeight;
	if (!Texture || CappingMaskWidth <= 0 || CappingMaskHeight <= 0 || Mask.Num() != PixelCount)
	{
		return;
	}

	FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() <= 0)
	{
		return;
	}

	FTexture2DMipMap& Mip = PlatformData->Mips[0];
	void* RawData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!RawData)
	{
		return;
	}

	uint8* TextureBytes = static_cast<uint8*>(RawData);
	for (int32 Index = 0; Index < PixelCount; ++Index)
	{
		const uint8 MaskValue = Mask[Index];
		const int32 ByteOffset = Index * 4;
		TextureBytes[ByteOffset + 0] = MaskValue;
		TextureBytes[ByteOffset + 1] = MaskValue;
		TextureBytes[ByteOffset + 2] = MaskValue;
		TextureBytes[ByteOffset + 3] = MaskValue;
	}

	Mip.BulkData.Unlock();
	Texture->UpdateResource();
}

void ABeehiveCombActor::ApplyWaxCappingMaskMaterialParameters()
{
	EnsureCappingMaskTextures();

	if (FrontWaxCappingMaterialInstance && FrontWaxCappingMaskTexture)
	{
		FrontWaxCappingMaterialInstance->SetTextureParameterValue(WaxCappingMaskMaterialParameterName, FrontWaxCappingMaskTexture);
	}

	if (BackWaxCappingMaterialInstance && BackWaxCappingMaskTexture)
	{
		BackWaxCappingMaterialInstance->SetTextureParameterValue(WaxCappingMaskMaterialParameterName, BackWaxCappingMaskTexture);
	}
}

void ABeehiveCombActor::EnsureHoneyMaterialInstances()
{
	const bool bAllowDynamicMaterialInstances = BeehiveCombActorNames::IsGameWorldContext(this);

	if (FrontHoneyPlane)
	{
		UMaterialInterface* CurrentMaterial = BeehiveCombActorNames::RestoreDynamicMaterialParentInEditor(FrontHoneyPlane, 0);
		if (!bAllowDynamicMaterialInstances)
		{
			FrontHoneyMaterialInstance = nullptr;
		}
		else if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
		{
			FrontHoneyMaterialInstance = ExistingMID;
		}
		else if (!FrontHoneyMaterialInstance || CurrentMaterial != FrontHoneyMaterialInstance)
		{
			FrontHoneyMaterialInstance = FrontHoneyPlane->CreateDynamicMaterialInstance(0, CurrentMaterial);
		}
	}
	else
	{
		FrontHoneyMaterialInstance = nullptr;
	}

	if (BackHoneyPlane)
	{
		UMaterialInterface* CurrentMaterial = BeehiveCombActorNames::RestoreDynamicMaterialParentInEditor(BackHoneyPlane, 0);
		if (!bAllowDynamicMaterialInstances)
		{
			BackHoneyMaterialInstance = nullptr;
		}
		else if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
		{
			BackHoneyMaterialInstance = ExistingMID;
		}
		else if (!BackHoneyMaterialInstance || CurrentMaterial != BackHoneyMaterialInstance)
		{
			BackHoneyMaterialInstance = BackHoneyPlane->CreateDynamicMaterialInstance(0, CurrentMaterial);
		}
	}
	else
	{
		BackHoneyMaterialInstance = nullptr;
	}

	if (FrontWaxCappingPlane)
	{
		UMaterialInterface* CurrentMaterial = BeehiveCombActorNames::RestoreDynamicMaterialParentInEditor(FrontWaxCappingPlane, 0);
		if (!bAllowDynamicMaterialInstances)
		{
			FrontWaxCappingMaterialInstance = nullptr;
		}
		else if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
		{
			FrontWaxCappingMaterialInstance = ExistingMID;
		}
		else if (!FrontWaxCappingMaterialInstance || CurrentMaterial != FrontWaxCappingMaterialInstance)
		{
			FrontWaxCappingMaterialInstance = FrontWaxCappingPlane->CreateDynamicMaterialInstance(0, CurrentMaterial);
		}
	}
	else
	{
		FrontWaxCappingMaterialInstance = nullptr;
	}

	if (BackWaxCappingPlane)
	{
		UMaterialInterface* CurrentMaterial = BeehiveCombActorNames::RestoreDynamicMaterialParentInEditor(BackWaxCappingPlane, 0);
		if (!bAllowDynamicMaterialInstances)
		{
			BackWaxCappingMaterialInstance = nullptr;
		}
		else if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
		{
			BackWaxCappingMaterialInstance = ExistingMID;
		}
		else if (!BackWaxCappingMaterialInstance || CurrentMaterial != BackWaxCappingMaterialInstance)
		{
			BackWaxCappingMaterialInstance = BackWaxCappingPlane->CreateDynamicMaterialInstance(0, CurrentMaterial);
		}
	}
	else
	{
		BackWaxCappingMaterialInstance = nullptr;
	}
}

void ABeehiveCombActor::ApplyHoneyVisualState()
{
	EnsureHoneyMaterialInstances();
	const float FillRatio = GetHoneyFillRatio();
	const float RipenessRatio = GetHoneyRipenessRatio();

	if (FrontHoneyPlane)
	{
		FrontHoneyPlane->SetRelativeLocation(FMath::Lerp(FrontHoneyEmptyRelativeLocation, FrontHoneyFullRelativeLocation, FillRatio));
	}

	if (BackHoneyPlane)
	{
		BackHoneyPlane->SetRelativeLocation(FMath::Lerp(BackHoneyEmptyRelativeLocation, BackHoneyFullRelativeLocation, FillRatio));
	}

	if (FrontHoneyMaterialInstance)
	{
		FrontHoneyMaterialInstance->SetScalarParameterValue(HoneyMaterialParameterName, FillRatio);
		FrontHoneyMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio);
	}

	if (BackHoneyMaterialInstance)
	{
		BackHoneyMaterialInstance->SetScalarParameterValue(HoneyMaterialParameterName, FillRatio);
		BackHoneyMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio);
	}

	if (FrontWaxCappingMaterialInstance)
	{
		FrontWaxCappingMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio);
	}

	if (BackWaxCappingMaterialInstance)
	{
		BackWaxCappingMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio);
	}

	ApplyWaxCappingMaskMaterialParameters();
	ApplyHoneyCappingVisualState();
}

void ABeehiveCombActor::ApplyHoneyCappingVisualState()
{
	const bool bShowFrontCapping = IsHoneyFull() && !IsWaxCappingFaceComplete(EBeehiveCombVisibleFace::Front);
	const bool bShowBackCapping = IsHoneyFull() && !IsWaxCappingFaceComplete(EBeehiveCombVisibleFace::Back);

	if (FrontWaxCappingPlane)
	{
		FrontWaxCappingPlane->SetHiddenInGame(!bShowFrontCapping);
	}

	if (BackWaxCappingPlane)
	{
		BackWaxCappingPlane->SetHiddenInGame(!bShowBackCapping);
	}
}

void ABeehiveCombActor::FlipCombFace()
{
	FlipCombFaceWithDirection(EBeehiveCombFlipDirection::Right);
}

void ABeehiveCombActor::FlipCombFaceWithDirection(EBeehiveCombFlipDirection FlipDirection)
{
	const EBeehiveCombVisibleFace NewFace = (VisibleCombFace == EBeehiveCombVisibleFace::Front)
		? EBeehiveCombVisibleFace::Back
		: EBeehiveCombVisibleFace::Front;
	SetVisibleCombFace(NewFace);
	ReceiveCombFlipped(VisibleCombFace);
	ReceiveCombFlippedWithDirection(VisibleCombFace, FlipDirection);
}

void ABeehiveCombActor::SetVisibleCombFace(EBeehiveCombVisibleFace NewFace)
{
	VisibleCombFace = NewFace;
	ApplyHoneyCappingVisualState();
}

void ABeehiveCombActor::SyncVisibleCombFacePresentation()
{
	if (!CombPivotRoot)
	{
		return;
	}

	FRotator PivotRotation = CombPivotRoot->GetRelativeRotation();
	PivotRotation.Yaw = (VisibleCombFace == EBeehiveCombVisibleFace::Back) ? 180.0f : 0.0f;
	CombPivotRoot->SetRelativeRotation(PivotRotation);
}

void ABeehiveCombActor::ApplyCombShakeByRatio(float ReductionRatio)
{
	ApplyCombShakeByRatioWithStrokeCount(ReductionRatio, 0);
}

void ABeehiveCombActor::ApplyCombShakeByRatioWithStrokeCount(float ReductionRatio, int32 StrokeCount)
{
	const float ClampedRatio = FMath::Clamp(ReductionRatio, 0.0f, 1.0f);
	ReduceAllTargetBeeCountsByRatio(ClampedRatio);
	ReceiveCombShaken(FMath::Max(0, StrokeCount), ClampedRatio);
}

void ABeehiveCombActor::ApplyStateFromItemInstance(const UItemInstance* SourceItemInstance)
{
	if (!SourceItemInstance || !SourceItemInstance->HasBeehiveCombState())
	{
		EnsureCappingMaskState();
		ApplyHoneyVisualState();
		return;
	}

	const FBeehiveCombItemState CombState = SourceItemInstance->GetBeehiveCombState();
	SetCurrentHoney(CombState.HoneyAmount);
	SetCurrentHoneyRipeness(CombState.HoneyRipeness);
	SetVisibleCombFace(CombState.bIsFrontFaceVisible ? EBeehiveCombVisibleFace::Front : EBeehiveCombVisibleFace::Back);
	SyncVisibleCombFacePresentation();

	int32 DesiredWidth = 0;
	int32 DesiredHeight = 0;
	ResolveCappingMaskDimensions(DesiredWidth, DesiredHeight);
	if (IsStoredCappingMaskValid(CombState))
	{
		CappingMaskWidth = DesiredWidth;
		CappingMaskHeight = DesiredHeight;
		FrontWaxCappingMask = CombState.FrontWaxCappingMask;
		BackWaxCappingMask = CombState.BackWaxCappingMask;
	}
	else
	{
		InitializeFullCappingMasks(DesiredWidth, DesiredHeight);
	}

	EnsureCappingMaskTextures();
	ApplyHoneyVisualState();
}

void ABeehiveCombActor::WriteStateToItemInstance(UItemInstance* TargetItemInstance) const
{
	if (!TargetItemInstance)
	{
		return;
	}

	const bool bIsFrontFaceVisible = (VisibleCombFace == EBeehiveCombVisibleFace::Front);
	TargetItemInstance->SetBeehiveCombStateWithCapping(
		CurrentHoney,
		CurrentHoneyRipeness,
		bIsFrontFaceVisible,
		CappingMaskWidth,
		CappingMaskHeight,
		FrontWaxCappingMask,
		BackWaxCappingMask);
}

bool ABeehiveCombActor::IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const
{
	if (Component == FrontWaxCappingUseAreaMesh || Component == BackWaxCappingUseAreaMesh)
	{
		const bool bFrontFaceComponent = (Component == FrontWaxCappingUseAreaMesh);
		const EBeehiveCombVisibleFace ComponentFace = bFrontFaceComponent ? EBeehiveCombVisibleFace::Front : EBeehiveCombVisibleFace::Back;
		return Cast<AUncappingTable>(HostActor) != nullptr
			&& IsHoneyFull()
			&& VisibleCombFace == ComponentFace
			&& !IsWaxCappingFaceComplete(ComponentFace);
	}

	if (Component != BeeBrushUseAreaMesh)
	{
		return true;
	}

	const ABeehive* BeehiveHost = Cast<ABeehive>(HostActor);
	return BeehiveHost && (BeehiveHost->GetLiftedCombActor() == this);
}

#if WITH_EDITOR
void ABeehiveCombActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SanitizeState();
	SanitizeHoneyState();
	SanitizeHoneyRipenessState();
	EnsureCappingMaskState();
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();
}
#endif
