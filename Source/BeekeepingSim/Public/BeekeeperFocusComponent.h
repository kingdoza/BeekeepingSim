// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Public/FocusTargetComponent.h"
#include "BeekeeperFocusComponent.generated.h"

class ABeekeeperCharacter;
class UCameraComponent;
class UFocusTargetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeekeeperFocusPromptChangedSignature, FFocusPromptData, PromptData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBeekeeperFocusRuleChangedSignature, bool, bHasFocusTarget, FFocusItemRule, FocusItemRule);

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

	UFUNCTION(BlueprintPure, Category = "Focus")
	UFocusTargetComponent* GetCurrentFocusTarget() const { return CurrentFocusTarget; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	bool HasFocusTarget() const { return CurrentFocusTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Focus")
	FFocusPromptData GetCurrentPromptData() const;

	UFUNCTION(BlueprintPure, Category = "Focus")
	bool EvaluateItemAllowed(const FGameplayTagContainer& ItemTags, FGameplayTag AllItemsTag) const;

	UPROPERTY(BlueprintAssignable, Category = "Focus")
	FBeekeeperFocusPromptChangedSignature OnFocusPromptChanged;

	UPROPERTY(BlueprintAssignable, Category = "Focus")
	FBeekeeperFocusRuleChangedSignature OnFocusRuleChanged;

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RefreshFocusTarget();

	UFocusTargetComponent* FindFocusTargetFromTrace() const;

	void SetFocusTarget(UFocusTargetComponent* NewFocusTarget);

	void ClearFocusTarget(bool bNotifyCancel);

	void BroadcastFocusState();

	bool ShouldDisableTickForNonLocal() const;

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
};
