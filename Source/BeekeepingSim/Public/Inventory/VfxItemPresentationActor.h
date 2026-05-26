#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemPresentationActor.h"
#include "VfxItemPresentationActor.generated.h"

class UNiagaraComponent;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API AVfxItemPresentationActor : public AItemPresentationActor
{
	GENERATED_BODY()

public:
	AVfxItemPresentationActor();

protected:
	virtual void ReceiveItemUseActiveStarted_Implementation() override;
	virtual void ReceiveItemUseActiveEnded_Implementation(bool bCanceled) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> UseVfxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Presentation|VFX")
	bool bResetVfxOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Presentation|VFX")
	bool bDeactivateImmediatelyOnEnd = false;
};

