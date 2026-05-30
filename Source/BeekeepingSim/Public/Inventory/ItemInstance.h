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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
	bool bIsFrontFaceVisible = true;
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
	void ClearBeehiveCombState();

	UFUNCTION(BlueprintPure, Category = "Item|Beehive Comb")
	bool HasBeehiveCombState() const;

	UFUNCTION(BlueprintPure, Category = "Item|Beehive Comb")
	FBeehiveCombItemState GetBeehiveCombState() const { return BeehiveCombState; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FGuid InstanceId;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UItemAction>> Actions;
};
