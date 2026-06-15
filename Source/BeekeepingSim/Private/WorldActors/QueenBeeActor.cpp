#include "WorldActors/QueenBeeActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "GameplayTagContainer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace QueenBeeUseAreaNames
{
	static const FName QueenCageUseAreaTag(TEXT("Item.UseArea.QueenBee.QueenCage"));
	static const FName UseAreaColorParameter(TEXT("UseAreaColor"));
	static const FName UseAreaOpacityParameter(TEXT("UseAreaOpacity"));
	static const FName PulseSpeedParameter(TEXT("PulseSpeed"));
	static const FName HoverStrengthParameter(TEXT("HoverStrength"));
}

AQueenBeeActor::AQueenBeeActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	QueenBeeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QueenBeeMesh"));
	QueenBeeMesh->SetupAttachment(Root);

	QueenCageUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("QueenCageUseAreaMesh"));
	QueenCageUseAreaMesh->SetupAttachment(QueenBeeMesh);
	QueenCageUseAreaMesh->SetAreaId(TEXT("QueenBee.QueenCage"));
	QueenCageUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
	QueenCageUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	QueenCageUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	const FGameplayTag QueenCageUseAreaTag = FGameplayTag::RequestGameplayTag(QueenBeeUseAreaNames::QueenCageUseAreaTag, false);
	if (QueenCageUseAreaTag.IsValid())
	{
		FGameplayTagContainer AreaTags;
		AreaTags.AddTag(QueenCageUseAreaTag);
		QueenCageUseAreaMesh->SetAreaTags(AreaTags);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s could not resolve gameplay tag %s."),
			*GetName(),
			*QueenBeeUseAreaNames::QueenCageUseAreaTag.ToString());
	}
}

void AQueenBeeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetCaptured(bCaptured);
	ApplyQueenCageUseAreaVisualIdleState();
	// Disease is now represented by ABeehive::DiseaseVfxNiagara.
	// ApplyDiseaseMaterialParameter();
}

void AQueenBeeActor::BeginPlay()
{
	Super::BeginPlay();
	SetCaptured(bCaptured);
	ApplyQueenCageUseAreaVisualIdleState();
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

void AQueenBeeActor::SetCaptured(bool bNewCaptured)
{
	bCaptured = bNewCaptured;
	if (!QueenCageUseAreaMesh)
	{
		return;
	}

	QueenCageUseAreaMesh->SetItemUseAreaEnabled(!bCaptured);
	if (bCaptured)
	{
		QueenCageUseAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		QueenCageUseAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	ApplyQueenCageUseAreaVisualIdleState();
}

FQueenCageItemState AQueenBeeActor::MakeQueenCageItemState() const
{
	FQueenCageItemState State;
	State.bHasState = true;
	State.bHasQueen = true;
	State.CapturedQueenBeeClass = GetClass();
	State.BaseEggLayingPower = FMath::Max(0.0f, BaseEggLayingPower);
	State.DiseaseValue = FMath::Clamp(DiseaseValue, 0.0f, 1.0f);
	return State;
}

bool AQueenBeeActor::IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const
{
	(void)HostActor;
	return Component != QueenCageUseAreaMesh || !IsCaptured();
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

void AQueenBeeActor::ApplyQueenCageUseAreaVisualIdleState()
{
	if (!QueenCageUseAreaMesh)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	UMaterialInterface* CurrentMaterial = QueenCageUseAreaMesh->GetMaterial(0);
	if (!CurrentMaterial)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(CurrentMaterial);
	if (!DynamicMaterial)
	{
		DynamicMaterial = QueenCageUseAreaMesh->CreateDynamicMaterialInstance(0, CurrentMaterial);
	}

	if (!DynamicMaterial)
	{
		return;
	}

	const FItemUseAreaVisualSettings& VisualSettings = QueenCageUseAreaMesh->GetVisualSettings();
	DynamicMaterial->SetVectorParameterValue(QueenBeeUseAreaNames::UseAreaColorParameter, VisualSettings.UseAreaColor);
	DynamicMaterial->SetScalarParameterValue(QueenBeeUseAreaNames::UseAreaOpacityParameter, 0.0f);
	DynamicMaterial->SetScalarParameterValue(QueenBeeUseAreaNames::PulseSpeedParameter, VisualSettings.PulseSpeed);
	DynamicMaterial->SetScalarParameterValue(QueenBeeUseAreaNames::HoverStrengthParameter, 0.0f);
}
