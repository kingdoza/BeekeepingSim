// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FocusInteractable.generated.h"

class ABeekeeperCharacter;

UINTERFACE(Blueprintable)
class BEEKEEPINGSIM_API UFocusInteractable : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IFocusInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focus")
	void OnFocusEnter(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focus")
	void OnFocusExit(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focus")
	void OnFocusConfirm(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focus")
	void OnFocusCancel(ABeekeeperCharacter* InteractingCharacter);
};
