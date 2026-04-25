#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/BeekeeperHotbarComponent.h"
#include "BeekeeperHeldItemVisualizerComponent.generated.h"

class ABeekeeperCharacter;
class UBeekeeperFocusComponent;
class UBeekeeperHotbarComponent;
class UCameraComponent;
class AItemPresentationActor;
class UItemInstance;
class UStaticMesh;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeekeeperHeldItemVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBeekeeperHeldItemVisualizerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void HandleHotbarChanged();

	UFUNCTION()
	void HandleFocusRuleChanged(bool bHasFocusTarget, FFocusItemRule FocusItemRule);

	void RefreshVisualization();

	void UpdateCursorPresentation();

	void UpdatePresentationForSelectedItem();

	void UpdateVisibilityAndTransform();

	bool ShouldRunLocally() const;

	void EnsurePresentationActor();

	void DestroyPresentationActor();

	UItemInstance* GetSelectedItemInstance() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	FVector InHandLocalOffset = FVector(45.0f, 18.0f, -18.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	FRotator InHandLocalRotation = FRotator(-10.0f, -8.0f, 6.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	FRotator OnCursorLocalRotation = FRotator(-2.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float OnCursorPlaneDistance = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	FVector OnCursorLocalOffset = FVector(0.0f, 0.0f, -8.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	FVector InHandRelativeScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	FVector OnCursorRelativeScale = FVector(1.0f, 1.0f, 1.0f);

private:
	UPROPERTY(Transient)
	TObjectPtr<ABeekeeperCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperHotbarComponent> HotbarComponent;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> FocusComponent;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> OwnerCamera;

	UPROPERTY(Transient)
	TObjectPtr<AItemPresentationActor> HeldPresentationActor;

	UPROPERTY(Transient)
	TObjectPtr<UItemInstance> CurrentVisualizedItem;

	UPROPERTY(Transient)
	TSubclassOf<AItemPresentationActor> CurrentPresentationClass;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CurrentFallbackMesh;

	bool bWasRunningLocally = false;
};
