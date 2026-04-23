// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FocusActionComponent.generated.h"

class ABeekeeperCharacter;

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UFocusActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFocusActionComponent();

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const;

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool BeginFocusAction(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool CancelFocusAction(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual void AbortFocusAction(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintPure, Category = "Focus Action")
	bool IsActionEngaged() const { return bIsActionEngaged; }

	UFUNCTION(BlueprintPure, Category = "Focus Action|UI")
	virtual bool WantsCrosshairHiddenWhileEngaged() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|UI")
	virtual bool ShouldRestoreCrosshairOnCancelStart() const;

protected:
	bool bIsActionEngaged = false;
};
