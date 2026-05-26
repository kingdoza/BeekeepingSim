#include "Inventory/VfxItemPresentationActor.h"

#include "NiagaraComponent.h"

AVfxItemPresentationActor::AVfxItemPresentationActor()
{
	UseVfxComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("UseVfxComponent"));
	if (Root)
	{
		UseVfxComponent->SetupAttachment(Root);
	}
	else
	{
		UseVfxComponent->SetupAttachment(GetRootComponent());
	}

	UseVfxComponent->SetAutoActivate(false);
}

void AVfxItemPresentationActor::ReceiveItemUseActiveStarted_Implementation()
{
	Super::ReceiveItemUseActiveStarted_Implementation();

	if (!UseVfxComponent)
	{
		return;
	}

	if (bResetVfxOnStart)
	{
		UseVfxComponent->ResetSystem();
	}

	UseVfxComponent->Activate(true);
}

void AVfxItemPresentationActor::ReceiveItemUseActiveEnded_Implementation(const bool bCanceled)
{
	Super::ReceiveItemUseActiveEnded_Implementation(bCanceled);

	if (!UseVfxComponent)
	{
		return;
	}

	if (bDeactivateImmediatelyOnEnd)
	{
		UseVfxComponent->DeactivateImmediate();
	}
	else
	{
		UseVfxComponent->Deactivate();
	}
}

