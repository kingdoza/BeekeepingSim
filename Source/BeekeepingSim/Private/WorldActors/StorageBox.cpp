#include "WorldActors/StorageBox.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Inventory/StorageBoxComponent.h"
#include "Interaction/StorageBoxFocusActionComponent.h"

AStorageBox::AStorageBox()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	BoxMesh->SetupAttachment(Root);

	FocusTarget = CreateDefaultSubobject<UFocusTargetComponent>(TEXT("FocusTarget"));
	StorageBoxComponent = CreateDefaultSubobject<UStorageBoxComponent>(TEXT("StorageBoxComponent"));
	StorageBoxFocusAction = CreateDefaultSubobject<UStorageBoxFocusActionComponent>(TEXT("StorageBoxFocusAction"));
}
