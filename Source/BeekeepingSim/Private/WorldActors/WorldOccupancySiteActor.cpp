// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldActors/WorldOccupancySiteActor.h"

#include "Components/SceneComponent.h"

AWorldOccupancySiteActor::AWorldOccupancySiteActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	OccupantSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("OccupantSpawnPoint"));
	OccupantSpawnPoint->SetupAttachment(Root);
}

void AWorldOccupancySiteActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindOccupantDestroyed(OccupyingActor);
	Super::EndPlay(EndPlayReason);
}

EWorldOccupancySiteState AWorldOccupancySiteActor::GetSiteState() const
{
	if (IsValid(OccupyingActor))
	{
		return EWorldOccupancySiteState::Occupied;
	}

	if (IsValid(ReservedByActor))
	{
		return EWorldOccupancySiteState::Reserved;
	}

	return EWorldOccupancySiteState::Available;
}

bool AWorldOccupancySiteActor::IsAvailable() const
{
	return bEnabled && GetSiteState() == EWorldOccupancySiteState::Available;
}

bool AWorldOccupancySiteActor::CanAcceptOccupant(AActor* Candidate) const
{
	if (!IsValid(Candidate))
	{
		return false;
	}

	return !AcceptedOccupantClass || Candidate->IsA(AcceptedOccupantClass);
}

bool AWorldOccupancySiteActor::TryReserve(AActor* RequestedBy)
{
	PruneInvalidReferences();

	if (!IsValid(RequestedBy) || !IsAvailable())
	{
		return false;
	}

	ReservedByActor = RequestedBy;
	return true;
}

bool AWorldOccupancySiteActor::ReleaseReservation(AActor* RequestedBy)
{
	PruneInvalidReferences();

	if (!IsValid(RequestedBy) || ReservedByActor != RequestedBy || IsValid(OccupyingActor))
	{
		return false;
	}

	ReservedByActor = nullptr;
	return true;
}

bool AWorldOccupancySiteActor::TryOccupy(AActor* RequestedBy, AActor* Occupant)
{
	PruneInvalidReferences();

	if (!bEnabled || !IsValid(RequestedBy) || !CanAcceptOccupant(Occupant))
	{
		return false;
	}

	const bool bReservedByRequester = IsValid(ReservedByActor) && ReservedByActor == RequestedBy;
	if (!IsAvailable() && !bReservedByRequester)
	{
		return false;
	}

	ReservedByActor = nullptr;
	OccupyingActor = Occupant;
	BindOccupantDestroyed(OccupyingActor);
	return true;
}

bool AWorldOccupancySiteActor::ClearOccupant(AActor* Occupant)
{
	if (!Occupant || OccupyingActor != Occupant)
	{
		return false;
	}

	UnbindOccupantDestroyed(Occupant);
	OccupyingActor = nullptr;
	return true;
}

FTransform AWorldOccupancySiteActor::GetOccupantSpawnTransform() const
{
	return OccupantSpawnPoint ? OccupantSpawnPoint->GetComponentTransform() : GetActorTransform();
}

void AWorldOccupancySiteActor::PruneInvalidReferences()
{
	if (!IsValid(ReservedByActor))
	{
		ReservedByActor = nullptr;
	}

	if (!IsValid(OccupyingActor))
	{
		OccupyingActor = nullptr;
	}
}

void AWorldOccupancySiteActor::BindOccupantDestroyed(AActor* Occupant)
{
	if (!bAutoReleaseWhenOccupantDestroyed || !Occupant)
	{
		return;
	}

	Occupant->OnDestroyed.RemoveDynamic(this, &AWorldOccupancySiteActor::HandleOccupyingActorDestroyed);
	Occupant->OnDestroyed.AddDynamic(this, &AWorldOccupancySiteActor::HandleOccupyingActorDestroyed);
}

void AWorldOccupancySiteActor::UnbindOccupantDestroyed(AActor* Occupant)
{
	if (Occupant)
	{
		Occupant->OnDestroyed.RemoveDynamic(this, &AWorldOccupancySiteActor::HandleOccupyingActorDestroyed);
	}
}

void AWorldOccupancySiteActor::HandleOccupyingActorDestroyed(AActor* DestroyedActor)
{
	if (OccupyingActor != DestroyedActor)
	{
		return;
	}

	UnbindOccupantDestroyed(DestroyedActor);
	OccupyingActor = nullptr;
}
