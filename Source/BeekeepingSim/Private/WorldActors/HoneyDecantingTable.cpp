#include "WorldActors/HoneyDecantingTable.h"

#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/AnchoredFocusCursorActionComponent.h"
#include "Focus/ChildCursorPartFocusProviderComponent.h"
#include "Focus/CursorItemUseAreaScopeComponent.h"
#include "Focus/CursorPartFocusRegistrationComponent.h"
#include "Focus/CursorPartFocusScopeComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Focus/ItemUseAreaMeshProviderComponent.h"
#include "NiagaraComponent.h"
#include "WorldActors/HoneyContainerSlotActor.h"
#include "WorldActors/HoneyTransferComponent.h"

AHoneyDecantingTable::AHoneyDecantingTable()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
	TableMesh->SetupAttachment(Root);

	FocusAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FocusAnchor"));
	FocusAnchor->SetupAttachment(Root);
	FocusAnchor->ComponentTags.AddUnique(TEXT("FocusAnchor"));

	CharacterAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CharacterAnchor"));
	CharacterAnchor->SetupAttachment(Root);
	CharacterAnchor->ComponentTags.AddUnique(TEXT("CharacterAnchor"));

	FocusTarget = CreateDefaultSubobject<UFocusTargetComponent>(TEXT("FocusTarget"));
	if (FocusTarget)
	{
		FocusTarget->SetDisplayName(FText::FromString(TEXT("꿀 소분 작업대")));
	}

	FocusAction = CreateDefaultSubobject<UAnchoredFocusCursorActionComponent>(TEXT("FocusAction"));
	CursorPartFocusScope = CreateDefaultSubobject<UCursorPartFocusScopeComponent>(TEXT("CursorPartFocusScope"));
	CursorPartFocusRegistration = CreateDefaultSubobject<UCursorPartFocusRegistrationComponent>(TEXT("CursorPartFocusRegistration"));
	ChildCursorPartFocusProvider = CreateDefaultSubobject<UChildCursorPartFocusProviderComponent>(TEXT("ChildCursorPartFocusProvider"));
	ItemUseAreaScope = CreateDefaultSubobject<UCursorItemUseAreaScopeComponent>(TEXT("ItemUseAreaScope"));
	ItemUseAreaMeshProvider = CreateDefaultSubobject<UItemUseAreaMeshProviderComponent>(TEXT("ItemUseAreaMeshProvider"));

	SourceSlotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SourceSlotRoot"));
	SourceSlotRoot->SetupAttachment(Root);

	TargetSlotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TargetSlotRoot"));
	TargetSlotRoot->SetupAttachment(Root);

	SourceSlotActorClass = AHoneyContainerSlotActor::StaticClass();
	TargetSlotActorClass = AHoneyContainerSlotActor::StaticClass();

	SourceSlotChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("SourceSlotChildActor"));
	SourceSlotChildActor->SetupAttachment(SourceSlotRoot);
	SourceSlotChildActor->ComponentTags.AddUnique(TEXT("PartFocusChild"));
	SourceSlotChildActor->SetChildActorClass(SourceSlotActorClass);

	TargetSlotChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("TargetSlotChildActor"));
	TargetSlotChildActor->SetupAttachment(TargetSlotRoot);
	TargetSlotChildActor->ComponentTags.AddUnique(TEXT("PartFocusChild"));
	TargetSlotChildActor->SetChildActorClass(TargetSlotActorClass);

	HoneyTransferComponent = CreateDefaultSubobject<UHoneyTransferComponent>(TEXT("HoneyTransferComponent"));

	HoneyStreamNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HoneyStreamNiagara"));
	HoneyStreamNiagara->SetupAttachment(Root);
	HoneyStreamNiagara->SetAutoActivate(false);
	HoneyStreamNiagara->SetVisibility(false, true);
}

