#include "WorldActors/BeeSwarmClusterActor.h"

#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "Focus/AnchoredFocusCursorActionComponent.h"
#include "Focus/CursorItemUseAreaScopeComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "Focus/ItemUseAreaMeshProviderComponent.h"
#include "GameplayTagContainer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "WorldActors/QueenBeeActor.h"

namespace BeeSwarmClusterNames
{
	static const FName BeeCarrierUseAreaTag(TEXT("Item.UseArea.SwarmCluster.BeeCarrier"));
	static const FName UseAreaColorParameter(TEXT("UseAreaColor"));
	static const FName UseAreaOpacityParameter(TEXT("UseAreaOpacity"));
	static const FName PulseSpeedParameter(TEXT("PulseSpeed"));
	static const FName HoverStrengthParameter(TEXT("HoverStrength"));
}

ABeeSwarmClusterActor::ABeeSwarmClusterActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ClusterCenter = CreateDefaultSubobject<USceneComponent>(TEXT("ClusterCenter"));
	ClusterCenter->SetupAttachment(Root);

	ClusterNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ClusterNiagara"));
	ClusterNiagara->SetupAttachment(ClusterCenter);

	QueenBeeChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("QueenBeeChildActor"));
	QueenBeeChildActor->SetupAttachment(ClusterCenter);

	CaptureUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("CaptureUseAreaMesh"));
	CaptureUseAreaMesh->SetupAttachment(ClusterCenter);
	CaptureUseAreaMesh->SetAreaId(TEXT("SwarmCluster.BeeCarrier"));
	CaptureUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
	CaptureUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CaptureUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	const FGameplayTag BeeCarrierUseAreaTag = FGameplayTag::RequestGameplayTag(BeeSwarmClusterNames::BeeCarrierUseAreaTag, false);
	if (BeeCarrierUseAreaTag.IsValid())
	{
		FGameplayTagContainer AreaTags;
		AreaTags.AddTag(BeeCarrierUseAreaTag);
		CaptureUseAreaMesh->SetAreaTags(AreaTags);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s could not resolve gameplay tag %s."),
			*GetName(),
			*BeeSwarmClusterNames::BeeCarrierUseAreaTag.ToString());
	}

	FocusAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FocusAnchor"));
	FocusAnchor->SetupAttachment(Root);
	FocusAnchor->ComponentTags.AddUnique(TEXT("FocusAnchor"));

	CharacterAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CharacterAnchor"));
	CharacterAnchor->SetupAttachment(Root);
	CharacterAnchor->ComponentTags.AddUnique(TEXT("CharacterAnchor"));

	FocusTarget = CreateDefaultSubobject<UFocusTargetComponent>(TEXT("FocusTarget"));
	if (FocusTarget)
	{
		FocusTarget->SetDisplayName(FText::FromString(TEXT("Bee Swarm Cluster")));
	}

	FocusAction = CreateDefaultSubobject<UAnchoredFocusCursorActionComponent>(TEXT("FocusAction"));
	ItemUseAreaScope = CreateDefaultSubobject<UCursorItemUseAreaScopeComponent>(TEXT("ItemUseAreaScope"));
	ItemUseAreaMeshProvider = CreateDefaultSubobject<UItemUseAreaMeshProviderComponent>(TEXT("ItemUseAreaMeshProvider"));

	SwarmQueenBeeActorClass = AQueenBeeActor::StaticClass();
}

void ABeeSwarmClusterActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SpawnAmount = FMath::Max(0, SpawnAmount);
	BeeDensityPerCubicMeter = SanitizeBeeDensityPerCubicMeter(BeeDensityPerCubicMeter);
	CapturedBeeAmount = FMath::Clamp(CapturedBeeAmount, 0.0f, GetTotalBeeAmount());
	if (bCaptured)
	{
		bBeesCaptured = true;
		bQueenCaptured = true;
	}
	if (bBeesCaptured)
	{
		CapturedBeeAmount = GetTotalBeeAmount();
	}
	RecalculateInitialRadiusFromDensity();
	EnsureQueenBeeChildActorClass();
	ApplyQueenBeeTransform();
	ApplyClusterNiagaraParameters();
	SetCaptureUseAreaActive(!bBeesCaptured);
	ApplyCaptureUseAreaVisualIdleState();
}

