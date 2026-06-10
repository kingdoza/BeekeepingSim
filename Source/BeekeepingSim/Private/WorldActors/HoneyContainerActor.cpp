#include "WorldActors/HoneyContainerActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/HoneyContainerItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "WorldActors/HoneyContainerRetrievePartFocusActionComponent.h"
#include "WorldActors/HoneyNozzlePartFocusActionComponent.h"
#include "WorldActors/PlacementOccupantComponent.h"

AHoneyContainerActor::AHoneyContainerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
	ContainerMesh->SetupAttachment(Root);
	ContainerMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ContainerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ContainerMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	HoneyVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HoneyVisualMesh"));
	HoneyVisualMesh->SetupAttachment(ContainerMesh);
	HoneyVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NozzleHitComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NozzleHitComponent"));
	NozzleHitComponent->SetupAttachment(ContainerMesh);
	NozzleHitComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	NozzleHitComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	NozzleHitComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	NozzleOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("NozzleOrigin"));
	NozzleOrigin->SetupAttachment(NozzleHitComponent);

	PourTarget = CreateDefaultSubobject<USceneComponent>(TEXT("PourTarget"));
	PourTarget->SetupAttachment(ContainerMesh);

	HoneyStreamNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HoneyStreamNiagara"));
	HoneyStreamNiagara->SetupAttachment(NozzleOrigin);
	HoneyStreamNiagara->SetAutoActivate(false);
	HoneyStreamNiagara->SetVisibility(false, true);

	PlacementOccupant = CreateDefaultSubobject<UPlacementOccupantComponent>(TEXT("PlacementOccupant"));
	RetrieveAction = CreateDefaultSubobject<UHoneyContainerRetrievePartFocusActionComponent>(TEXT("RetrieveAction"));
	NozzleAction = CreateDefaultSubobject<UHoneyNozzlePartFocusActionComponent>(TEXT("NozzleAction"));
	if (RetrieveAction)
	{
		RetrieveAction->SetEngageMode(ECursorPartFocusEngageMode::PreviewOnly);
	}
}

void AHoneyContainerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	bHasCapturedHoneyVisualFullScale = false;
	CaptureHoneyVisualFullScaleIfNeeded();
}

void AHoneyContainerActor::BeginPlay()
{
	Super::BeginPlay();
	CaptureHoneyVisualFullScaleIfNeeded();
	if (const UHoneyContainerItemDefinition* HoneyContainerDefinition = ResolveHoneyContainerDefinition())
	{
		if (MaxVolumeMl <= 0.0f && CurrentVolumeMl <= 0.0f)
		{
			ApplyDefinitionDefaults(HoneyContainerDefinition);
		}
	}
	RefreshHoneyVisualState();
}

void AHoneyContainerActor::ApplyStateFromItemInstance(const UItemInstance* SourceItemInstance)
{
	const UHoneyContainerItemDefinition* HoneyContainerDefinition = SourceItemInstance
		? Cast<UHoneyContainerItemDefinition>(SourceItemInstance->GetDefinition())
		: ResolveHoneyContainerDefinition();
	if (!HoneyContainerDefinition)
	{
		MaxVolumeMl = 0.0f;
		CurrentVolumeMl = 0.0f;
		HoneyDensity = 0.0f;
		HoneyRipeness = 0.0f;
		RefreshHoneyVisualState();
		return;
	}

	MaxVolumeMl = FMath::Max(0.0f, HoneyContainerDefinition->MaxVolumeMl);
	if (SourceItemInstance && SourceItemInstance->HasHoneyContainerState())
	{
		const FHoneyContainerItemState ContainerState = SourceItemInstance->GetHoneyContainerState();
		CurrentVolumeMl = ContainerState.CurrentVolumeMl;
		HoneyDensity = ContainerState.HoneyDensity;
		HoneyRipeness = ContainerState.HoneyRipeness;
	}
	else
	{
		ApplyDefinitionDefaults(HoneyContainerDefinition);
	}

	SanitizeHoneyState();
	RefreshHoneyVisualState();
}

void AHoneyContainerActor::WriteHoneyContainerStateToItemInstance(UItemInstance* TargetItemInstance) const
{
	if (!TargetItemInstance)
	{
		return;
	}

	TargetItemInstance->SetHoneyContainerState(CurrentVolumeMl, HoneyDensity, HoneyRipeness);
}

float AHoneyContainerActor::GetFreeVolumeMl() const
{
	return FMath::Max(0.0f, MaxVolumeMl - CurrentVolumeMl);
}

float AHoneyContainerActor::GetFillRatio() const
{
	if (MaxVolumeMl <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentVolumeMl / MaxVolumeMl, 0.0f, 1.0f);
}

void AHoneyContainerActor::RemoveHoneyVolume(float VolumeMl)
{
	const float VolumeToRemove = FMath::Clamp(VolumeMl, 0.0f, CurrentVolumeMl);
	if (VolumeToRemove <= 0.0f)
	{
		return;
	}

	CurrentVolumeMl -= VolumeToRemove;
	SanitizeHoneyState();
	RefreshHoneyVisualState();
}

