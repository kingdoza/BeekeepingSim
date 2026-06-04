#include "WorldActors/QueenBeeActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AQueenBeeActor::AQueenBeeActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	QueenBeeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QueenBeeMesh"));
	QueenBeeMesh->SetupAttachment(Root);
}

void AQueenBeeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// Disease is now represented by ABeehive::DiseaseVfxNiagara.
	// ApplyDiseaseMaterialParameter();
}

void AQueenBeeActor::BeginPlay()
{
	Super::BeginPlay();
	// Disease is now represented by ABeehive::DiseaseVfxNiagara.
	// ApplyDiseaseMaterialParameter();
}

void AQueenBeeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float YawDelta = FMath::FRandRange(-YawJitterDegreesPerTick, YawJitterDegreesPerTick);
	AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AQueenBeeActor::SetDiseaseValue(float NewDiseaseValue)
{
	DiseaseValue = FMath::Clamp(NewDiseaseValue, 0.0f, 1.0f);
	// Disease is now represented by ABeehive::DiseaseVfxNiagara.
	// ApplyDiseaseMaterialParameter();
}

void AQueenBeeActor::EnsureDiseaseMaterialInstances()
{
	DiseaseMaterialInstances.Reset();
	if (!QueenBeeMesh)
	{
		return;
	}

	const int32 MaterialCount = QueenBeeMesh->GetNumMaterials();
	DiseaseMaterialInstances.SetNum(MaterialCount);
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInterface* CurrentMaterial = QueenBeeMesh->GetMaterial(MaterialIndex);
		if (!CurrentMaterial)
		{
			DiseaseMaterialInstances[MaterialIndex] = nullptr;
			continue;
		}

		if (UMaterialInstanceDynamic* ExistingDynamicMaterial = Cast<UMaterialInstanceDynamic>(CurrentMaterial))
		{
			DiseaseMaterialInstances[MaterialIndex] = ExistingDynamicMaterial;
			continue;
		}

		DiseaseMaterialInstances[MaterialIndex] = QueenBeeMesh->CreateDynamicMaterialInstance(MaterialIndex, CurrentMaterial);
	}
}

void AQueenBeeActor::ApplyDiseaseMaterialParameter()
{
	DiseaseValue = FMath::Clamp(DiseaseValue, 0.0f, 1.0f);
	EnsureDiseaseMaterialInstances();
	if (DiseaseMaterialParameterName.IsNone())
	{
		return;
	}

	for (TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance : DiseaseMaterialInstances)
	{
		if (MaterialInstance)
		{
			// MaterialInstance->SetScalarParameterValue(DiseaseMaterialParameterName, DiseaseValue);
		}
	}
}
