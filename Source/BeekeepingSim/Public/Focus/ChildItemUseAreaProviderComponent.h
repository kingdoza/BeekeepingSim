#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/ItemUseAreaProvider.h"
#include "ChildItemUseAreaProviderComponent.generated.h"

class AActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UChildItemUseAreaProviderComponent : public UActorComponent, public IItemUseAreaProvider
{
	GENERATED_BODY()

public:
	virtual void GetItemUseAreaDescriptors_Implementation(TArray<FItemUseAreaDescriptor>& OutDescriptors) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	FName RequiredChildActorComponentTag = TEXT("ItemUseAreaChild");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	TSubclassOf<AActor> RequiredChildActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	bool bLogSkippedChildren = false;
};
