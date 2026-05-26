#include "Focus/ChildCursorPartFocusProviderComponent.h"

#include "Components/ChildActorComponent.h"

namespace
{
FString FormatComponentTagsForLog(const UActorComponent* Component)
{
	if (!Component || Component->ComponentTags.Num() == 0)
	{
		return TEXT("<none>");
	}

	TArray<FString> TagStrings;
	TagStrings.Reserve(Component->ComponentTags.Num());
	for (const FName& Tag : Component->ComponentTags)
	{
		TagStrings.Add(Tag.ToString());
	}

	return FString::Join(TagStrings, TEXT(", "));
}
}

void UChildCursorPartFocusProviderComponent::GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		if (bLogProviderDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot scan PartFocus children because owner is null."), *GetName());
		}
		return;
	}

	TArray<UChildActorComponent*> ChildActorComponents;
	OwnerActor->GetComponents<UChildActorComponent>(ChildActorComponents);
	if (bLogProviderDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("%s scanning %d child actor components on %s. RequiredTag=%s RequiredClass=%s"),
			*GetName(),
			ChildActorComponents.Num(),
			*OwnerActor->GetName(),
			*RequiredChildActorComponentTag.ToString(),
			RequiredChildActorClass ? *RequiredChildActorClass->GetName() : TEXT("<none>"));
	}

	const int32 InitialDescriptorCount = OutDescriptors.Num();
	for (UChildActorComponent* ChildActorComponent : ChildActorComponents)
	{
		if (!ChildActorComponent)
		{
			continue;
		}

		if (!RequiredChildActorComponentTag.IsNone() && !ChildActorComponent->ComponentHasTag(RequiredChildActorComponentTag))
		{
			if (bLogSkippedChildren || bLogProviderDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("%s skipped child actor component %s due to missing required tag %s. ComponentTags=[%s]"),
					*GetName(),
					*ChildActorComponent->GetName(),
					*RequiredChildActorComponentTag.ToString(),
					*FormatComponentTagsForLog(ChildActorComponent));
			}
			continue;
		}

		AActor* ChildActor = ChildActorComponent->GetChildActor();
		if (!ChildActor)
		{
			if (bLogSkippedChildren || bLogProviderDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("%s skipped child actor component %s due to null child actor."),
					*GetName(), *ChildActorComponent->GetName());
			}
			continue;
		}

		if (RequiredChildActorClass && !ChildActor->IsA(RequiredChildActorClass))
		{
			if (bLogSkippedChildren || bLogProviderDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("%s skipped child actor %s due to class filter. ActualClass=%s RequiredClass=%s"),
					*GetName(),
					*ChildActor->GetName(),
					*ChildActor->GetClass()->GetName(),
					*RequiredChildActorClass->GetName());
			}
			continue;
		}

		if (!ChildActor->GetClass()->ImplementsInterface(UCursorPartFocusProvider::StaticClass()))
		{
			if (bLogSkippedChildren || bLogProviderDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("%s skipped child actor %s because it does not implement ICursorPartFocusProvider. Class=%s"),
					*GetName(),
					*ChildActor->GetName(),
					*ChildActor->GetClass()->GetName());
			}
			continue;
		}

		TArray<FCursorPartFocusPartDescriptor> ChildDescriptors;
		ICursorPartFocusProvider::Execute_GetCursorPartFocusDescriptors(ChildActor, ChildDescriptors);
		if (bLogProviderDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("%s accepted child actor component %s -> child %s. DescriptorCount=%d"),
				*GetName(),
				*ChildActorComponent->GetName(),
				*ChildActor->GetName(),
				ChildDescriptors.Num());
		}
		OutDescriptors.Append(MoveTemp(ChildDescriptors));
	}

	if (bLogProviderDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("%s appended %d PartFocus descriptors."),
			*GetName(),
			OutDescriptors.Num() - InitialDescriptorCount);
	}
}
