#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "BeekeeperFlashlightComponent.generated.h"

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UBeekeeperFlashlightComponent : public USpotLightComponent
{
	GENERATED_BODY()

public:
	UBeekeeperFlashlightComponent();

	virtual void BeginPlay() override;

	void InitializeFlashlightAttachment(USceneComponent* AttachParent);
	void ToggleFlashlight();
	void SetFlashlightEnabled(bool bEnabled);
	bool IsFlashlightEnabled() const { return bIsFlashlightEnabled; }
	void ApplyFlashlightSettings();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
	bool bStartEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
	FVector FlashlightRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
	FRotator FlashlightRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	bool bIsFlashlightEnabled = false;
};
