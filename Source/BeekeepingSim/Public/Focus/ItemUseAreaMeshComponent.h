#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/CursorItemUseAreaTypes.h"
#include "ItemUseAreaMeshComponent.generated.h"

UENUM(BlueprintType)
enum class EItemUseAreaEffectTargetPolicy : uint8
{
	ComponentOwner,
	HostActor,
	ExplicitObject
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UItemUseAreaMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	bool IsItemUseAreaEnabled() const { return bItemUseAreaEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void SetItemUseAreaEnabled(bool bEnabled) { bItemUseAreaEnabled = bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	FName GetResolvedAreaId() const;

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	FName GetConfiguredAreaId() const { return AreaId; }

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void SetAreaId(FName InAreaId) { AreaId = InAreaId; }

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	const FGameplayTagContainer& GetAreaTags() const { return AreaTags; }

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void SetAreaTags(const FGameplayTagContainer& InAreaTags) { AreaTags = InAreaTags; }

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	const FItemUseAreaVisualSettings& GetVisualSettings() const { return VisualSettings; }

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void SetVisualSettings(const FItemUseAreaVisualSettings& InVisualSettings) { VisualSettings = InVisualSettings; }

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy InPolicy) { EffectTargetPolicy = InPolicy; }

	UFUNCTION(BlueprintCallable, Category = "Item Use Area")
	void SetExplicitEffectTargetObject(UObject* InTargetObject) { ExplicitEffectTargetObject = InTargetObject; }

	UFUNCTION(BlueprintPure, Category = "Item Use Area")
	UObject* ResolveEffectTargetObject(AActor* HostActor) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	bool bItemUseAreaEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	FName AreaId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	FGameplayTagContainer AreaTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	FItemUseAreaVisualSettings VisualSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
	EItemUseAreaEffectTargetPolicy EffectTargetPolicy = EItemUseAreaEffectTargetPolicy::ComponentOwner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area", meta = (EditCondition = "EffectTargetPolicy == EItemUseAreaEffectTargetPolicy::ExplicitObject"))
	TObjectPtr<UObject> ExplicitEffectTargetObject = nullptr;
};
