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
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();
}

void ABeehiveCombActor::BeginPlay()
{
	Super::BeginPlay();
	SanitizeState();
	SanitizeHoneyState();
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();
}

void ABeehiveCombActor::ApplyCombBeeParameters(const FVector2D& InPlaneSize, int32 InSpawnAmount, int32 InTargetBeeCount)
{
	const int32 PreviousSpawnAmount = SpawnAmount;
	PlaneSize = InPlaneSize;
	SpawnAmount = InSpawnAmount;
	TargetBeeCount = InTargetBeeCount;
	SanitizeState();
	ApplyNiagaraUserParameters();
	if (SpawnAmount != PreviousSpawnAmount)
	{
		RestartBeeNiagaraSystems();
	}
}

void ABeehiveCombActor::SetSpawnAmountAndResetTargetBeeCount(const FVector2D& InPlaneSize, int32 InSpawnAmount)
{
	const int32 PreviousSpawnAmount = SpawnAmount;
	PlaneSize = InPlaneSize;
	SpawnAmount = InSpawnAmount;
	TargetBeeCount = SpawnAmount;
	SanitizeState();
	ApplyNiagaraUserParameters();
	if (SpawnAmount != PreviousSpawnAmount)
	{
		RestartBeeNiagaraSystems();
	}
}

void ABeehiveCombActor::SetTargetBeeCount(int32 NewTargetBeeCount)
{
	TargetBeeCount = NewTargetBeeCount;
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ResetTargetBeeCountToSpawnAmount()
{
	TargetBeeCount = SpawnAmount;
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ReduceTargetBeeCountByRatio(float Ratio)
{
	const float ClampedRatio = FMath::Clamp(Ratio, 0.0f, 1.0f);
	const int32 Reduction = FMath::RoundToInt(static_cast<float>(TargetBeeCount) * ClampedRatio);
	ReduceTargetBeeCountByAmount(Reduction);
}

void ABeehiveCombActor::ReduceTargetBeeCountByAmount(int32 Amount)
{
	const int32 ClampedAmount = FMath::Max(0, Amount);
	TargetBeeCount -= ClampedAmount;
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ApplyNiagaraUserParameters()
{
	if (FrontFaceBeeNiagara)
	{
		FrontFaceBeeNiagara->SetVariableVec2(BeehiveCombActorNames::PlaneSize, PlaneSize);
		FrontFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::SpawnAmount, SpawnAmount);
		FrontFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::TargetBeeCount, TargetBeeCount);
	}

	if (BackFaceBeeNiagara)
	{
		BackFaceBeeNiagara->SetVariableVec2(BeehiveCombActorNames::PlaneSize, PlaneSize);
		BackFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::SpawnAmount, SpawnAmount);
		BackFaceBeeNiagara->SetVariableInt(BeehiveCombActorNames::TargetBeeCount, TargetBeeCount);
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
	SpawnAmount = FMath::Max(0, SpawnAmount);
	TargetBeeCount = FMath::Clamp(TargetBeeCount, 0, SpawnAmount);
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

void ABeehiveCombActor::SanitizeHoneyState()
{
	MaxHoneyPerComb = FMath::Max(KINDA_SMALL_NUMBER, MaxHoneyPerComb);
	CurrentHoney = FMath::Clamp(CurrentHoney, 0.0f, MaxHoneyPerComb);
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
	}

	if (BackHoneyMaterialInstance)
	{
		BackHoneyMaterialInstance->SetScalarParameterValue(HoneyMaterialParameterName, FillRatio);
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
	ReduceTargetBeeCountByRatio(ClampedRatio);
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
	SetVisibleCombFace(CombState.bIsFrontFaceVisible ? EBeehiveCombVisibleFace::Front : EBeehiveCombVisibleFace::Back);
}

void ABeehiveCombActor::WriteStateToItemInstance(UItemInstance* TargetItemInstance) const
{
	if (!TargetItemInstance)
	{
		return;
	}

	const bool bIsFrontFaceVisible = (VisibleCombFace == EBeehiveCombVisibleFace::Front);
	TargetItemInstance->SetBeehiveCombState(CurrentHoney, bIsFrontFaceVisible);
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
	ApplyNiagaraUserParameters();
	ApplyHoneyVisualState();
}
#endif
