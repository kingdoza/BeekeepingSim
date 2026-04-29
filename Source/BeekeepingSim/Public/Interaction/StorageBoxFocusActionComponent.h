#pragma once

#include "CoreMinimal.h"
#include "Focus/FocusActionComponent.h"
#include "StorageBoxFocusActionComponent.generated.h"

class ABeekeeperCharacter;
class UStorageBoxComponent;
class UStorageBoxWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UStorageBoxFocusActionComponent : public UFocusActionComponent
{
	GENERATED_BODY()

public:
	virtual bool CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const override;

	virtual bool BeginFocusAction(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool CancelFocusAction(ABeekeeperCharacter* InteractingCharacter) override;

	virtual void AbortFocusAction(ABeekeeperCharacter* InteractingCharacter) override;

	virtual bool WantsCrosshairHiddenWhileEngaged() const override;

	virtual bool ShouldRestoreCrosshairOnCancelStart() const override;

	virtual EHotbarPresentationMode GetHotbarPresentationModeWhileEngaged() const override;

	virtual bool ShouldClearHotbarSelectionOnFocusEngaged() const override;

	virtual bool ShouldBlockHotbarSlotInputWhileEngaged() const override;

	virtual bool ShouldBlockHotbarWheelInputWhileEngaged() const override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void CleanupInteractionUI();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage UI")
	TSubclassOf<UStorageBoxWidget> StorageWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> ActiveCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UStorageBoxWidget> ActiveWidget;

	UPROPERTY(Transient)
	TObjectPtr<UStorageBoxComponent> StorageComponent;

private:
	bool bAppliedInputMode = false;
};
