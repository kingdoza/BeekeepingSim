#include "Focus/CursorPartFocusRegistrationComponent.h"

#include "Focus/CursorPartFocusProvider.h"
#include "Focus/CursorPartFocusScopeComponent.h"

void UCursorPartFocusRegistrationComponent::RebuildCursorPartFocusDescriptors()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot rebuild PartFocus descriptors because owner is null."), *GetName());
		}
		return;
	}

	UCursorPartFocusScopeComponent* ScopeComponent = OwnerActor->FindComponentByClass<UCursorPartFocusScopeComponent>();
	if (!ScopeComponent)
	{
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot rebuild PartFocus descriptors for %s because CursorPartFocusScopeComponent is missing."),
				*GetName(), *OwnerActor->GetName());
		}
		return;
	}

	if (bLogRegistrationDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("%s rebuilding PartFocus descriptors for %s."),
			*GetName(), *OwnerActor->GetName());
	}

	ScopeComponent->ClearRegisteredParts();
	GatherAndRegisterDescriptors(ScopeComponent);
}

void UCursorPartFocusRegistrationComponent::AppendCursorPartFocusDescriptorsToScope()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot append PartFocus descriptors because owner is null."), *GetName());
		}
		return;
	}

	UCursorPartFocusScopeComponent* ScopeComponent = OwnerActor->FindComponentByClass<UCursorPartFocusScopeComponent>();
	if (!ScopeComponent)
	{
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot append PartFocus descriptors for %s because CursorPartFocusScopeComponent is missing."),
				*GetName(), *OwnerActor->GetName());
		}
		return;
	}

	if (bLogRegistrationDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("%s appending PartFocus descriptors for %s."),
			*GetName(), *OwnerActor->GetName());
	}

	GatherAndRegisterDescriptors(ScopeComponent);
}

void UCursorPartFocusRegistrationComponent::GatherAndRegisterDescriptors(UCursorPartFocusScopeComponent* ScopeComponent) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !ScopeComponent)
	{
		return;
	}

	TArray<FCursorPartFocusPartDescriptor> CollectedDescriptors;
	if (OwnerActor->GetClass()->ImplementsInterface(UCursorPartFocusProvider::StaticClass()))
	{
		const int32 BeforeCount = CollectedDescriptors.Num();
		ICursorPartFocusProvider::Execute_GetCursorPartFocusDescriptors(OwnerActor, CollectedDescriptors);
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("%s collected %d PartFocus descriptors from owner actor %s."),
				*GetName(), CollectedDescriptors.Num() - BeforeCount, *OwnerActor->GetName());
		}
	}

	TInlineComponentArray<UActorComponent*> Components(OwnerActor);
	for (UActorComponent* Component : Components)
	{
		if (!Component || Component == this)
		{
			continue;
		}

		if (!Component->GetClass()->ImplementsInterface(UCursorPartFocusProvider::StaticClass()))
		{
			continue;
		}

		const int32 BeforeCount = CollectedDescriptors.Num();
		ICursorPartFocusProvider::Execute_GetCursorPartFocusDescriptors(Component, CollectedDescriptors);
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("%s collected %d PartFocus descriptors from provider component %s."),
				*GetName(), CollectedDescriptors.Num() - BeforeCount, *Component->GetName());
		}
	}

	if (bLogRegistrationDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("%s registering %d collected PartFocus descriptors."),
			*GetName(), CollectedDescriptors.Num());
	}

	for (const FCursorPartFocusPartDescriptor& Descriptor : CollectedDescriptors)
	{
		if (bLogRegistrationDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("%s registering PartFocus descriptor PartId=%s Owner=%s Hit=%s Action=%s"),
				*GetName(),
				*Descriptor.PartId.ToString(),
				Descriptor.OwnerActor ? *Descriptor.OwnerActor->GetName() : TEXT("<none>"),
				Descriptor.HitComponent ? *Descriptor.HitComponent->GetName() : TEXT("<none>"),
				Descriptor.ActionHandler ? *Descriptor.ActionHandler->GetName() : TEXT("<none>"));
		}
		ScopeComponent->RegisterPartDescriptor(Descriptor);
	}
}
