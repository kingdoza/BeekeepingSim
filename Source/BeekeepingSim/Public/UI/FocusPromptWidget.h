#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Focus/FocusTargetComponent.h"
#include "FocusPromptWidget.generated.h"

class UBeekeeperFocusComponent;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UFocusPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void BindToFocusComponent(UBeekeeperFocusComponent* InFocusComponent);

	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void UnbindFromFocusComponent();

	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void SetPromptData(const FFocusPromptData& InPromptData);

	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void ClearPrompt();

	UFUNCTION(BlueprintPure, Category = "Focus Prompt")
	const FFocusPromptData& GetCurrentPromptData() const { return CurrentPromptData; }

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Focus Prompt")
	void OnPromptDataApplied(const FFocusPromptData& PromptData, bool bVisible);

private:
	UFUNCTION()
	void HandleFocusPromptChanged(FFocusPromptData PromptData);

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TargetNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> BoundFocusComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Focus Prompt", meta = (AllowPrivateAccess = "true"))
	FFocusPromptData CurrentPromptData;
};
