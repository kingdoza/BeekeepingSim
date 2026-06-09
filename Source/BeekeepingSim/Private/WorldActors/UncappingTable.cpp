#include "WorldActors/UncappingTable.h"

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
#include "WorldActors/UncappingTableCombSlot.h"

AUncappingTable::AUncappingTable()
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
		FocusTarget->SetDisplayName(FText::FromString(TEXT("밀도 작업대")));
	}

	FocusAction = CreateDefaultSubobject<UAnchoredFocusCursorActionComponent>(TEXT("FocusAction"));
	CursorPartFocusScope = CreateDefaultSubobject<UCursorPartFocusScopeComponent>(TEXT("CursorPartFocusScope"));
	CursorPartFocusRegistration = CreateDefaultSubobject<UCursorPartFocusRegistrationComponent>(TEXT("CursorPartFocusRegistration"));
	ChildCursorPartFocusProvider = CreateDefaultSubobject<UChildCursorPartFocusProviderComponent>(TEXT("ChildCursorPartFocusProvider"));
	ItemUseAreaScope = CreateDefaultSubobject<UCursorItemUseAreaScopeComponent>(TEXT("ItemUseAreaScope"));
	ItemUseAreaMeshProvider = CreateDefaultSubobject<UItemUseAreaMeshProviderComponent>(TEXT("ItemUseAreaMeshProvider"));

	CombSlotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CombSlotRoot"));
	CombSlotRoot->SetupAttachment(Root);

	CombSlotActorClass = AUncappingTableCombSlot::StaticClass();

	CombSlotChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("CombSlotChildActor"));
	CombSlotChildActor->SetupAttachment(CombSlotRoot);
	CombSlotChildActor->ComponentTags.AddUnique(TEXT("PartFocusChild"));
	CombSlotChildActor->SetChildActorClass(CombSlotActorClass);
}

void AUncappingTable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureCombSlotChildActorClass();
	RebuildCursorPartFocusDescriptors();
	RebuildItemUseAreaDescriptors();
}

void AUncappingTable::BeginPlay()
{
	Super::BeginPlay();
	EnsureCombSlotChildActorClass();
	RebuildCursorPartFocusDescriptors();
	RebuildItemUseAreaDescriptors();
}

void AUncappingTable::RebuildCursorPartFocusDescriptors()
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

void AUncappingTable::RebuildItemUseAreaDescriptors()
{
	if (ItemUseAreaScope)
	{
		ItemUseAreaScope->RebuildItemUseAreaDescriptors();
	}
}

AUncappingTableCombSlot* AUncappingTable::GetCombSlotActor() const
{
	return CombSlotChildActor ? Cast<AUncappingTableCombSlot>(CombSlotChildActor->GetChildActor()) : nullptr;
}

void AUncappingTable::EnsureCombSlotChildActorClass()
{
	if (!CombSlotChildActor)
	{
		return;
	}

	if (!CombSlotChildActor->ComponentHasTag(TEXT("PartFocusChild")))
	{
		CombSlotChildActor->ComponentTags.AddUnique(TEXT("PartFocusChild"));
	}

	const TSubclassOf<AActor> ActiveSlotClass = CombSlotActorClass
		? TSubclassOf<AActor>(CombSlotActorClass)
		: TSubclassOf<AActor>(AUncappingTableCombSlot::StaticClass());
	if (CombSlotChildActor->GetChildActorClass() != ActiveSlotClass)
	{
		CombSlotChildActor->SetChildActorClass(ActiveSlotClass);
	}
}
