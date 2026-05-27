#include "WorldActors/PlacedItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/PlacementOccupantComponent.h"
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
	ItemDefinition = PlacementOccupant ? PlacementOccupant->GetReturnItemDefinition() : (SourceItemInstance ? SourceItemInstance->GetDefinition() : nullptr);
	OwningPlacementSlotActor = PlacementOccupant ? PlacementOccupant->GetOwningPlacementSlotActor() : InOwningPlacementSlotActor;

	if (ItemMesh)
	{
		UItemDefinition* ReturnDefinition = GetItemDefinition();
		if (ReturnDefinition && ReturnDefinition->WorldMesh)
		{
			ItemMesh->SetStaticMesh(ReturnDefinition->WorldMesh);
		}
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
