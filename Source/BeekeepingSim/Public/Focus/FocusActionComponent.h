// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorPartFocusTypes.h"
#include "Inventory/HotbarPresentationTypes.h"
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
	virtual bool HandleConfirmInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandleCancelInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandlePartFocusClickInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandlePartFocusPreviewKeyInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual void AbortFocusAction(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintPure, Category = "Focus Action")
	bool IsActionEngaged() const { return bIsActionEngaged; }

	UFUNCTION(BlueprintPure, Category = "Focus Action|UI")
	virtual bool WantsCrosshairHiddenWhileEngaged() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|UI")
	virtual bool ShouldRestoreCrosshairOnCancelStart() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|Hotbar")
	virtual EHotbarPresentationMode GetHotbarPresentationModeWhileEngaged() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|Hotbar")
	virtual bool ShouldClearHotbarSelectionOnFocusEngaged() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|Hotbar")
	virtual bool ShouldBlockHotbarSlotInputWhileEngaged() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|Hotbar")
	virtual bool ShouldBlockHotbarWheelInputWhileEngaged() const;

protected:
	bool bIsActionEngaged = false;
};
