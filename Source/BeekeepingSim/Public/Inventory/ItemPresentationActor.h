#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemPresentationActor.generated.h"

class ABeekeeperCharacter;
class UItemInstance;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API AItemPresentationActor : public AActor
{
	GENERATED_BODY()

public:
	AItemPresentationActor();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item|Presentation")
	void InitializePresentation(ABeekeeperCharacter* InOwningCharacter, UItemInstance* InItemInstance);
	virtual void InitializePresentation_Implementation(ABeekeeperCharacter* InOwningCharacter, UItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Item|Presentation")
	void SetPresentationHidden(bool bInHidden);

	UFUNCTION(BlueprintCallable, Category = "Item|Presentation")
	void DisablePresentationCollision();

	UFUNCTION(BlueprintCallable, Category = "Item|Presentation")
	void ApplyFirstPersonVisibilityPolicy(
		bool bOnlyOwnerSee = true,
		bool bCastShadow = false,
		bool bVisibleInReflectionCaptures = false,
		bool bVisibleInRayTracing = false);

	UFUNCTION(BlueprintCallable, Category = "Item|Presentation")
	void SetFallbackStaticMesh(UStaticMesh* InStaticMesh);

	UFUNCTION(BlueprintPure, Category = "Item|Presentation")
	UStaticMeshComponent* GetFallbackMeshComponent() const { return FallbackMeshComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FallbackMeshComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<ABeekeeperCharacter> OwningCharacter;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<UItemInstance> ItemInstance;
};
