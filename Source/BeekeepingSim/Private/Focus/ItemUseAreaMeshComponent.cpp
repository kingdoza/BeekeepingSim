#include "Focus/ItemUseAreaMeshComponent.h"

FName UItemUseAreaMeshComponent::GetResolvedAreaId() const
{
	return AreaId.IsNone() ? GetFName() : AreaId;
}

UObject* UItemUseAreaMeshComponent::ResolveEffectTargetObject(AActor* HostActor) const
{
	switch (EffectTargetPolicy)
	{
	case EItemUseAreaEffectTargetPolicy::ComponentOwner:
		return GetOwner();
	case EItemUseAreaEffectTargetPolicy::HostActor:
		return HostActor ? HostActor : GetOwner();
	case EItemUseAreaEffectTargetPolicy::ExplicitObject:
		return ExplicitEffectTargetObject ? ExplicitEffectTargetObject.Get() : GetOwner();
	default:
		return GetOwner();
	}
}