void ABeeSwarmClusterActor::BeginPlay()
{
	Super::BeginPlay();
	SpawnAmount = FMath::Max(0, SpawnAmount);
	BeeDensityPerCubicMeter = SanitizeBeeDensityPerCubicMeter(BeeDensityPerCubicMeter);
	CapturedBeeAmount = FMath::Clamp(CapturedBeeAmount, 0.0f, GetTotalBeeAmount());
	if (bCaptured)
	{
		bBeesCaptured = true;
		bQueenCaptured = true;
	}
	if (bBeesCaptured)
	{
		CapturedBeeAmount = GetTotalBeeAmount();
	}
	RecalculateInitialRadiusFromDensity();
	EnsureQueenBeeChildActorClass();
	ApplyQueenBeeTransform();
	ApplyClusterNiagaraParameters();
	SetCaptureUseAreaActive(!bBeesCaptured);
	ApplyCaptureUseAreaVisualIdleState();
	HandleCapturedIfNeeded();
}

void ABeeSwarmClusterActor::InitializeSwarmClusterFromDensity(int32 InSpawnAmount, float InBeeDensityPerCubicMeter)
{
	bCaptured = false;
	bBeesCaptured = false;
	bQueenCaptured = false;
	SpawnAmount = FMath::Max(0, InSpawnAmount);
	CapturedBeeAmount = 0.0f;
	BeeDensityPerCubicMeter = SanitizeBeeDensityPerCubicMeter(InBeeDensityPerCubicMeter);
	RecalculateInitialRadiusFromDensity();

	EnsureQueenBeeChildActorClass();
	ApplyQueenBeeTransform();
	RandomizeSwarmQueenRotation();
	SetCaptureUseAreaActive(true);
	ApplyCaptureUseAreaVisualIdleState();
	ApplyClusterNiagaraParameters();
	ReceiveSwarmClusterInitialized();
	ReceiveAliveRadiusChanged(AliveRadius);
	HandleCapturedIfNeeded();
}

void ABeeSwarmClusterActor::ApplyClusterNiagaraParameters()
{
	if (!ClusterNiagara)
	{
		return;
	}

	if (!AliveRadiusParameterName.IsNone())
	{
		ClusterNiagara->SetVariableFloat(AliveRadiusParameterName, FMath::Max(0.0f, AliveRadius));
	}

	if (!SpawnAmountParameterName.IsNone())
	{
		ClusterNiagara->SetVariableInt(SpawnAmountParameterName, FMath::Max(0, SpawnAmount));
	}

	if (!SphereRadiusParameterName.IsNone())
	{
		ClusterNiagara->SetVariableFloat(SphereRadiusParameterName, FMath::Max(0.0f, SphereRadius));
	}
}

float ABeeSwarmClusterActor::DecreaseAliveRadius(float DeltaRadius)
{
	// Legacy/manual visual adjustment only. BeeCarrier capture gameplay must use CaptureBees so bee amounts stay authoritative.
	const float OldAliveRadius = AliveRadius;
	SetAliveRadius(AliveRadius - FMath::Max(0.0f, DeltaRadius));
	return FMath::Max(0.0f, OldAliveRadius - AliveRadius);
}

void ABeeSwarmClusterActor::SetAliveRadius(float NewAliveRadius)
{
	const float ClampedAliveRadius = bBeesCaptured ? 0.0f : FMath::Max(0.0f, NewAliveRadius);
	if (FMath::IsNearlyEqual(AliveRadius, ClampedAliveRadius))
	{
		HandleCapturedIfNeeded();
		return;
	}

	AliveRadius = ClampedAliveRadius;
	ApplyClusterNiagaraParameters();
	ReceiveAliveRadiusChanged(AliveRadius);
	HandleCapturedIfNeeded();
}

