// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "WorldOccupancySiteActor.generated.h"

UENUM(BlueprintType)
enum class EWorldOccupancySiteState : uint8
{
	Available,
	Reserved,
	Occupied
};

UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AWorldOccupancySiteActor : public AActor
{
	GENERATED_BODY()

public:
	AWorldOccupancySiteActor();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "World Occupancy Site")
	EWorldOccupancySiteState GetSiteState() const;

	UFUNCTION(BlueprintPure, Category = "World Occupancy Site")
	bool IsAvailable() const;

	UFUNCTION(BlueprintPure, Category = "World Occupancy Site")
	bool CanAcceptOccupant(AActor* Candidate) const;

	UFUNCTION(BlueprintCallable, Category = "World Occupancy Site")
	bool TryReserve(AActor* RequestedBy);

	UFUNCTION(BlueprintCallable, Category = "World Occupancy Site")
	bool ReleaseReservation(AActor* RequestedBy);

	UFUNCTION(BlueprintCallable, Category = "World Occupancy Site")
	bool TryOccupy(AActor* RequestedBy, AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category = "World Occupancy Site")
	bool ClearOccupant(AActor* Occupant);

	UFUNCTION(BlueprintPure, Category = "World Occupancy Site")
	FTransform GetOccupantSpawnTransform() const;

	UFUNCTION(BlueprintPure, Category = "World Occupancy Site")
	AActor* GetReservedByActor() const { return ReservedByActor; }

	UFUNCTION(BlueprintPure, Category = "World Occupancy Site")
	AActor* GetOccupyingActor() const { return OccupyingActor; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> OccupantSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Occupancy Site")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Occupancy Site")
	bool bAutoReleaseWhenOccupantDestroyed = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Occupancy Site")
	FGameplayTagContainer SiteTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Occupancy Site")
	TSubclassOf<AActor> AcceptedOccupantClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "World Occupancy Site")
	TObjectPtr<AActor> ReservedByActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "World Occupancy Site")
	TObjectPtr<AActor> OccupyingActor;

private:
	void PruneInvalidReferences();
	void BindOccupantDestroyed(AActor* Occupant);
	void UnbindOccupantDestroyed(AActor* Occupant);

	UFUNCTION()
	void HandleOccupyingActorDestroyed(AActor* DestroyedActor);
};
