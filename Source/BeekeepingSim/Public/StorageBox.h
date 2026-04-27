#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StorageBox.generated.h"

class UFocusTargetComponent;
class USceneComponent;
class UStaticMeshComponent;
class UStorageBoxComponent;
class UStorageBoxFocusActionComponent;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API AStorageBox : public AActor
{
	GENERATED_BODY()

public:
	AStorageBox();

	UFUNCTION(BlueprintPure, Category = "Storage")
	UStorageBoxComponent* GetStorageBoxComponent() const { return StorageBoxComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFocusTargetComponent> FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStorageBoxComponent> StorageBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStorageBoxFocusActionComponent> StorageBoxFocusAction;
};
