#include "Focus/ChildItemUseAreaProviderComponent.h"

#include "Components/ChildActorComponent.h"

void UChildItemUseAreaProviderComponent::GetItemUseAreaDescriptors_Implementation(TArray<FItemUseAreaDescriptor>& OutDescriptors) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	TArray<UChildActorComponent*> ChildActorComponents;
	OwnerActor->GetComponents<UChildActorComponent>(ChildActorComponents);

	for (UChildActorComponent* ChildActorComponent : ChildActorComponents)
	{
		if (!ChildActorComponent)
		{
			continue;
		}

		if (!RequiredChildActorComponentTag.IsNone() && !ChildActorComponent->ComponentHasTag(RequiredChildActorComponentTag))
		{
			if (bLogSkippedChildren)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s skipped child actor component %s due to missing required tag %s."),
					*GetName(), *ChildActorComponent->GetName(), *RequiredChildActorComponentTag.ToString());
			}
			continue;
		}

		AActor* ChildActor = ChildActorComponent->GetChildActor();
		if (!ChildActor)
		{
			if (bLogSkippedChildren)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s skipped child actor component %s due to null child actor."),
					*GetName(), *ChildActorComponent->GetName());
			}
			continue;
		}

		if (RequiredChildActorClass && !ChildActor->IsA(RequiredChildActorClass))
		{
			if (bLogSkippedChildren)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s skipped child actor %s due to class filter."),
					*GetName(), *ChildActor->GetName());
			}
			continue;
		}

		if (!ChildActor->GetClass()->ImplementsInterface(UItemUseAreaProvider::StaticClass()))
		{
			if (bLogSkippedChildren)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s skipped child actor %s because it does not implement IItemUseAreaProvider."),
					*GetName(), *ChildActor->GetName());
			}
			continue;
		}

		TArray<FItemUseAreaDescriptor> ChildDescriptors;
		IItemUseAreaProvider::Execute_GetItemUseAreaDescriptors(ChildActor, ChildDescriptors);
		OutDescriptors.Append(MoveTemp(ChildDescriptors));
	}
}
