#include "WorldActors/PlacedItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
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

	RetrieveAction = CreateDefaultSubobject<UPlacedItemRetrievePartFocusActionComponent>(TEXT("RetrieveAction"));
	if (RetrieveAction)
	{
		RetrieveAction->SetEngageMode(ECursorPartFocusEngageMode::PreviewOnly);
	}
}

void APlacedItemActor::InitializePlacedItem(UItemInstance* SourceItemInstance, AActor* InOwningPlacementSlotActor)
{
	ItemDefinition = SourceItemInstance ? SourceItemInstance->GetDefinition() : nullptr;
	OwningPlacementSlotActor = InOwningPlacementSlotActor;

	if (ItemMesh)
	{
		if (ItemDefinition && ItemDefinition->WorldMesh)
		{
			ItemMesh->SetStaticMesh(ItemDefinition->WorldMesh);
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

FText APlacedItemActor::GetPlacedItemDisplayName() const
{
	return ItemDefinition ? ItemDefinition->DisplayName : FText::GetEmpty();
}
