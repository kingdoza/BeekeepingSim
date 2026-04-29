// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "HotbarItemInterface.generated.h"

UINTERFACE(Blueprintable)
class BEEKEEPINGSIM_API UHotbarItemInterface : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IHotbarItemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hotbar")
	FGameplayTagContainer GetHotbarItemTags() const;
};
