#include "Character/BeekeeperFlashlightComponent.h"

UBeekeeperFlashlightComponent::UBeekeeperFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);
	SetIntensity(5000.0f);
	SetAttenuationRadius(1200.0f);
	SetInnerConeAngle(18.0f);
	SetOuterConeAngle(32.0f);
	SetLightColor(FLinearColor::White);
	SetCastShadows(true);
	SetVisibility(false);
}

void UBeekeeperFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyFlashlightSettings();
	SetFlashlightEnabled(bStartEnabled);
}

void UBeekeeperFlashlightComponent::InitializeFlashlightAttachment(USceneComponent* InAttachParent)
{
	if (!InAttachParent)
	{
		return;
	}

	AttachToComponent(InAttachParent, FAttachmentTransformRules::KeepRelativeTransform);
	SetRelativeLocationAndRotation(FlashlightRelativeLocation, FlashlightRelativeRotation);
}

void UBeekeeperFlashlightComponent::ToggleFlashlight()
{
	SetFlashlightEnabled(!bIsFlashlightEnabled);
}

void UBeekeeperFlashlightComponent::SetFlashlightEnabled(bool bEnabled)
{
	bIsFlashlightEnabled = bEnabled;
	SetVisibility(bIsFlashlightEnabled);
}

void UBeekeeperFlashlightComponent::ApplyFlashlightSettings()
{
	SetRelativeLocationAndRotation(FlashlightRelativeLocation, FlashlightRelativeRotation);
}
