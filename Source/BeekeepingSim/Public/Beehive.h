// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Public/FocusInteractable.h"
#include "Beehive.generated.h"

class UFocusTargetComponent;
class UAnchoredFocusCursorActionComponent;
class USceneComponent;
class UStaticMeshComponent;
class ABeekeeperCharacter;

UCLASS()
class BEEKEEPINGSIM_API ABeehive : public AActor, public IFocusInteractable
{
	GENERATED_BODY()

public:
	ABeehive();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BeehiveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFocusTargetComponent> FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAnchoredFocusCursorActionComponent> FocusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive")
	bool bIsLidOpen = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusEntered(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusExited(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusConfirmed(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Beehive")
	void ReceiveFocusCanceled(ABeekeeperCharacter* InteractingCharacter);

public:
	virtual void OnFocusEnter_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusExit_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusConfirm_Implementation(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void OnFocusCancel_Implementation(ABeekeeperCharacter* InteractingCharacter) override;
};
