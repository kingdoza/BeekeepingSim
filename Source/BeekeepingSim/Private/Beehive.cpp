// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Beehive.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Public/FocusTargetComponent.h"

ABeehive::ABeehive()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BeehiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeehiveMesh"));
	BeehiveMesh->SetupAttachment(Root);

	FocusTarget = CreateDefaultSubobject<UFocusTargetComponent>(TEXT("FocusTarget"));
}

void ABeehive::OnFocusEnter_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	bIsLidOpen = true;
	ReceiveFocusEntered(InteractingCharacter);
}

void ABeehive::OnFocusExit_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	bIsLidOpen = false;
	ReceiveFocusExited(InteractingCharacter);
}

void ABeehive::OnFocusConfirm_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	ReceiveFocusConfirmed(InteractingCharacter);
}

void ABeehive::OnFocusCancel_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	ReceiveFocusCanceled(InteractingCharacter);
}
