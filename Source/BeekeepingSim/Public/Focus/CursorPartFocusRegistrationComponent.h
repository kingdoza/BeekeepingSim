#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CursorPartFocusRegistrationComponent.generated.h"

class UCursorPartFocusScopeComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UCursorPartFocusRegistrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void RebuildCursorPartFocusDescriptors();

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void AppendCursorPartFocusDescriptorsToScope();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Debug")
	bool bLogRegistrationDebug = false;

private:
	void GatherAndRegisterDescriptors(UCursorPartFocusScopeComponent* ScopeComponent) const;
};
