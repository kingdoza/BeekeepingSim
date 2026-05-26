// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorPartFocusTypes.h"
#include "GameplayTagContainer.h"
#include "Focus/FocusTargetComponent.h"
#include "BeekeeperFocusComponent.generated.h"

class ABeekeeperCharacter;
class UCameraComponent;
class UFocusActionComponent;
class UFocusTargetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeekeeperFocusPromptChangedSignature, FFocusPromptData, PromptData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBeekeeperFocusRuleChangedSignature, bool, bHasFocusTarget, FFocusItemRule, FocusItemRule);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeekeeperCrosshairVisibilityChangedSignature, bool, bVisible);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeekeeperFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBeekeeperFocusComponent();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void ConfirmFocus();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void CancelFocus();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void HandleFocusPrimaryPressedInput();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void HandleFocusPrimaryReleasedInput();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	bool HandlePartFocusClickInput();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	bool HandlePartFocusClickReleasedInput();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	bool HandlePartFocusPointerPressedInput();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	bool HandlePartFocusPointerReleasedInput();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	bool HandlePartFocusPreviewKeyInput(ECursorPartFocusPreviewInputKey Key);

	UFUNCTION(BlueprintCallable, Category = "Focus")
	bool HandleSecondaryInput();

	UFUNCTION(BlueprintPure, Category = "Focus")
	UFocusTargetComponent* GetCurrentFocusTarget() const { return CurrentFocusTarget; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	bool HasFocusTarget() const { return CurrentFocusTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	UFocusTargetComponent* GetEngagedFocusTarget() const { return EngagedFocusTarget; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	UFocusActionComponent* GetEngagedFocusAction() const { return EngagedFocusAction; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	UFocusTargetComponent* GetActiveInteractionTarget() const { return bIsFocusEngaged ? EngagedFocusTarget : CurrentFocusTarget; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	bool IsFocusEngaged() const { return bIsFocusEngaged; }

	UFUNCTION(BlueprintPure, Category = "Focus|UI")
	bool ShouldHideCrosshair() const { return bShouldHideCrosshair; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	FFocusPromptData GetCurrentPromptData() const;

	UFUNCTION(BlueprintPure, Category = "Focus")
	bool EvaluateItemAllowed(const FGameplayTagContainer& ItemTags, FGameplayTag AllItemsTag) const;

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void SetEngagedFocusPromptOverride(const FFocusPromptData& PromptData);

	UPROPERTY(BlueprintAssignable, Category = "Focus")
	FBeekeeperFocusPromptChangedSignature OnFocusPromptChanged;

	UPROPERTY(BlueprintAssignable, Category = "Focus")
	FBeekeeperFocusRuleChangedSignature OnFocusRuleChanged;

	UPROPERTY(BlueprintAssignable, Category = "Focus|UI")
	FBeekeeperCrosshairVisibilityChangedSignature OnCrosshairVisibilityChanged;

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RefreshFocusTarget();

	UFocusTargetComponent* FindFocusTargetFromTrace() const;

	void SetPreviewFocusTarget(UFocusTargetComponent* NewFocusTarget);

	void ClearPreviewFocus(bool bNotifyCancel);

	void ClearEngagedFocus();

	void BroadcastPreviewPromptState();

	void BroadcastEngagedFocusRule();

	void UpdateCrosshairVisibility(bool bNewShouldHideCrosshair);

	void RefreshCrosshairVisibilityFromCurrentAction();

	UFocusActionComponent* FindFocusActionComponent(const UFocusTargetComponent* FocusTarget) const;

	void UpdateEngagedFocusState();

	bool ShouldDisableTickForNonLocal() const;

	bool TryGetMouseScreenPosition(FVector2D& OutPosition) const;

	void ResetFocusPrimaryGestureState();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FocusTraceDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> FocusTraceChannel = ECC_Visibility;

	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> OwnerCamera;

	UPROPERTY(Transient)
	TObjectPtr<UFocusTargetComponent> CurrentFocusTarget;

	UPROPERTY(Transient)
	TObjectPtr<UFocusTargetComponent> EngagedFocusTarget;

	UPROPERTY(Transient)
	TObjectPtr<UFocusActionComponent> EngagedFocusAction;

	UPROPERTY(Transient)
	FFocusPromptData EngagedPromptOverride;

	UPROPERTY(Transient)
	bool bHasEngagedPromptOverride = false;

	UPROPERTY(Transient)
	TObjectPtr<UFocusTargetComponent> PressedFocusTarget;

	FVector2D PressedFocusScreenPosition = FVector2D::ZeroVector;
	float MaxFocusPointerMoveDistanceSincePress = 0.0f;
	bool bIsFocusPrimaryPointerDown = false;
	bool bFocusClickCanceledByMovement = false;

	bool bIsFocusEngaged = false;

	bool bShouldHideCrosshair = false;
};
