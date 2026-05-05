#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Focus/CursorPartFocusTypes.h"
#include "GameplayTagContainer.h"
#include "CursorPartFocusActionComponent.generated.h"

class ABeekeeperCharacter;
class UCursorPartFocusScopeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCursorPartFocusActionSignature, UCursorPartFocusActionComponent*, ActionComponent, UCursorPartFocusScopeComponent*, ScopeComponent, ABeekeeperCharacter*, InteractingCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCursorPartFocusPreviewKeyActionSignature, UCursorPartFocusActionComponent*, ActionComponent, UCursorPartFocusScopeComponent*, ScopeComponent, ABeekeeperCharacter*, InteractingCharacter, ECursorPartFocusPreviewInputKey, Key);

UENUM(BlueprintType)
enum class ECursorPartFocusEngageMode : uint8
{
	PreviewOnly,
	InstantAction,
	PersistentAction
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UCursorPartFocusActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCursorPartFocusActionComponent();

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool CanBeginPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const;

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool BeginPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool CancelPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void AbortPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	bool IsPartActionEngaged() const { return bIsPartActionEngaged; }

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	ECursorPartFocusEngageMode GetEngageMode() const { return EngageMode; }

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	const FGameplayTagContainer& GetProvidedStateTags() const { return ProvidedStateTags; }

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	const FGameplayTagContainer& GetRequiredStateTags() const { return RequiredStateTags; }

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	FGameplayTag GetExclusiveGroup() const { return ExclusiveGroup; }

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void SetEngageMode(ECursorPartFocusEngageMode NewEngageMode) { EngageMode = NewEngageMode; }

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void SetProvidedStateTags(const FGameplayTagContainer& NewTags) { ProvidedStateTags = NewTags; }

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void SetRequiredStateTags(const FGameplayTagContainer& NewTags) { RequiredStateTags = NewTags; }

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void SetExclusiveGroup(FGameplayTag NewGroup) { ExclusiveGroup = NewGroup; }

	UFUNCTION(BlueprintPure, Category = "Cursor Part Focus")
	bool CanHandlePreviewKeyAction(ECursorPartFocusPreviewInputKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	bool HandlePreviewKeyAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key);

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusActionSignature OnPartFocusBegin;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusActionSignature OnPartFocusCancel;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusActionSignature OnPartFocusAbort;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusPreviewKeyActionSignature OnPartFocusPreviewKeyAction;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusActionSignature OnPartFocusPreviewR;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusActionSignature OnPartFocusPreviewF;

	UPROPERTY(BlueprintAssignable, Category = "Cursor Part Focus|Events")
	FCursorPartFocusActionSignature OnPartFocusPreviewC;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	ECursorPartFocusEngageMode EngageMode = ECursorPartFocusEngageMode::PersistentAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	FGameplayTagContainer ProvidedStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	FGameplayTagContainer RequiredStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
	FGameplayTag ExclusiveGroup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Preview Key")
	bool bEnableRPreviewAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Preview Key")
	bool bEnableFPreviewAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus|Preview Key")
	bool bEnableCPreviewAction = false;

	bool bIsPartActionEngaged = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Begin"))
	void ReceivePartFocusBegin(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Cancel"))
	void ReceivePartFocusCancel(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Abort"))
	void ReceivePartFocusAbort(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Preview Key Action"))
	void ReceivePartFocusPreviewKeyAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, ECursorPartFocusPreviewInputKey Key);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Preview R"))
	void ReceivePartFocusPreviewR(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Preview F"))
	void ReceivePartFocusPreviewF(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursor Part Focus", meta = (DisplayName = "Receive Part Focus Preview C"))
	void ReceivePartFocusPreviewC(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);
};
