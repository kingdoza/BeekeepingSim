#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorPartFocusTypes.h"
#include "GameplayTagContainer.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "CursorPartFocusScopeComponent.generated.h"

class ABeekeeperCharacter;
class UBeekeeperFocusComponent;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FCursorPartFocusPromptData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	bool bIsValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	FText InteractionKeyText;
};

USTRUCT(BlueprintType)
struct FCursorPartFocusPartDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	FName PartId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	TArray<TObjectPtr<UPrimitiveComponent>> OutlineComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	TArray<FName> OutlineComponentTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	FCursorPartFocusPromptData PromptData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	TObjectPtr<UCursorPartFocusActionComponent> ActionHandler = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	ECursorPartFocusEngageMode EngageMode = ECursorPartFocusEngageMode::PreviewOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus")
	FGameplayTagContainer RequiredStateTags;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCursorPartFocusPromptChangedSignature, FCursorPartFocusPromptData, PromptData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UCursorPartFocusScopeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCursorPartFocusScopeComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void ActivatePartFocusScope(ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void DeactivatePartFocusScope(bool bAbortActiveActions);

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool HandleConfirmInput();

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool HandlePartFocusClickInput();

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool HandlePreviewKeyInput(ECursorPartFocusPreviewInputKey Key);

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool HandleCancelInput();

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void ClearRegisteredParts();

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void RegisterPartDescriptor(const FCursorPartFocusPartDescriptor& Descriptor);

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	FGameplayTagContainer GetActiveProvidedStateTags() const;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus")
	FCursorPartFocusPromptChangedSignature OnPartFocusPromptChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	float ScreenEdgeCancelRegionThickness = 64.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	float CursorTraceDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	TEnumAsByte<ECollisionChannel> CursorTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Outline")
	bool bUseCustomDepthOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Outline", meta = (ClampMin = "0"))
	int32 CustomDepthStencilValue = 1;

private:
	struct FResolvedPartHit
	{
		int32 PartIndex = INDEX_NONE;
		UPrimitiveComponent* HitComponent = nullptr;
	};

	FResolvedPartHit ResolvePartFromCursorTrace() const;
	void UpdateHoveredPartFromCursor();
	void SetHoveredPartIndex(int32 NewPartIndex);
	bool IsDescriptorPreviewAllowed(const FCursorPartFocusPartDescriptor& Descriptor) const;
	void ResolveDescriptorOutlineComponents(FCursorPartFocusPartDescriptor& Descriptor) const;
	FGameplayTagContainer BuildEffectiveRequiredStateTags(const FCursorPartFocusPartDescriptor& Descriptor) const;
	void ApplyOutlineForPart(int32 PartIndex, bool bEnabled);
	void BroadcastPartPrompt();
	bool CancelTopActionCascade(bool bAbort);
	bool CancelActionCascade(UCursorPartFocusActionComponent* Action, bool bAbort, TSet<TObjectPtr<UCursorPartFocusActionComponent>>& Visited);
	void RemoveInactiveActions();
	bool BeginPartActionForDescriptor(const FCursorPartFocusPartDescriptor& Descriptor);
	bool HandleEdgeCancelClick() const;
	void RequestHostFocusCancel() const;

	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> OwnerFocusComponent;

	UPROPERTY(Transient)
	TArray<FCursorPartFocusPartDescriptor> RegisteredParts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCursorPartFocusActionComponent>> ActivePartActions;

	int32 HoveredPartIndex = INDEX_NONE;
	bool bIsScopeActive = false;
};
