#include "WorldActors/UncappingTableCombSlot.h"

#include "Components/PrimitiveComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/CombUncappingPartFocusActionComponent.h"
#include "WorldActors/PlacementOccupantComponent.h"
#include "WorldActors/UncappingTable.h"

namespace UncappingTableCombSlotNames
{
	static const FName SlotUseAreaTag(TEXT("Item.UseArea.UncappingTable"));
}

AUncappingTableCombSlot::AUncappingTableCombSlot()
{
	CombPartFocusAction = CreateDefaultSubobject<UCombUncappingPartFocusActionComponent>(TEXT("CombPartFocusAction"));

	if (SlotMeshComponent)
	{
		SlotMeshComponent->SetAreaId(TEXT("UncappingTable.CombSlot"));

		FGameplayTagContainer SlotAreaTags;
		const FGameplayTag SlotTag = FGameplayTag::RequestGameplayTag(UncappingTableCombSlotNames::SlotUseAreaTag, false);
		if (SlotTag.IsValid())
		{
			SlotAreaTags.AddTag(SlotTag);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Missing gameplay tag '%s' for uncapping table comb slot."),
				*UncappingTableCombSlotNames::SlotUseAreaTag.ToString());
		}
		SlotMeshComponent->SetAreaTags(SlotAreaTags);
	}
}

ABeehiveCombActor* AUncappingTableCombSlot::GetPlacedCombActor() const
{
	return Cast<ABeehiveCombActor>(GetOccupiedActor());
}

void AUncappingTableCombSlot::GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const
{
	ABeehiveCombActor* CombActor = GetPlacedCombActor();
	if (!CombActor)
	{
		return;
	}

	UPrimitiveComponent* CombMesh = CombActor->GetCombMeshComponent();
	if (!CombMesh)
	{
		return;
	}

	FCursorPartFocusPartDescriptor Descriptor;
	Descriptor.PartId = FName(TEXT("UncappingTable.Comb"));
	Descriptor.OwnerActor = CombActor;
	Descriptor.HitComponent = CombMesh;
	Descriptor.OutlineComponents.Add(CombMesh);
	Descriptor.ActionHandler = CombPartFocusAction;
	Descriptor.EngageMode = CombPartFocusAction ? CombPartFocusAction->GetEngageMode() : ECursorPartFocusEngageMode::PersistentAction;
	Descriptor.PromptData.bIsValid = true;

	FText DisplayName = FText::FromString(TEXT("소비장"));
	if (const UPlacementOccupantComponent* Occupant = CombActor->GetPlacementOccupantComponent())
	{
		if (const UItemDefinition* ReturnDefinition = Occupant->GetReturnItemDefinition())
		{
			DisplayName = ReturnDefinition->DisplayName;
		}
	}
	Descriptor.PromptData.DisplayName = DisplayName;
	Descriptor.PromptData.InteractionKeyText = FText::FromString(TEXT("LMB"));
	OutDescriptors.Add(MoveTemp(Descriptor));
}

bool AUncappingTableCombSlot::TryPlaceItem_Implementation(
	TSubclassOf<AActor> PlacedActorClass,
	UItemInstance* SourceItemInstance,
	ABeekeeperCharacter* InteractingCharacter)
{
	UClass* ResolvedClass = PlacedActorClass.Get();
	if (!ResolvedClass || !ResolvedClass->IsChildOf(ABeehiveCombActor::StaticClass()))
	{
		return false;
	}

	const bool bPlaced = Super::TryPlaceItem_Implementation(PlacedActorClass, SourceItemInstance, InteractingCharacter);
	if (!bPlaced)
	{
		return false;
	}

	if (ABeehiveCombActor* CombActor = GetPlacedCombActor())
	{
		CombActor->ApplyStateFromItemInstance(SourceItemInstance);
	}

	RequestOwningUncappingTableRefresh();
	return true;
}

void AUncappingTableCombSlot::ClearPlacedItem_Implementation()
{
	Super::ClearPlacedItem_Implementation();
	RequestOwningUncappingTableRefresh();
}

void AUncappingTableCombSlot::BeginPlay()
{
	Super::BeginPlay();
	RequestOwningUncappingTableRefresh();
}

bool AUncappingTableCombSlot::CanAcceptOccupantActor(AActor* CandidateActor) const
{
	return CandidateActor
		&& CandidateActor->IsA<ABeehiveCombActor>()
		&& Super::CanAcceptOccupantActor(CandidateActor);
}

void AUncappingTableCombSlot::RequestOwningUncappingTableRefresh() const
{
	if (AUncappingTable* Table = Cast<AUncappingTable>(GetAttachParentActor()))
	{
		Table->RebuildCursorPartFocusDescriptors();
		Table->RebuildItemUseAreaDescriptors();
	}
}
