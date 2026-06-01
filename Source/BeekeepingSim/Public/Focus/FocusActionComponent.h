// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorPartFocusTypes.h"
#include "Focus/FocusTargetComponent.h"
#include "Inventory/HotbarPresentationTypes.h"
#include "FocusActionComponent.generated.h"

class ABeekeeperCharacter;
class UFocusTargetComponent;

USTRUCT()
struct FFocusPromptBuildContext
{
	GENERATED_BODY()

	ABeekeeperCharacter* InteractingCharacter = nullptr;
	UFocusTargetComponent* FocusTarget = nullptr;
	FFocusPromptData BasePromptData;
};

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
	virtual bool HandlePartFocusClickReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandlePartFocusPointerPressedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandlePartFocusPointerReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandlePartFocusPreviewKeyInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key);

	UFUNCTION(BlueprintCallable, Category = "Focus Action")
	virtual bool HandleSecondaryInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);

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

	UFUNCTION(BlueprintCallable, Category = "Focus Action|Prompt")
	void SetPromptActionText(const FText& NewText);

	UFUNCTION(BlueprintPure, Category = "Focus Action|Prompt")
	FText GetPromptActionText() const;

	UFUNCTION(BlueprintCallable, Category = "Focus Action|Prompt")
	void SetEngagedPromptActionText(const FText& NewText);

	UFUNCTION(BlueprintPure, Category = "Focus Action|Prompt")
	FText GetEngagedPromptActionText() const;

	UFUNCTION(BlueprintPure, Category = "Focus Action|Prompt")
	virtual FText ResolveFocusPromptActionText() const;

	virtual void AppendFocusPromptEntries(const FFocusPromptBuildContext& Context, TArray<FFocusPromptEntry>& OutEntries) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Action|Prompt")
	FText PromptActionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Action|Prompt")
	FText EngagedPromptActionText;

	bool bIsActionEngaged = false;
};