float ABeeSwarmClusterActor::CaptureBees(float RequestedBeeAmount)
{
	if (bBeesCaptured || RequestedBeeAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float TotalBeeAmount = GetTotalBeeAmount();
	if (TotalBeeAmount <= KINDA_SMALL_NUMBER)
	{
		HandleCapturedIfNeeded();
		return 0.0f;
	}

	const float ActualCaptured = FMath::Clamp(RequestedBeeAmount, 0.0f, GetRemainingBeeAmount());
	if (ActualCaptured <= KINDA_SMALL_NUMBER)
	{
		HandleCapturedIfNeeded();
		return 0.0f;
	}

	SetCapturedBeeAmount(CapturedBeeAmount + ActualCaptured);
	return ActualCaptured;
}

void ABeeSwarmClusterActor::SetCapturedBeeAmount(float NewCapturedBeeAmount)
{
	const float TotalBeeAmount = GetTotalBeeAmount();
	if (bBeesCaptured)
	{
		CapturedBeeAmount = TotalBeeAmount;
		AliveRadius = 0.0f;
		ApplyClusterNiagaraParameters();
		HandleSwarmCapturedIfNeeded();
		return;
	}

	CapturedBeeAmount = FMath::Clamp(NewCapturedBeeAmount, 0.0f, TotalBeeAmount);

	const float OldAliveRadius = AliveRadius;
	RefreshAliveRadiusFromBeeAmounts();
	ApplyClusterNiagaraParameters();

	if (!FMath::IsNearlyEqual(OldAliveRadius, AliveRadius))
	{
		ReceiveAliveRadiusChanged(AliveRadius);
	}

	HandleCapturedIfNeeded();
}

int32 ABeeSwarmClusterActor::GetCapturedBeeCountRounded() const
{
	return FMath::Max(0, FMath::RoundToInt(GetCapturedBeeAmount()));
}

float ABeeSwarmClusterActor::GetRemainingBeeAmount() const
{
	const float TotalBeeAmount = GetTotalBeeAmount();
	return FMath::Clamp(TotalBeeAmount - FMath::Max(0.0f, CapturedBeeAmount), 0.0f, TotalBeeAmount);
}

int32 ABeeSwarmClusterActor::GetRemainingBeeCountRounded() const
{
	return FMath::Max(0, FMath::RoundToInt(GetRemainingBeeAmount()));
}

float ABeeSwarmClusterActor::GetTotalBeeAmount() const
{
	return FMath::Max(0.0f, static_cast<float>(SpawnAmount));
}

float ABeeSwarmClusterActor::CalculateAliveRadiusFromRemainingBees() const
{
	const float TotalBeeAmount = GetTotalBeeAmount();
	if (TotalBeeAmount <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const float RemainingRatio = FMath::Clamp(GetRemainingBeeAmount() / TotalBeeAmount, 0.0f, 1.0f);
	return FMath::Max(0.0f, InitialAliveRadius) * FMath::Pow(RemainingRatio, 1.0f / 3.0f);
}

void ABeeSwarmClusterActor::RefreshAliveRadiusFromBeeAmounts()
{
	AliveRadius = CalculateAliveRadiusFromRemainingBees();
}

float ABeeSwarmClusterActor::SanitizeBeeDensityPerCubicMeter(float InDensity)
{
	return FMath::Max(0.0001f, InDensity);
}

float ABeeSwarmClusterActor::CalculateRadiusCmFromBeeDensity(int32 InSpawnAmount, float InDensityPerCubicMeter)
{
	const int32 SafeSpawnAmount = FMath::Max(0, InSpawnAmount);
	if (SafeSpawnAmount <= 0)
	{
		return 0.0f;
	}

	const float SafeDensity = SanitizeBeeDensityPerCubicMeter(InDensityPerCubicMeter);
	const float VolumeM3 = static_cast<float>(SafeSpawnAmount) / SafeDensity;
	const float RadiusM = FMath::Pow((3.0f * VolumeM3) / (4.0f * UE_PI), 1.0f / 3.0f);
	return FMath::Max(0.0f, RadiusM * 100.0f);
}

void ABeeSwarmClusterActor::RecalculateInitialRadiusFromDensity()
{
	BeeDensityPerCubicMeter = SanitizeBeeDensityPerCubicMeter(BeeDensityPerCubicMeter);
	InitialAliveRadius = CalculateRadiusCmFromBeeDensity(SpawnAmount, BeeDensityPerCubicMeter);
	SphereRadius = InitialAliveRadius;
	RefreshAliveRadiusFromBeeAmounts();
	if (bBeesCaptured)
	{
		AliveRadius = 0.0f;
	}
}

AQueenBeeActor* ABeeSwarmClusterActor::GetQueenBeeActor() const
{
	if (bQueenCaptured)
	{
		return nullptr;
	}

	return QueenBeeChildActor ? Cast<AQueenBeeActor>(QueenBeeChildActor->GetChildActor()) : nullptr;
}

void ABeeSwarmClusterActor::RebuildItemUseAreaDescriptors()
{
	if (ItemUseAreaScope)
	{
		ItemUseAreaScope->RebuildItemUseAreaDescriptors();
	}
}

bool ABeeSwarmClusterActor::IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const
{
	(void)HostActor;
	return Component != CaptureUseAreaMesh || !IsBeesCaptured();
}

bool ABeeSwarmClusterActor::CanCaptureQueenBee_Implementation(AQueenBeeActor* QueenBee) const
{
	return !bQueenCaptured
		&& QueenBee
		&& !QueenBee->IsCaptured()
		&& QueenBee == GetQueenBeeActor();
}

bool ABeeSwarmClusterActor::CaptureQueenBee_Implementation(AQueenBeeActor* QueenBee, FQueenCageItemState& OutCapturedState)
{
	OutCapturedState = FQueenCageItemState();
	if (!CanCaptureQueenBee_Implementation(QueenBee))
	{
		return false;
	}

	OutCapturedState = QueenBee->MakeQueenCageItemState();
	bQueenCaptured = true;
	QueenBee->SetCaptured(true);

	if (QueenBeeChildActor && QueenBeeChildActor->GetChildActorClass() != nullptr)
	{
		QueenBeeChildActor->SetChildActorClass(nullptr);
	}

	RebuildItemUseAreaDescriptors();
	HandleSwarmCapturedIfNeeded();
	return OutCapturedState.bHasState && OutCapturedState.bHasQueen && OutCapturedState.CapturedQueenBeeClass;
}

void ABeeSwarmClusterActor::EnsureQueenBeeChildActorClass()
{
	if (!QueenBeeChildActor)
	{
		return;
	}

	if (bQueenCaptured || bCaptured || !SwarmQueenBeeActorClass)
	{
		if (QueenBeeChildActor->GetChildActorClass() != nullptr)
		{
			if (AQueenBeeActor* QueenBee = Cast<AQueenBeeActor>(QueenBeeChildActor->GetChildActor()))
			{
				QueenBee->SetCaptured(true);
			}
			QueenBeeChildActor->SetChildActorClass(nullptr);
		}
		return;
	}

	const TSubclassOf<AActor> ActiveQueenClass = SwarmQueenBeeActorClass
		? TSubclassOf<AActor>(SwarmQueenBeeActorClass)
		: nullptr;
	if (QueenBeeChildActor->GetChildActorClass() != ActiveQueenClass)
	{
		QueenBeeChildActor->SetChildActorClass(ActiveQueenClass);
	}
}

void ABeeSwarmClusterActor::ApplyQueenBeeTransform()
{
	if (!QueenBeeChildActor)
	{
		return;
	}

	if (ClusterCenter && QueenBeeChildActor->GetAttachParent() != ClusterCenter)
	{
		QueenBeeChildActor->AttachToComponent(ClusterCenter, FAttachmentTransformRules::KeepRelativeTransform);
	}

	QueenBeeChildActor->SetRelativeLocation(QueenCenterOffset);
}

void ABeeSwarmClusterActor::RandomizeSwarmQueenRotation()
{
	if (!QueenBeeChildActor || bQueenCaptured || bCaptured)
	{
		return;
	}

	const FRotator RandomQueenRotation(
		FMath::FRandRange(0.0f, 360.0f),
		FMath::FRandRange(0.0f, 360.0f),
		FMath::FRandRange(0.0f, 360.0f));
	QueenBeeChildActor->SetRelativeRotation(RandomQueenRotation);
}

void ABeeSwarmClusterActor::ApplyCaptureUseAreaVisualIdleState()
{
	if (!CaptureUseAreaMesh)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	UMaterialInterface* CurrentMaterial = CaptureUseAreaMesh->GetMaterial(0);
	if (!CurrentMaterial)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(CurrentMaterial);
	if (!DynamicMaterial)
	{
		DynamicMaterial = CaptureUseAreaMesh->CreateDynamicMaterialInstance(0, CurrentMaterial);
	}

	if (!DynamicMaterial)
	{
		return;
	}

	const FItemUseAreaVisualSettings& VisualSettings = CaptureUseAreaMesh->GetVisualSettings();
	DynamicMaterial->SetVectorParameterValue(BeeSwarmClusterNames::UseAreaColorParameter, VisualSettings.UseAreaColor);
	DynamicMaterial->SetScalarParameterValue(BeeSwarmClusterNames::UseAreaOpacityParameter, 0.0f);
	DynamicMaterial->SetScalarParameterValue(BeeSwarmClusterNames::PulseSpeedParameter, VisualSettings.PulseSpeed);
	DynamicMaterial->SetScalarParameterValue(BeeSwarmClusterNames::HoverStrengthParameter, 0.0f);
}

void ABeeSwarmClusterActor::SetCaptureUseAreaActive(bool bActive)
{
	if (!CaptureUseAreaMesh)
	{
		return;
	}

	CaptureUseAreaMesh->SetItemUseAreaEnabled(bActive);
	if (!bActive)
	{
		CaptureUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CaptureUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ApplyCaptureUseAreaVisualIdleState();
	}
}

void ABeeSwarmClusterActor::HandleCapturedIfNeeded()
{
	HandleBeesCapturedIfNeeded();
	HandleSwarmCapturedIfNeeded();
}

void ABeeSwarmClusterActor::HandleBeesCapturedIfNeeded()
{
	if (bBeesCaptured)
	{
		return;
	}

	const float TotalBeeAmount = GetTotalBeeAmount();
	const bool bNoBeesConfigured = TotalBeeAmount <= KINDA_SMALL_NUMBER;
	const bool bAllBeesCaptured = !bNoBeesConfigured
		&& (CapturedBeeAmount >= TotalBeeAmount || GetRemainingBeeAmount() <= KINDA_SMALL_NUMBER);
	if (!bNoBeesConfigured && !bAllBeesCaptured)
	{
		return;
	}

	bBeesCaptured = true;
	CapturedBeeAmount = bNoBeesConfigured ? 0.0f : TotalBeeAmount;
	const float OldAliveRadius = AliveRadius;
	AliveRadius = 0.0f;
	ApplyClusterNiagaraParameters();
	if (!FMath::IsNearlyEqual(OldAliveRadius, AliveRadius))
	{
		ReceiveAliveRadiusChanged(AliveRadius);
	}
	SetCaptureUseAreaActive(false);
	RebuildItemUseAreaDescriptors();
}

void ABeeSwarmClusterActor::HandleSwarmCapturedIfNeeded()
{
	if (bCaptured || !bBeesCaptured || !bQueenCaptured)
	{
		return;
	}

	bCaptured = true;
	AliveRadius = 0.0f;
	ApplyClusterNiagaraParameters();
	SetCaptureUseAreaActive(false);
	RebuildItemUseAreaDescriptors();
	ReceiveSwarmCaptured();
}
