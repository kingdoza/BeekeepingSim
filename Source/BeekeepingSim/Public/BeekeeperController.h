// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BeekeeperController.generated.h"

class UInputMappingContext;

UCLASS()
class BEEKEEPINGSIM_API ABeekeeperController : public APlayerController
{
	GENERATED_BODY()
public:
	ABeekeeperController();
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Input Mappings")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	virtual void SetupInputComponent() override;
};
