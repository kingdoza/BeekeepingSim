#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "ItemPlacementUseAction.generated.h"

class AActor;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UItemPlacementUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	UItemPlacementUseAction();

	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Placement")
	TSubclassOf<AActor> PlacedActorClass = nullptr;
};
