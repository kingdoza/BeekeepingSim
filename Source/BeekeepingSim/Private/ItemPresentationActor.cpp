#include "Public/ItemPresentationActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AItemPresentationActor::AItemPresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FallbackMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FallbackMesh"));
	FallbackMeshComponent->SetupAttachment(Root);
	FallbackMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FallbackMeshComponent->SetGenerateOverlapEvents(false);
	FallbackMeshComponent->SetOnlyOwnerSee(true);
	FallbackMeshComponent->CastShadow = false;
	FallbackMeshComponent->bVisibleInReflectionCaptures = false;
	FallbackMeshComponent->bVisibleInRayTracing = false;
	FallbackMeshComponent->SetIsReplicated(false);
}

void AItemPresentationActor::InitializePresentation_Implementation(ABeekeeperCharacter* InOwningCharacter, UItemInstance* InItemInstance)
{
	OwningCharacter = InOwningCharacter;
	ItemInstance = InItemInstance;
}

void AItemPresentationActor::SetPresentationHidden(bool bInHidden)
{
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetHiddenInGame(bInHidden);
		PrimitiveComponent->SetVisibility(!bInHidden, true);
	}

	SetActorHiddenInGame(bInHidden);
}

void AItemPresentationActor::DisablePresentationCollision()
{
	SetActorEnableCollision(false);

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
	}
}

void AItemPresentationActor::ApplyFirstPersonVisibilityPolicy(
	const bool bOnlyOwnerSee,
	const bool bCastShadow,
	const bool bVisibleInReflectionCaptures,
	const bool bVisibleInRayTracing)
{
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetOnlyOwnerSee(bOnlyOwnerSee);
		PrimitiveComponent->CastShadow = bCastShadow;
		PrimitiveComponent->bVisibleInReflectionCaptures = bVisibleInReflectionCaptures;
		PrimitiveComponent->bVisibleInRayTracing = bVisibleInRayTracing;
		PrimitiveComponent->SetIsReplicated(false);
	}
}

void AItemPresentationActor::SetFallbackStaticMesh(UStaticMesh* InStaticMesh)
{
	if (!FallbackMeshComponent)
	{
		return;
	}

	FallbackMeshComponent->SetStaticMesh(InStaticMesh);
	FallbackMeshComponent->SetVisibility(InStaticMesh != nullptr, true);
	FallbackMeshComponent->SetHiddenInGame(InStaticMesh == nullptr);
}
