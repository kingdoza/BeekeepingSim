#pragma once

#include "CoreMinimal.h"
#include "Environment/DynamicSkyTypes.h"
#include "GameFramework/Actor.h"
#include "DynamicSky.generated.h"

class AActor;
class AGameTimeOfDayActor;
class AEnvironmentTimeOfDayActor;
class UExponentialHeightFogComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPostProcessComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UDirectionalLightComponent;
struct FTimeOfDayVisualState;

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API ADynamicSky : public AActor
{
	GENERATED_BODY()

public:
	ADynamicSky();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dynamic Sky|Preview")
	void ApplyPreviewTime();

	UFUNCTION(BlueprintPure, Category = "Dynamic Sky|Preview")
	bool ShouldStartGameTimeFromPreviewHour() const { return bUseEditorPreviewTime && bStartGameTimeFromPreviewHour; }

	UFUNCTION(BlueprintPure, Category = "Dynamic Sky|Preview")
	float GetPreviewHour24() const { return PreviewHour24; }

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	static float NormalizeHour24(float Hour24);
	static bool IsTimeWithinWrappedRange(float Hour24, float RangeStart, float RangeEnd);
	static float CalculateOrbitElapsedHours(float Hour24, float OrbitStartHour, float OrbitDurationHours, float GapHours);
	float EvaluateSunLightBlendAlpha(float Hour24) const;
	FLinearColor EvaluateRayleighScattering(float Hour24) const;
	float EvaluateMultiScattering(float Hour24) const;
	bool ResolveAndBindTimeSource();
	void UnbindTimeSource();
	void HandleProviderHour(float Hour24);
	FDynamicSkyState EvaluateSkyState(float Hour24) const;
	void ApplySkyState(float Hour24);
	void ApplySkyStateInternal(const FDynamicSkyState& SkyState);
	void RefreshSkySphereMaterial();
	bool ShouldUseEditorPreview() const;

	UFUNCTION()
	void HandleGameTimeOfDayChanged(float Hour24);

	UFUNCTION()
	void HandleLegacyTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDirectionalLightComponent> SunDirectionalLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDirectionalLightComponent> MoonDirectionalLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SkySphereMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Time", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> TimeOfDaySourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Time", meta = (AllowPrivateAccess = "true"))
	bool bAutoFindTimeOfDaySource = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Time", meta = (AllowPrivateAccess = "true"))
	bool bApplyCurrentTimeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Orbit", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "24.0"))
	float SunriseHour = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Orbit", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "24.0"))
	float SunsetHour = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Orbit", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float GapTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Orbit", meta = (AllowPrivateAccess = "true"))
	float OrbitYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
	FLinearColor SunLightRayleighScattering = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
	FLinearColor NoSunLightRayleighScattering = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
	float SunLightMultiScattering = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
	float NoSunLightMultiScattering = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Sphere", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> SkySphereMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Sphere", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> SkySphereMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SkySphereMID = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Preview", meta = (AllowPrivateAccess = "true"))
	bool bUseEditorPreviewTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Preview", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "24.0", EditCondition = "bUseEditorPreviewTime", EditConditionHides))
	float PreviewHour24 = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Preview", meta = (AllowPrivateAccess = "true", EditCondition = "bUseEditorPreviewTime"))
	bool bUpdatePreviewOnPropertyChange = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Preview", meta = (AllowPrivateAccess = "true", EditCondition = "bUseEditorPreviewTime"))
	bool bStartGameTimeFromPreviewHour = true;

	UPROPERTY(Transient)
	TObjectPtr<AGameTimeOfDayActor> BoundGameTimeOfDayActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AEnvironmentTimeOfDayActor> BoundLegacyTimeOfDayActor = nullptr;
};
