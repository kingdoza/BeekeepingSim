#include "WorldActors/BeehiveCombActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "Inventory/ItemInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeehiveCombPartFocusActionComponent.h"
#include "WorldActors/BeehiveCombPlacementOccupantComponent.h"
#include "WorldActors/PlacementSlotRetrievePartFocusActionComponent.h"

namespace BeehiveCombActorNames
{
	static const FName PlaneSize(TEXT("User.PlaneSize"));
	static const FName SpawnAmount(TEXT("User.SpawnAmount"));
	static const FName TargetBeeCount(TEXT("User.TargetBeeCount"));
	static const FName Disease(TEXT("User.Disease"));
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

	BeeBrushUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("BeeBrushUseAreaMesh"));
	BeeBrushUseAreaMesh->SetupAttachment(CombMesh);
	BeeBrushUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
	BeeBrushUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeeBrushUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ABeehiveCombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SanitizeState();
	SanitizeHoneyState();
	SanitizeHoneyRipenessState();
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
		FrontFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, FMath::Clamp(BeeDiseaseValue, 0.0f, 1.0f));
	}

	if (BackFaceBeeNiagara)
	{
		BackFaceBeeNiagara->SetVariableVec2(BeehiveCombActorNames::PlaneSize, PlaneSize);
		BackFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::SpawnAmount, BackSpawnAmount);
		BackFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::TargetBeeCount, BackFaceTargetBeeCount);
		BackFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, FMath::Clamp(BeeDiseaseValue, 0.0f, 1.0f));
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
	ApplyNiagaraUserParameters();
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

void ABeehiveCombActor::EnsureHoneyMaterialInstances()
{
	if (FrontHoneyPlane)
	{
		UMaterialInterface* CurrentMaterial = FrontHoneyPlane->GetMaterial(0);
		if (!FrontHoneyMaterialInstance || CurrentMaterial != FrontHoneyMaterialInstance)
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
		UMaterialInterface* CurrentMaterial = BackHoneyPlane->GetMaterial(0);
		if (!BackHoneyMaterialInstance || CurrentMaterial != BackHoneyMaterialInstance)
		{
			BackHoneyMaterialInstance = BackHoneyPlane->CreateDynamicMaterialInstance(0, CurrentMaterial);
		}
	}
	else
	{
		BackHoneyMaterialInstance = nullptr;
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
		return;
	}

	const FBeehiveCombItemState CombState = SourceItemInstance->GetBeehiveCombState();
	SetCurrentHoney(CombState.HoneyAmount);
	SetCurrentHoneyRipeness(CombState.HoneyRipeness);
	SetVisibleCombFace(CombState.bIsFrontFaceVisible ? EBeehiveCombVisibleFace::Front : EBeehiveCombVisibleFace::Back);
}

void ABeehiveCombActor::WriteStateToItemInstance(UItemInstance* TargetItemInstance) const
{
	if (!TargetItemInstance)
	{
		return;
	}

	const bool bIsFrontFaceVisible = (VisibleCombFace == EBeehiveCombVisibleFace::Front);
	TargetItemInstance->SetBeehiveCombStateWithRipeness(CurrentHoney, CurrentHoneyRipeness, bIsFrontFaceVisible);
}

bool ABeehiveCombActor::IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const
{
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
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();
}
#endif
