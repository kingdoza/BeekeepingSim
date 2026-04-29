// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FocusTargetComponent.generated.h"

class ABeekeeperCharacter;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FFocusItemRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FGameplayTagContainer AllowedItemTags;
};

USTRUCT(BlueprintType)
struct FFocusPromptData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	bool bIsValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FText InteractionKeyText;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UFocusTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFocusTargetComponent();

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void SetFocused(bool bInFocused);

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void SetDisplayName(const FText& NewDisplayName);

	UFUNCTION(BlueprintCallable, Category = "Focus")
	void SetInteractionKeyText(const FText& NewInteractionKeyText);

	UFUNCTION(BlueprintPure, Category = "Focus")
	FFocusPromptData GetPromptData() const;

	UFUNCTION(BlueprintPure, Category = "Focus")
	const FFocusItemRule& GetItemRule() const { return FocusItemRule; }

	void NotifyFocusEnter(ABeekeeperCharacter* InteractingCharacter);

	void NotifyFocusExit(ABeekeeperCharacter* InteractingCharacter);

	void NotifyFocusConfirm(ABeekeeperCharacter* InteractingCharacter);

	void NotifyFocusCancel(ABeekeeperCharacter* InteractingCharacter);

protected:
	virtual void BeginPlay() override;

	void ApplyOutlineState(bool bEnabled);

	void ResolveOutlineComponents();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FText InteractionKeyText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FFocusItemRule FocusItemRule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus|Outline")
	bool bUseCustomDepthOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus|Outline", meta = (ClampMin = "0"))
	int32 CustomDepthStencilValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus|Outline")
	TArray<FName> OutlineComponentTags;

private:
	bool bIsFocused = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> ResolvedOutlineComponents;
};
