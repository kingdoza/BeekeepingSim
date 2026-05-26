#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorPartFocusProvider.h"
#include "ChildCursorPartFocusProviderComponent.generated.h"

class AActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UChildCursorPartFocusProviderComponent : public UActorComponent, public ICursorPartFocusProvider
{
	GENERATED_BODY()

public:
	virtual void GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	FName RequiredChildActorComponentTag = TEXT("PartFocusChild");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	TSubclassOf<AActor> RequiredChildActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	bool bLogSkippedChildren = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Debug")
	bool bLogProviderDebug = false;
};
