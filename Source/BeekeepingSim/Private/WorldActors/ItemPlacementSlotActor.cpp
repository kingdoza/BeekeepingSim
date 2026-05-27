#include "WorldActors/ItemPlacementSlotActor.h"

#include "Character/BeekeeperCharacter.h"
#include "Components/SceneComponent.h"
#include "Focus/CursorItemUseAreaScopeComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/PlacedItemActor.h"

AItemPlacementSlotActor::AItemPlacementSlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SlotMeshComponent = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("SlotMeshComponent"));
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

void AItemPlacementSlotActor::GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const
{
	const_cast<AItemPlacementSlotActor*>(this)->ApplySlotAuthoringSettings();
	const_cast<AItemPlacementSlotActor*>(this)->RefreshSlotVisualState();

	if (!SanitizeAndCheckOccupied())
	{
		return;
	}

	APlacedItemActor* PlacedItemActor = Cast<APlacedItemActor>(PlacedActor);
	if (!PlacedItemActor)
	{
		return;
	}

	UPrimitiveComponent* HitComponent = PlacedItemActor->GetPartFocusHitComponent();
	if (!HitComponent)
	{
		return;
	}

	FCursorPartFocusPartDescriptor Descriptor;
	Descriptor.PartId = FName(*FString::Printf(TEXT("PlacedItem.%s"), *GetName()));
	Descriptor.OwnerActor = PlacedItemActor;
	Descriptor.HitComponent = HitComponent;
	Descriptor.OutlineComponents.Add(HitComponent);
	Descriptor.ActionHandler = PlacedItemActor->GetPartFocusActionComponent();
	Descriptor.EngageMode = Descriptor.ActionHandler ? Descriptor.ActionHandler->GetEngageMode() : ECursorPartFocusEngageMode::PreviewOnly;
	Descriptor.PromptData.bIsValid = true;
	Descriptor.PromptData.DisplayName = PlacedItemActor->GetPlacedItemDisplayName();
	Descriptor.PromptData.InteractionKeyText = FText::FromString(TEXT("RMB"));
	OutDescriptors.Add(MoveTemp(Descriptor));
}

bool AItemPlacementSlotActor::IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const
{
	(void)HostActor;

	if (Component != SlotMeshComponent)
	{
		return true;
	}

	return !SanitizeAndCheckOccupied();
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
	if (APlacedItemActor* PlacedItemActor = Cast<APlacedItemActor>(SpawnedActor))
	{
		PlacedItemActor->InitializePlacedItem(SourceItemInstance, this);
	}

	RefreshSlotVisualState();
	RequestHostPartFocusRebuild();
	RequestHostItemUseAreaRebuild();
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
	RequestHostPartFocusRebuild();
	RequestHostItemUseAreaRebuild();
}

void AItemPlacementSlotActor::RequestHostPartFocusRebuild() const
{
	if (AActor* HostActor = GetAttachParentActor())
	{
		static const FName RebuildFuncName(TEXT("RebuildCursorPartFocusDescriptors"));
		if (UFunction* RebuildFunction = HostActor->FindFunction(RebuildFuncName))
		{
			HostActor->ProcessEvent(RebuildFunction, nullptr);
		}
	}
}

void AItemPlacementSlotActor::RequestHostItemUseAreaRebuild() const
{
	if (AActor* HostActor = GetAttachParentActor())
	{
		if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = HostActor->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
		{
			ItemUseAreaScope->RebuildItemUseAreaDescriptors();
		}
	}
}

void AItemPlacementSlotActor::ApplySlotAuthoringSettings()
{
	if (SlotMeshComponent)
	{
		SlotMeshComponent->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner);
		if (SlotMeshComponent->GetConfiguredAreaId().IsNone() && !AreaId.IsNone())
		{
			SlotMeshComponent->SetAreaId(AreaId);
		}
		if (SlotMeshComponent->GetAreaTags().IsEmpty() && !AreaTags.IsEmpty())
		{
			SlotMeshComponent->SetAreaTags(AreaTags);
		}

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
