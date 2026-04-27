#include "Public/StorageBox.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Public/FocusTargetComponent.h"
#include "Public/StorageBoxComponent.h"
#include "Public/StorageBoxFocusActionComponent.h"

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
