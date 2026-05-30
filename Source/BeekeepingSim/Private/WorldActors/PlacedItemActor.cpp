#include "WorldActors/PlacedItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/PlacementOccupantComponent.h"
#include "WorldActors/PlacedItemRemainingComponent.h"
#include "WorldActors/PlacedItemRemainingVisualComponent.h"
#include "WorldActors/PlacedItemRetrievePartFocusActionComponent.h"

APlacedItemActor::APlacedItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(Root);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	PlacementOccupant = CreateDefaultSubobject<UPlacementOccupantComponent>(TEXT("PlacementOccupant"));
	RetrieveAction = CreateDefaultSubobject<UPlacedItemRetrievePartFocusActionComponent>(TEXT("RetrieveAction"));
	RemainingComponent = CreateDefaultSubobject<UPlacedItemRemainingComponent>(TEXT("RemainingComponent"));
	if (RetrieveAction)
	{
		RetrieveAction->SetEngageMode(ECursorPartFocusEngageMode::PreviewOnly);
	}
}

void APlacedItemActor::InitializePlacedItem(UItemInstance* SourceItemInstance, AActor* InOwningPlacementSlotActor)
{
	if (PlacementOccupant)
	{
		PlacementOccupant->InitializeFromPlacement(SourceItemInstance, InOwningPlacementSlotActor);
	}
	UItemDefinition* ReturnDefinition = PlacementOccupant ? PlacementOccupant->GetReturnItemDefinition() : (SourceItemInstance ? SourceItemInstance->GetDefinition() : nullptr);
	ItemDefinition = ReturnDefinition;
	OwningPlacementSlotActor = PlacementOccupant ? PlacementOccupant->GetOwningPlacementSlotActor() : InOwningPlacementSlotActor;

	if (ItemMesh)
	{
		if (ReturnDefinition && ReturnDefinition->WorldMesh)
		{
			ItemMesh->SetStaticMesh(ReturnDefinition->WorldMesh);
		}
	}

	InitializeRemainingVisualComponent(ReturnDefinition);
	if (RemainingComponent)
	{
		RemainingComponent->InitializeFromPlacement(SourceItemInstance, ReturnDefinition, OwningPlacementSlotActor);
	}
	if (RuntimeRemainingVisualComponent)
	{
		RuntimeRemainingVisualComponent->ApplyRemainingRatio(RemainingComponent ? RemainingComponent->GetRemainingRatio() : 0.0f);
	}

	ReceivePlacedItemInitialized(SourceItemInstance);
}

UPrimitiveComponent* APlacedItemActor::GetPartFocusHitComponent() const
{
	return ItemMesh;
}

UCursorPartFocusActionComponent* APlacedItemActor::GetPartFocusActionComponent() const
{
	return RetrieveAction;
}

UItemDefinition* APlacedItemActor::GetItemDefinition() const
{
	if (PlacementOccupant)
	{
		if (UItemDefinition* ReturnDefinition = PlacementOccupant->GetReturnItemDefinition())
		{
			return ReturnDefinition;
		}
	}

	return ItemDefinition;
}

AActor* APlacedItemActor::GetOwningPlacementSlotActor() const
{
	if (PlacementOccupant)
	{
		if (AActor* SlotActor = PlacementOccupant->GetOwningPlacementSlotActor())
		{
			return SlotActor;
		}
	}

	return OwningPlacementSlotActor;
}

FText APlacedItemActor::GetPlacedItemDisplayName() const
{
	return GetItemDefinition() ? GetItemDefinition()->DisplayName : FText::GetEmpty();
}

void APlacedItemActor::InitializeRemainingVisualComponent(UItemDefinition* ReturnItemDefinition)
{
	if (RuntimeRemainingVisualComponent)
	{
		RuntimeRemainingVisualComponent->DestroyComponent();
		RuntimeRemainingVisualComponent = nullptr;
	}

	if (!ReturnItemDefinition)
	{
		return;
	}

	if (!ReturnItemDefinition->PlacedRemainingSpec.bUseDurabilityAsPlacedRemaining)
	{
		return;
	}

	UClass* VisualComponentClass = ReturnItemDefinition->PlacedRemainingSpec.VisualComponentClass.Get();
	if (!VisualComponentClass)
	{
		return;
	}

	if (!VisualComponentClass->IsChildOf(UPlacedItemRemainingVisualComponent::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has invalid placed remaining visual class '%s'. Expected UPlacedItemRemainingVisualComponent subclass."), *GetNameSafe(ReturnItemDefinition), *GetNameSafe(VisualComponentClass));
		return;
	}

	RuntimeRemainingVisualComponent = NewObject<UPlacedItemRemainingVisualComponent>(this, VisualComponentClass, TEXT("RuntimeRemainingVisualComponent"));
	if (!RuntimeRemainingVisualComponent)
	{
		return;
	}

	AddInstanceComponent(RuntimeRemainingVisualComponent);
	RuntimeRemainingVisualComponent->RegisterComponent();
	RuntimeRemainingVisualComponent->InitializeRemainingVisual(this, RemainingComponent);
}
