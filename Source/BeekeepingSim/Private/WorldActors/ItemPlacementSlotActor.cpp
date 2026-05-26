#include "WorldActors/ItemPlacementSlotActor.h"

#include "Character/BeekeeperCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/ItemInstance.h"

AItemPlacementSlotActor::AItemPlacementSlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SlotMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMeshComponent"));
	SlotMeshComponent->SetupAttachment(Root);
	SlotMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SlotMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SlotMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AttachComponent"));
	AttachComponent->SetupAttachment(Root);
}

void AItemPlacementSlotActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplySlotAuthoringSettings();
	RefreshSlotVisualState();
}

void AItemPlacementSlotActor::BeginPlay()
{
	Super::BeginPlay();
	ApplySlotAuthoringSettings();
	RefreshSlotVisualState();
}

void AItemPlacementSlotActor::GetItemUseAreaDescriptors_Implementation(TArray<FItemUseAreaDescriptor>& OutDescriptors) const
{
	const_cast<AItemPlacementSlotActor*>(this)->ApplySlotAuthoringSettings();
	const_cast<AItemPlacementSlotActor*>(this)->RefreshSlotVisualState();

	if (SanitizeAndCheckOccupied() || AreaId.IsNone() || !SlotMeshComponent || !SlotMeshComponent->GetStaticMesh())
	{
		return;
	}

	FItemUseAreaDescriptor Descriptor;
	Descriptor.AreaId = AreaId;
	Descriptor.AreaTags = AreaTags;
	Descriptor.OwnerActor = const_cast<AItemPlacementSlotActor*>(this);
	Descriptor.HitComponent = SlotMeshComponent;
	Descriptor.EffectTargetObject = const_cast<AItemPlacementSlotActor*>(this);
	Descriptor.VisualComponents.Add(SlotMeshComponent);

	OutDescriptors.Add(MoveTemp(Descriptor));
}

bool AItemPlacementSlotActor::TryPlaceItem_Implementation(TSubclassOf<AActor> PlacedActorClass, UItemInstance* SourceItemInstance, ABeekeeperCharacter* InteractingCharacter)
{
	(void)SourceItemInstance;
	(void)InteractingCharacter;

	if (SanitizeAndCheckOccupied() || !PlacedActorClass || !AttachComponent)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(PlacedActorClass, AttachComponent->GetComponentTransform(), SpawnParams);
	if (!SpawnedActor)
	{
		return false;
	}

	const bool bAttached = SpawnedActor->AttachToComponent(AttachComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);
	if (!bAttached)
	{
		SpawnedActor->Destroy();
		return false;
	}

	PlacedActor = SpawnedActor;
	RefreshSlotVisualState();
	return true;
}

bool AItemPlacementSlotActor::IsPlacementOccupied_Implementation() const
{
	return SanitizeAndCheckOccupied();
}

void AItemPlacementSlotActor::ClearPlacedItem_Implementation()
{
	if (AActor* ExistingPlacedActor = PlacedActor.Get())
	{
		ExistingPlacedActor->Destroy();
	}

	PlacedActor = nullptr;
	RefreshSlotVisualState();
}

void AItemPlacementSlotActor::ApplySlotAuthoringSettings()
{
	if (SlotMeshComponent)
	{
		if (SlotMeshAsset)
		{
			SlotMeshComponent->SetStaticMesh(SlotMeshAsset);
		}
		else
		{
			SlotMeshComponent->SetStaticMesh(ResolveClassDefaultSlotMesh());
		}

		if (SlotMeshMaterial)
		{
			SlotMeshComponent->SetMaterial(0, SlotMeshMaterial);
		}

		SlotMeshComponent->SetRelativeTransform(SlotMeshRelativeTransform);
	}

	if (AttachComponent)
	{
		AttachComponent->SetRelativeTransform(AttachRelativeTransform);
	}
}

bool AItemPlacementSlotActor::SanitizeAndCheckOccupied() const
{
	if (!IsValid(PlacedActor))
	{
		PlacedActor = nullptr;
		return false;
	}

	return true;
}

UStaticMesh* AItemPlacementSlotActor::ResolveClassDefaultSlotMesh() const
{
	const AItemPlacementSlotActor* ClassDefault = GetClass() ? GetClass()->GetDefaultObject<AItemPlacementSlotActor>() : nullptr;
	return (ClassDefault && ClassDefault->SlotMeshComponent) ? ClassDefault->SlotMeshComponent->GetStaticMesh() : nullptr;
}

void AItemPlacementSlotActor::RefreshSlotVisualState()
{
	if (!SlotMeshComponent)
	{
		return;
	}

	const bool bOccupied = SanitizeAndCheckOccupied();
	SlotMeshComponent->SetHiddenInGame(bOccupied);
	SlotMeshComponent->SetVisibility(!bOccupied, true);

	if (bOccupied)
	{
		SlotMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		SlotMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SlotMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		SlotMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}
