#include "WorldActors/QueenBeeActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AQueenBeeActor::AQueenBeeActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	QueenBeeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QueenBeeMesh"));
	QueenBeeMesh->SetupAttachment(Root);
}

void AQueenBeeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float YawDelta = FMath::FRandRange(-YawJitterDegreesPerTick, YawJitterDegreesPerTick);
	AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}
