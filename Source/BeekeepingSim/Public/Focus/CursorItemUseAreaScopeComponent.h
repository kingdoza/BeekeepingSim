#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorItemUseAreaTypes.h"
#include "CursorItemUseAreaScopeComponent.generated.h"

class ABeekeeperCharacter;
class UBeekeeperFocusComponent;
class UBeekeeperHotbarComponent;
class UHoldItemUseAction;
class UItemInstance;
class UMaterialInstanceDynamic;
class UCursorPartFocusScopeComponent;
struct FItemActionContext;
struct FItemActionExecutionResult;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UCursorItemUseAreaScopeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCursorItemUseAreaScopeComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void ActivateItemUseAreaScope(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void DeactivateItemUseAreaScope(bool bCancelActiveUse);

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void RebuildItemUseAreaDescriptors();

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void RegisterItemUseAreaDescriptor(const FItemUseAreaDescriptor& Descriptor);

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	bool HandleItemUsePressed();

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	bool HandleItemUseReleased();

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	bool HandleItemUseCanceled();

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	bool IsItemUseAreaScopeActive() const { return bIsScopeActive; }

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	bool IsItemUseInProgress() const { return bIsUseInProgress; }

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	bool HasActiveUseAreas() const { return ActiveDescriptorIndices.Num() > 0; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	float CursorTraceDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	TEnumAsByte<ECollisionChannel> CursorTraceChannel = ECC_Visibility;

private:
	struct FStoredUseAreaCollisionState
	{
		ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::NoCollision;
		ECollisionResponse CursorTraceResponse = ECR_Ignore;
	};

	UFUNCTION()
	void HandleHotbarChanged();

	AActor* ResolveActiveHostActor() const;
	void RebuildDescriptorsFromProviderActor(AActor* ProviderActor);
	void RebuildDescriptorsFromProviderComponents(AActor* HostActor);
	void RebuildDescriptorsFromDirectComponentTags(AActor* HostActor);
	void RefreshSelectedItemAndAction();
	void RefreshActiveUseAreas();
	void UpdateHoveredDescriptorFromCursor();
	int32 ResolveHoveredActiveDescriptor() const;
	void SetHoveredDescriptorIndex(int32 NewIndex);
	void ApplyVisualStateForDescriptor(int32 DescriptorIndex, bool bDescriptorActive, bool bIsHovered);
	void ApplyVisualStateForAllDescriptors();
	void ClearAllVisualState();
	void ApplyCollisionStateForAllDescriptors();
	void RestoreOriginalCollisionStates();
	void CacheOriginalCollisionState(UPrimitiveComponent* Component);
	UMaterialInstanceDynamic* ResolveOrCreateMID(UPrimitiveComponent* Component);
	bool DoesDescriptorMatchActionQuery(const FItemUseAreaDescriptor& Descriptor, const UHoldItemUseAction* HoldAction) const;
	void EndUseSession(bool bWasCanceled);
	struct FItemActionContext BuildItemActionContext(int32 DescriptorIndex) const;
	void ApplyUseEffectResultToSelectedItem(const FItemActionExecutionResult& Result);
	void UpdatePartFocusOutlineSuppression() const;

	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperHotbarComponent> OwnerHotbarComponent;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> OwnerFocusComponent;

	UPROPERTY(Transient)
	TObjectPtr<UCursorPartFocusScopeComponent> SiblingPartFocusScopeComponent;

	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveHostActor;

	UPROPERTY(Transient)
	TArray<FItemUseAreaDescriptor> RegisteredDescriptors;

	UPROPERTY(Transient)
	TArray<int32> ActiveDescriptorIndices;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UPrimitiveComponent>, TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, FStoredUseAreaCollisionState> OriginalCollisionStates;

	UPROPERTY(Transient)
	TObjectPtr<UItemInstance> CachedSelectedItemInstance;

	UPROPERTY(Transient)
	TObjectPtr<UHoldItemUseAction> CachedHoldAction;

	int32 HoveredDescriptorIndex = INDEX_NONE;
	bool bIsScopeActive = false;
	bool bIsUseInProgress = false;
};
