#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Focus/FocusTargetComponent.h"
#include "FocusPromptWidget.generated.h"

class UBeekeeperFocusComponent;
class UTextBlock;
class UWidget;

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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Focus Prompt")
	void OnPromptDataApplied(const FFocusPromptData& PromptData, bool bVisible);

private:
	UFUNCTION()
	void HandleFocusPromptChanged(FFocusPromptData PromptData);

	void UpdatePromptPosition();
	bool TryGetPromptAnchorPosition(FVector2D& OutPosition) const;
	bool TryGetViewportSizeInWidgetUnits(FVector2D& OutViewportSize) const;
	bool TryGetMousePositionInWidgetUnits(FVector2D& OutMousePosition) const;
	static FVector2D ClampPromptAnchorToViewport(
		const FVector2D& AnchorPosition,
		const FVector2D& ViewportSize,
		const FVector2D& DesiredSize,
		const FVector2D& Alignment,
		const FVector2D& Padding);

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UWidget> PromptContent;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TargetNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Prompt|Layout", meta = (AllowPrivateAccess = "true"))
	FVector2D ScreenCenterOffset = FVector2D(20.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Prompt|Layout", meta = (AllowPrivateAccess = "true"))
	FVector2D MouseCursorOffset = FVector2D(18.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Prompt|Layout", meta = (AllowPrivateAccess = "true"))
	FVector2D ViewportPadding = FVector2D(8.0f, 8.0f);

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> BoundFocusComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Focus Prompt", meta = (AllowPrivateAccess = "true"))
	FFocusPromptData CurrentPromptData;
};
