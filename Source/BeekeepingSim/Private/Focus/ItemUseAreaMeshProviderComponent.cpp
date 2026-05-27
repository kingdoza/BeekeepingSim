#include "Focus/ItemUseAreaMeshProviderComponent.h"

#include "Components/ChildActorComponent.h"
#include "Focus/ItemUseAreaActivationProvider.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "Focus/ItemUseAreaMeshSource.h"

void UItemUseAreaMeshProviderComponent::BuildItemUseAreaDescriptors(TArray<FItemUseAreaDescriptor>& OutDescriptors) const
{
	AActor* HostActor = GetOwner();
	if (!HostActor)
	{
		return;
	}

	auto BuildDescriptorFromComponent = [&](UItemUseAreaMeshComponent* ItemUseAreaMesh)
	{
		if (!ItemUseAreaMesh || !ItemUseAreaMesh->IsItemUseAreaEnabled())
		{
			return;
		}

		AActor* ComponentOwner = ItemUseAreaMesh->GetOwner();
		bool bActive = true;
		if (ComponentOwner && ComponentOwner->GetClass()->ImplementsInterface(UItemUseAreaActivationProvider::StaticClass()))
		{
			bActive = IItemUseAreaActivationProvider::Execute_IsItemUseAreaMeshActive(ComponentOwner, ItemUseAreaMesh, HostActor);
		}

		FItemUseAreaDescriptor Descriptor;
		Descriptor.AreaId = ItemUseAreaMesh->GetResolvedAreaId();
		Descriptor.OwnerActor = ComponentOwner;
		Descriptor.HitComponent = ItemUseAreaMesh;
		Descriptor.VisualComponents.Add(ItemUseAreaMesh);
		Descriptor.VisualSettings = ItemUseAreaMesh->GetVisualSettings();
		Descriptor.EffectTargetObject = ItemUseAreaMesh->ResolveEffectTargetObject(HostActor);
		Descriptor.AreaTags = bActive ? ItemUseAreaMesh->GetAreaTags() : FGameplayTagContainer();
		OutDescriptors.Add(MoveTemp(Descriptor));
	};

	if (bIncludeOwnerComponents)
	{
		TInlineComponentArray<UItemUseAreaMeshComponent*> OwnerUseAreaMeshes(HostActor);
		for (UItemUseAreaMeshComponent* ItemUseAreaMesh : OwnerUseAreaMeshes)
		{
			BuildDescriptorFromComponent(ItemUseAreaMesh);
		}
	}

	if (!bIncludeDirectChildActors)
	{
		return;
	}

	TInlineComponentArray<UChildActorComponent*> ChildActorComponents(HostActor);
	for (UChildActorComponent* ChildActorComponent : ChildActorComponents)
	{
		if (!ChildActorComponent)
		{
			continue;
		}

		if (!RequiredChildActorComponentTag.IsNone() && !ChildActorComponent->ComponentHasTag(RequiredChildActorComponentTag))
		{
			if (bLogCollectionDebug)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s skipped child actor component %s due to missing required tag %s."),
					*GetName(), *ChildActorComponent->GetName(), *RequiredChildActorComponentTag.ToString());
			}
			continue;
		}

		AActor* ChildActor = ChildActorComponent->GetChildActor();
		if (!ChildActor)
		{
			if (bLogCollectionDebug)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s skipped child actor component %s due to null child actor."),
					*GetName(), *ChildActorComponent->GetName());
			}
			continue;
		}

		TInlineComponentArray<UItemUseAreaMeshComponent*> ChildUseAreaMeshes(ChildActor);
		for (UItemUseAreaMeshComponent* ItemUseAreaMesh : ChildUseAreaMeshes)
		{
			BuildDescriptorFromComponent(ItemUseAreaMesh);
		}

		if (const IItemUseAreaMeshSource* MeshSource = Cast<IItemUseAreaMeshSource>(ChildActor))
		{
			TArray<UItemUseAreaMeshComponent*> ProvidedUseAreaMeshes;
			MeshSource->GetProvidedItemUseAreaMeshes(ProvidedUseAreaMeshes);
			for (UItemUseAreaMeshComponent* ItemUseAreaMesh : ProvidedUseAreaMeshes)
			{
				BuildDescriptorFromComponent(ItemUseAreaMesh);
			}
		}
	}
}
