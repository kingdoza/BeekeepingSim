#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/ItemSlotDragDropTypes.h"
#include "StorageBoxComponent.generated.h"

class UBeekeeperHotbarComponent;
class UItemInstance;

USTRUCT(BlueprintType)
struct FStorageBoxSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage")
	TObjectPtr<UItemInstance> ItemInstance = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStorageBoxChangedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UStorageBoxComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStorageBoxComponent();

	UFUNCTION(BlueprintCallable, Category = "Storage")
	void InitializeSlots();

	UFUNCTION(BlueprintPure, Category = "Storage")
	const TArray<FStorageBoxSlotData>& GetSlots() const { return Slots; }

	UFUNCTION(BlueprintPure, Category = "Storage")
	UItemInstance* GetItemAt(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "Storage")
	bool IsIndexValid(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool SetSlotItem(int32 Index, UItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool ClearSlot(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool SwapStorageSlots(int32 FromIndex, int32 ToIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool MoveHotbarItemToStorage(UBeekeeperHotbarComponent* HotbarComponent, int32 HotbarIndex, int32 StorageIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool MoveStorageItemToHotbar(UBeekeeperHotbarComponent* HotbarComponent, int32 StorageIndex, int32 HotbarIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool SwapHotbarAndStorage(UBeekeeperHotbarComponent* HotbarComponent, int32 HotbarIndex, int32 StorageIndex);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	FItemSlotMoveResult MovePartialStorageToStorage(int32 FromIndex, int32 ToIndex, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	FItemSlotMoveResult MovePartialStorageToHotbar(UBeekeeperHotbarComponent* HotbarComponent, int32 StorageIndex, int32 HotbarIndex, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	FItemSlotMoveResult MovePartialHotbarToStorage(UBeekeeperHotbarComponent* HotbarComponent, int32 HotbarIndex, int32 StorageIndex, int32 Quantity);

	UPROPERTY(BlueprintAssignable, Category = "Storage")
	FStorageBoxChangedSignature OnStorageChanged;

protected:
	virtual void BeginPlay() override;

	void BroadcastStorageChanged();

	bool IsHotbarIndexValid(const UBeekeeperHotbarComponent* HotbarComponent, int32 Index) const;

	UObject* GetHotbarItemAt(const UBeekeeperHotbarComponent* HotbarComponent, int32 Index) const;

	int32 FindFirstEmptyStorageSlot() const;

	UItemInstance* CreateStorageItemInstance(class UItemDefinition* ItemDefinition, int32 StackCount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage", meta = (ClampMin = "1"))
	int32 DefaultSlotCount = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage")
	TArray<FStorageBoxSlotData> Slots;
};
