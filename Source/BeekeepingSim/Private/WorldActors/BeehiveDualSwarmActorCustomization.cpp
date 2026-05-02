#include "WorldActors/BeehiveDualSwarmActorCustomization.h"

#if WITH_EDITOR

#include "DetailLayoutBuilder.h"
#include "NiagaraComponent.h"
#include "WorldActors/BeehiveDualSwarmActor.h"

TSharedRef<IDetailCustomization> FBeehiveDualSwarmActorCustomization::MakeInstance()
{
	return MakeShared<FBeehiveDualSwarmActorCustomization>();
}

void FBeehiveDualSwarmActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideProperty(FName(TEXT("OutgoingNiagara")));
	DetailBuilder.HideProperty(FName(TEXT("IngoingNiagara")));
}

TSharedRef<IDetailCustomization> FBeehiveDualSwarmNiagaraComponentCustomization::MakeInstance()
{
	return MakeShared<FBeehiveDualSwarmNiagaraComponentCustomization>();
}

void FBeehiveDualSwarmNiagaraComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	bool bIsBeehiveDualSwarmNiagara = false;
	for (const TWeakObjectPtr<UObject>& Object : CustomizedObjects)
	{
		const UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(Object.Get());
		if (!NiagaraComponent)
		{
			continue;
		}

		const AActor* OwnerActor = NiagaraComponent->GetOwner();
		if (OwnerActor && OwnerActor->IsA<ABeehiveDualSwarmActor>())
		{
			bIsBeehiveDualSwarmNiagara = true;
			break;
		}
	}

	if (!bIsBeehiveDualSwarmNiagara)
	{
		return;
	}

	// Hide user-parameter override editing while keeping asset binding workflow.
	DetailBuilder.HideProperty(FName(TEXT("OverrideParameters")));
}

#endif
