#include "WorldActors/BeehiveCombActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "NiagaraComponent.h"

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

	CombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CombMesh"));
	CombMesh->SetupAttachment(Root);

	FrontFaceBeeNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FrontFaceBeeNiagara"));
	FrontFaceBeeNiagara->SetupAttachment(Root);

	BackFaceBeeNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BackFaceBeeNiagara"));
	BackFaceBeeNiagara->SetupAttachment(Root);

	PartFocusAction = CreateDefaultSubobject<UCursorPartFocusActionComponent>(TEXT("PartFocusAction"));
	if (PartFocusAction)
	{
		PartFocusAction->SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
		FGameplayTagContainer RequiredTags;
		RequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.LidOpen")), false));
		PartFocusAction->SetRequiredStateTags(RequiredTags);
		PartFocusAction->SetExclusiveGroup(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.CombLift")), false));
	}
}

void ABeehiveCombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::BeginPlay()
{
	Super::BeginPlay();
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::ApplyCombBeeParameters(const FVector2D& InPlaneSize, int32 InSpawnAmount, int32 InTargetBeeCount)
{
	PlaneSize = InPlaneSize;
	SpawnAmount = InSpawnAmount;
	TargetBeeCount = InTargetBeeCount;
	SanitizeState();
	ApplyNiagaraUserParameters();
}

void ABeehiveCombActor::SetSpawnAmountAndResetTargetBeeCount(const FVector2D& InPlaneSize, int32 InSpawnAmount)
{
	PlaneSize = InPlaneSize;
	SpawnAmount = InSpawnAmount;
	TargetBeeCount = SpawnAmount;
	SanitizeState();
	ApplyNiagaraUserParameters();
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

void ABeehiveCombActor::SanitizeState()
{
	SpawnAmount = FMath::Max(0, SpawnAmount);
	TargetBeeCount = FMath::Clamp(TargetBeeCount, 0, SpawnAmount);
}

#if WITH_EDITOR
void ABeehiveCombActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SanitizeState();
	ApplyNiagaraUserParameters();
}
#endif
