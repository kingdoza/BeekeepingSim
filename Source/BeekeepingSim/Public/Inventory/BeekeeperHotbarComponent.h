// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/HotbarPresentationTypes.h"
#include "Focus/FocusTargetComponent.h"
#include "UI/ItemSlotDragDropTypes.h"
#include "BeekeeperHotbarComponent.generated.h"

class ABeekeeperCharacter;
class UBeekeeperFocusComponent;
class UFocusActionComponent;
class UItemDefinition;
class UItemInstance;
class UTexture2D;

USTRUCT(BlueprintType)
struct FHotbarSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TObjectPtr<UObject> ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	bool bIsEnabled = true;
};

USTRUCT(BlueprintType)
struct FHotbarItemAcquireResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	bool bPartiallySucceeded = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	int32 RequestedQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	int32 AddedQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	int32 RemainingQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	FText Message;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBeekeeperHotbarChangedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeekeeperHotbarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr int32 SlotCount = 8;

	UBeekeeperHotbarComponent();

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void InitializeSlots();

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void HandleSlotInput(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void HandleWheelInput(bool bForward);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void SelectSlot(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void ToggleSelectionFromLastSelectedSlot();

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void ApplyFocusRule(bool bEngaged, const FFocusItemRule& Rule);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void ReevaluateSlots();

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void NotifyHotbarItemsChanged();

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	FHotbarItemAcquireResult TryAcquireItem(UItemDefinition* ItemDefinition, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	const TArray<FHotbarSlotData>& GetSlots() const { return Slots; }

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	int32 GetSelectedIndex() const { return SelectedIndex; }

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	int32 GetLastSelectedIndex() const { return LastSelectedIndex; }

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	UObject* GetSelectedItem() const;

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	UItemInstance* GetSelectedItemInstance() const;

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	EHotbarPresentationMode GetPresentationMode() const;

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void SetSlotItem(int32 Index, UObject* NewItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool SwapSlots(int32 FromIndex, int32 ToIndex);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	FItemSlotMoveResult MovePartialToSlot(int32 FromIndex, int32 ToIndex, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Hotbar|UI")
	FText GetSelectedItemDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Hotbar|UI")
	UTexture2D* GetSelectedItemIcon() const;

	UFUNCTION(BlueprintPure, Category = "Hotbar|UI")
	int32 GetSelectedItemStackCount() const;

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	bool IsSlotEnabled(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool ApplySelectedItemStackDelta(int32 StackDelta);

	UPROPERTY(BlueprintAssignable, Category = "Hotbar")
	FBeekeeperHotbarChangedSignature OnHotbarChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleFocusRuleChanged(bool bHasFocusTarget, FFocusItemRule FocusItemRule);

	void BroadcastHotbarChanged();

	bool IsIndexValid(int32 Index) const;

	FGameplayTagContainer GetItemTagsForSlot(int32 Index) const;

	bool IsSlotAllowedByActiveRule(int32 Index) const;

	bool ShouldClearSelectionByActiveFocusPolicy() const;

	bool ShouldClearSelectedSlot() const;

	bool ReevaluateSlotsInternal();

	int32 FindFirstEmptySlot() const;
	void RememberSelectedIndex();
	int32 ResolveToggleFallbackSelectionIndex() const;

	UItemInstance* CreateItemInstance(UItemDefinition* ItemDefinition, int32 StackCount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TArray<FHotbarSlotData> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	FGameplayTag AllItemsRootTag;

private:
	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> FocusComponent;

	UPROPERTY(Transient)
	TObjectPtr<UFocusActionComponent> ActiveFocusAction;

	int32 SelectedIndex = INDEX_NONE;
	int32 LastSelectedIndex = 0;

	bool bIsEngagedFocusActive = false;

	FFocusItemRule ActiveFocusRule;
};
