#include "WorldActors/PollenPattyActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

APollenPattyActor::APollenPattyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PattyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PattyMesh"));
	PattyMesh->SetupAttachment(Root);
}