void AHoneyContainerActor::AddHoneyVolume(float VolumeMl, float IncomingDensity, float IncomingRipeness)
{
	const float VolumeToAdd = FMath::Clamp(VolumeMl, 0.0f, GetFreeVolumeMl());
	if (VolumeToAdd <= 0.0f)
	{
		return;
	}

	const float ExistingVolume = CurrentVolumeMl;
	const float NewTotalVolume = ExistingVolume + VolumeToAdd;
	const float SanitizedIncomingDensity = FMath::Clamp(IncomingDensity, 0.0f, 1.0f);
	const float SanitizedIncomingRipeness = NormalizeHoneyRipeness(SanitizedIncomingDensity, IncomingRipeness);

	if (ExistingVolume <= KINDA_SMALL_NUMBER)
	{
		HoneyDensity = SanitizedIncomingDensity;
		HoneyRipeness = SanitizedIncomingRipeness;
	}
	else
	{
		HoneyDensity = ((HoneyDensity * ExistingVolume) + (SanitizedIncomingDensity * VolumeToAdd)) / NewTotalVolume;
		HoneyRipeness = ((HoneyRipeness * ExistingVolume) + (SanitizedIncomingRipeness * VolumeToAdd)) / NewTotalVolume;
	}

	CurrentVolumeMl = NewTotalVolume;
	SanitizeHoneyState();
	RefreshHoneyVisualState();
}

void AHoneyContainerActor::RefreshHoneyVisualState()
{
	CaptureHoneyVisualFullScaleIfNeeded();
	EnsureHoneyVisualMaterialInstance();

	const float FillRatio = GetFillRatio();
	if (HoneyVisualMesh)
	{
		FVector NewScale = HoneyVisualFullRelativeScale;
		NewScale.Z = HoneyVisualFullRelativeScale.Z * FillRatio;
		HoneyVisualMesh->SetRelativeScale3D(NewScale);
	}

	const float VisualRipeness = NormalizeHoneyRipeness(HoneyDensity, HoneyRipeness);
	if (HoneyVisualMaterialInstance)
	{
		HoneyVisualMaterialInstance->SetScalarParameterValue(HoneyDensityMaterialParameterName, FMath::Clamp(HoneyDensity, 0.0f, 1.0f));
		HoneyVisualMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, VisualRipeness);
	}
}

void AHoneyContainerActor::ApplyDefinitionDefaults(const UHoneyContainerItemDefinition* HoneyContainerDefinition)
{
	if (!HoneyContainerDefinition)
	{
		return;
	}

	MaxVolumeMl = FMath::Max(0.0f, HoneyContainerDefinition->MaxVolumeMl);
	CurrentVolumeMl = FMath::Clamp(HoneyContainerDefinition->DefaultCurrentVolumeMl, 0.0f, MaxVolumeMl);
	HoneyDensity = FMath::Clamp(HoneyContainerDefinition->DefaultHoneyDensity, 0.0f, 1.0f);
	HoneyRipeness = NormalizeHoneyRipeness(HoneyDensity, HoneyContainerDefinition->DefaultHoneyRipeness);
}

void AHoneyContainerActor::SanitizeHoneyState()
{
	MaxVolumeMl = FMath::Max(0.0f, MaxVolumeMl);
	CurrentVolumeMl = FMath::Clamp(CurrentVolumeMl, 0.0f, MaxVolumeMl);
	HoneyDensity = FMath::Clamp(HoneyDensity, 0.0f, 1.0f);
	HoneyRipeness = NormalizeHoneyRipeness(HoneyDensity, HoneyRipeness);
}

void AHoneyContainerActor::CaptureHoneyVisualFullScaleIfNeeded()
{
	if (bHasCapturedHoneyVisualFullScale || !HoneyVisualMesh)
	{
		return;
	}

	HoneyVisualFullRelativeScale = HoneyVisualMesh->GetRelativeScale3D();
	bHasCapturedHoneyVisualFullScale = true;
}

void AHoneyContainerActor::EnsureHoneyVisualMaterialInstance()
{
	if (!HoneyVisualMesh)
	{
		HoneyVisualMaterialInstance = nullptr;
		return;
	}

	UMaterialInterface* CurrentMaterial = HoneyVisualMesh->GetMaterial(0);
	if (!CurrentMaterial)
	{
		HoneyVisualMaterialInstance = nullptr;
		return;
	}

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		HoneyVisualMaterialInstance = Cast<UMaterialInstanceDynamic>(CurrentMaterial);
		return;
	}

	if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
	{
		HoneyVisualMaterialInstance = ExistingMID;
		return;
	}

	if (!HoneyVisualMaterialInstance)
	{
		HoneyVisualMaterialInstance = HoneyVisualMesh->CreateDynamicMaterialInstance(0, CurrentMaterial);
	}
}

UItemDefinition* AHoneyContainerActor::ResolveReturnItemDefinition() const
{
	return PlacementOccupant ? PlacementOccupant->GetReturnItemDefinition() : nullptr;
}

const UHoneyContainerItemDefinition* AHoneyContainerActor::ResolveHoneyContainerDefinition() const
{
	return Cast<UHoneyContainerItemDefinition>(ResolveReturnItemDefinition());
}

float AHoneyContainerActor::NormalizeHoneyRipeness(float Density, float Ripeness) const
{
	const float SanitizedDensity = FMath::Clamp(Density, 0.0f, 1.0f);
	if (SanitizedDensity < 1.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(Ripeness, 0.0f, 1.0f);
}
