#include "Public/WorldItemPickup.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Public/FocusTargetComponent.h"
#include "Public/ItemDefinition.h"
#include "Public/PickupFocusActionComponent.h"

AWorldItemPickup::AWorldItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(Root);

	FocusTarget = CreateDefaultSubobject<UFocusTargetComponent>(TEXT("FocusTarget"));
	FocusAction = CreateDefaultSubobject<UPickupFocusActionComponent>(TEXT("FocusAction"));
}

void AWorldItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshPickupPresentation();
}

bool AWorldItemPickup::IsPickupValid() const
{
	return ItemDefinition != nullptr;
}

void AWorldItemPickup::ConsumePickup()
{
	Destroy();
}

void AWorldItemPickup::RefreshPickupPresentation()
{
	if (PickupMesh)
	{
		PickupMesh->SetStaticMesh(ItemDefinition ? ItemDefinition->WorldMesh : nullptr);
	}

	if (FocusTarget)
	{
		FocusTarget->SetDisplayName(ItemDefinition ? ItemDefinition->DisplayName : FText::GetEmpty());
	}
}
