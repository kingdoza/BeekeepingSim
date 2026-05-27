#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorItemUseAreaTypes.h"
#include "ItemUseAreaMeshProviderComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UItemUseAreaMeshProviderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void BuildItemUseAreaDescriptors(TArray<FItemUseAreaDescriptor>& OutDescriptors) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	FName RequiredChildActorComponentTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	bool bIncludeOwnerComponents = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	bool bIncludeDirectChildActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	bool bLogCollectionDebug = false;
};

