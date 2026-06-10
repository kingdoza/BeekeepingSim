#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/HotbarItemInterface.h"
#include "Inventory/ItemActionContext.h"
#include "Inventory/ItemActionTypes.h"
#include "UObject/Object.h"
#include "ItemInstance.generated.h"

class UItemAction;
class UItemDefinition;
class UHoldItemUseAction;
class AItemPresentationActor;

USTRUCT(BlueprintType)
struct FBeehiveCombItemState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
	bool bHasState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0.0"))
	float HoneyAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0.0"))
	float HoneyRipeness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
	bool bIsFrontFaceVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0"))
	int32 CappingMaskWidth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0"))
	int32 CappingMaskHeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
	TArray<uint8> FrontWaxCappingMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
	TArray<uint8> BackWaxCappingMask;
};

USTRUCT(BlueprintType)
struct FHoneyContainerItemState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container")
	bool bHasState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container", meta = (ClampMin = "0.0"))
	float CurrentVolumeMl = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoneyDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoneyRipeness = 0.0f;
};

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UItemInstance : public UObject, public IHotbarItemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeFromDefinition(UItemDefinition* InDefinition, int32 InStackCount = 1, float InDurability = -1.0f);

	UFUNCTION(BlueprintPure, Category = "Item")
	UItemDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetDescription() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	class UTexture2D* GetIcon() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	class UStaticMesh* GetWorldMesh() const;

	UFUNCTION(BlueprintPure, Category = "Item|Presentation")
	TSubclassOf<AItemPresentationActor> GetHeldPresentationActorClass() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetStackCount() const { return StackCount; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetStackCount(int32 NewStackCount);

	UFUNCTION(BlueprintPure, Category = "Item")
	float GetDurability() const { return Durability; }

	UFUNCTION(BlueprintPure, Category = "Item|Durability")
	bool HasDurability() const;

	UFUNCTION(BlueprintPure, Category = "Item|Durability")
	float GetCurrentDurability() const;

	UFUNCTION(BlueprintPure, Category = "Item|Durability")
	float GetMaxDurability() const;

	UFUNCTION(BlueprintPure, Category = "Item|Durability")
	float GetDurabilityRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetDurability(float NewDurability);

	UFUNCTION(BlueprintCallable, Category = "Item|Beehive Comb")
	void SetBeehiveCombState(float HoneyAmount, bool bIsFrontFaceVisible);

	UFUNCTION(BlueprintCallable, Category = "Item|Beehive Comb")
	void SetBeehiveCombStateWithRipeness(float HoneyAmount, float HoneyRipeness, bool bIsFrontFaceVisible);

	UFUNCTION(BlueprintCallable, Category = "Item|Beehive Comb")
	void SetBeehiveCombStateWithCapping(
		float HoneyAmount,
		float HoneyRipeness,
		bool bIsFrontFaceVisible,
		int32 CappingMaskWidth,
		int32 CappingMaskHeight,
		const TArray<uint8>& FrontWaxCappingMask,
		const TArray<uint8>& BackWaxCappingMask);

	UFUNCTION(BlueprintCallable, Category = "Item|Beehive Comb")
	void ClearBeehiveCombState();

	UFUNCTION(BlueprintPure, Category = "Item|Beehive Comb")
	bool HasBeehiveCombState() const;

	UFUNCTION(BlueprintPure, Category = "Item|Beehive Comb")
	FBeehiveCombItemState GetBeehiveCombState() const { return BeehiveCombState; }

	UFUNCTION(BlueprintCallable, Category = "Item|Honey Container")
	void SetHoneyContainerState(float CurrentVolumeMl, float HoneyDensity, float HoneyRipeness);

	UFUNCTION(BlueprintCallable, Category = "Item|Honey Container")
	void ClearHoneyContainerState();

	UFUNCTION(BlueprintPure, Category = "Item|Honey Container")
	bool HasHoneyContainerState() const;

	UFUNCTION(BlueprintPure, Category = "Item|Honey Container")
	FHoneyContainerItemState GetHoneyContainerState() const { return HoneyContainerState; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void CopyRuntimeStateFrom(const UItemInstance* SourceItemInstance);

	UFUNCTION(BlueprintPure, Category = "Item")
	FGuid GetInstanceId() const { return InstanceId; }

	const TArray<UItemAction*>& GetActions() const { return Actions; }

	UFUNCTION(BlueprintPure, Category = "Item")
	UItemAction* FindActionByTag(FGameplayTag ActionTag) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	UHoldItemUseAction* FindHoldItemUseAction() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	bool HasActionByTag(FGameplayTag ActionTag) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemActionExecutionResult ExecuteActionByTag(FGameplayTag ActionTag, const FItemActionContext& Context);

	virtual FGameplayTagContainer GetHotbarItemTags_Implementation() const override;

protected:
	void RebuildActions();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UItemDefinition> Definition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
	int32 StackCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float Durability = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Beehive Comb")
	FBeehiveCombItemState BeehiveCombState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Honey Container")
	FHoneyContainerItemState HoneyContainerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FGuid InstanceId;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UItemAction>> Actions;
};