void AHoneyDecantingTable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureSlotChildActorClasses();
	ConfigureTransferComponent();
	RebuildCursorPartFocusDescriptors();
	RebuildItemUseAreaDescriptors();
}

void AHoneyDecantingTable::BeginPlay()
{
	Super::BeginPlay();
	EnsureSlotChildActorClasses();
	ConfigureTransferComponent();
	RebuildCursorPartFocusDescriptors();
	RebuildItemUseAreaDescriptors();
}

void AHoneyDecantingTable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HoneyTransferComponent)
	{
		HoneyTransferComponent->StopTransfer(true);
	}
	Super::EndPlay(EndPlayReason);
}

void AHoneyDecantingTable::RebuildCursorPartFocusDescriptors()
{
	if (!CursorPartFocusScope)
	{
		return;
	}

	CursorPartFocusScope->ClearRegisteredParts();
	if (CursorPartFocusRegistration)
	{
		CursorPartFocusRegistration->AppendCursorPartFocusDescriptorsToScope();
	}
}

void AHoneyDecantingTable::RebuildItemUseAreaDescriptors()
{
	if (ItemUseAreaScope)
	{
		ItemUseAreaScope->RebuildItemUseAreaDescriptors();
	}
}

AHoneyContainerSlotActor* AHoneyDecantingTable::GetSourceSlotActor() const
{
	return SourceSlotChildActor ? Cast<AHoneyContainerSlotActor>(SourceSlotChildActor->GetChildActor()) : nullptr;
}

AHoneyContainerSlotActor* AHoneyDecantingTable::GetTargetSlotActor() const
{
	return TargetSlotChildActor ? Cast<AHoneyContainerSlotActor>(TargetSlotChildActor->GetChildActor()) : nullptr;
}

void AHoneyDecantingTable::EnsureSlotChildActorClasses()
{
	if (SourceSlotChildActor)
	{
		if (!SourceSlotChildActor->ComponentHasTag(TEXT("PartFocusChild")))
		{
			SourceSlotChildActor->ComponentTags.AddUnique(TEXT("PartFocusChild"));
		}

		const TSubclassOf<AActor> ActiveSourceClass = SourceSlotActorClass
			? TSubclassOf<AActor>(SourceSlotActorClass)
			: TSubclassOf<AActor>(AHoneyContainerSlotActor::StaticClass());
		if (SourceSlotChildActor->GetChildActorClass() != ActiveSourceClass)
		{
			SourceSlotChildActor->SetChildActorClass(ActiveSourceClass);
		}
	}

	if (TargetSlotChildActor)
	{
		if (!TargetSlotChildActor->ComponentHasTag(TEXT("PartFocusChild")))
		{
			TargetSlotChildActor->ComponentTags.AddUnique(TEXT("PartFocusChild"));
		}

		const TSubclassOf<AActor> ActiveTargetClass = TargetSlotActorClass
			? TSubclassOf<AActor>(TargetSlotActorClass)
			: TSubclassOf<AActor>(AHoneyContainerSlotActor::StaticClass());
		if (TargetSlotChildActor->GetChildActorClass() != ActiveTargetClass)
		{
			TargetSlotChildActor->SetChildActorClass(ActiveTargetClass);
		}
	}

	if (AHoneyContainerSlotActor* SourceSlot = GetSourceSlotActor())
	{
		SourceSlot->SetSlotRole(EHoneyContainerSlotRole::Source);
	}

	if (AHoneyContainerSlotActor* TargetSlot = GetTargetSlotActor())
	{
		TargetSlot->SetSlotRole(EHoneyContainerSlotRole::Target);
	}
}

void AHoneyDecantingTable::ConfigureTransferComponent()
{
	if (!HoneyTransferComponent)
	{
		return;
	}

	HoneyTransferComponent->ConfigureSlots(GetSourceSlotActor(), GetTargetSlotActor());
	HoneyTransferComponent->SetHoneyStreamNiagara(HoneyStreamNiagara);
}
