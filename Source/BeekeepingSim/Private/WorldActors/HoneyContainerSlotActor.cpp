#include "WorldActors/HoneyContainerSlotActor.h"

#include "Components/PrimitiveComponent.h"
#include "Focus/CursorItemUseAreaScopeComponent.h"
#include "Focus/ItemUseAreaMeshComponent.h"
#include "Inventory/HoneyContainerItemDefinition.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "WorldActors/HoneyContainerActor.h"
#include "WorldActors/HoneyContainerRetrievePartFocusActionComponent.h"
#include "WorldActors/HoneyNozzlePartFocusActionComponent.h"
#include "WorldActors/HoneyTransferComponent.h"
#include "WorldActors/PlacementOccupantComponent.h"

AHoneyContainerSlotActor::AHoneyContainerSlotActor()
{
	ApplySlotRoleAuthoring();
}

void AHoneyContainerSlotActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplySlotRoleAuthoring();
}

void AHoneyContainerSlotActor::BeginPlay()
{
	Super::BeginPlay();
	ApplySlotRoleAuthoring();

	if (AHoneyContainerActor* ContainerActor = GetPlacedHoneyContainerActor())
	{
		ContainerActor->ApplyStateFromItemInstance(nullptr);
	}
	RequestOwningHostRefresh();
}

void AHoneyContainerSlotActor::GetCursorPartFocusDescriptors_Implementation(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const
{
	AHoneyContainerActor* ContainerActor = GetPlacedHoneyContainerActor();
	if (!ContainerActor)
	{
		return;
	}

	UPrimitiveComponent* ContainerHitComponent = ContainerActor->GetContainerMeshComponent();
	if (ContainerHitComponent && ContainerActor->GetRetrieveActionComponent())
	{
		FCursorPartFocusPartDescriptor RetrieveDescriptor;
		RetrieveDescriptor.PartId = FName(*FString::Printf(TEXT("HoneyContainer.Retrieve.%s"), *GetName()));
		RetrieveDescriptor.OwnerActor = ContainerActor;
		RetrieveDescriptor.HitComponent = ContainerHitComponent;
		RetrieveDescriptor.OutlineComponents.Add(ContainerHitComponent);
		RetrieveDescriptor.ActionHandler = ContainerActor->GetRetrieveActionComponent();
		RetrieveDescriptor.EngageMode = ECursorPartFocusEngageMode::PreviewOnly;
		RetrieveDescriptor.PromptData.bIsValid = true;
		RetrieveDescriptor.PromptData.DisplayName = ResolveContainerDisplayName(ContainerActor);
		RetrieveDescriptor.PromptData.InteractionKeyText = FText::FromString(TEXT("RMB"));
		OutDescriptors.Add(MoveTemp(RetrieveDescriptor));
	}

	if (SlotRole != EHoneyContainerSlotRole::Source || !ContainerActor->GetNozzleActionComponent())
	{
		return;
	}

	UPrimitiveComponent* NozzleHitComponent = ContainerActor->GetNozzleHitComponent();
	if (!NozzleHitComponent)
	{
		return;
	}

	FCursorPartFocusPartDescriptor NozzleDescriptor;
	NozzleDescriptor.PartId = FName(*FString::Printf(TEXT("HoneyContainer.Nozzle.%s"), *GetName()));
	NozzleDescriptor.OwnerActor = ContainerActor;
	NozzleDescriptor.HitComponent = NozzleHitComponent;
	NozzleDescriptor.OutlineComponents.Add(NozzleHitComponent);
	NozzleDescriptor.ActionHandler = ContainerActor->GetNozzleActionComponent();
	NozzleDescriptor.EngageMode = ECursorPartFocusEngageMode::InstantAction;
	NozzleDescriptor.PromptData.bIsValid = true;
	NozzleDescriptor.PromptData.DisplayName = FText::FromString(TEXT("노즐"));
	NozzleDescriptor.PromptData.InteractionKeyText = FText::FromString(TEXT("LMB"));
	OutDescriptors.Add(MoveTemp(NozzleDescriptor));
}

bool AHoneyContainerSlotActor::TryPlaceItem_Implementation(
	TSubclassOf<AActor> PlacedActorClass,
	UItemInstance* SourceItemInstance,
	ABeekeeperCharacter* InteractingCharacter)
{
	UClass* ResolvedClass = PlacedActorClass.Get();
	if (!ResolvedClass || !ResolvedClass->IsChildOf(AHoneyContainerActor::StaticClass()))
	{
		return false;
	}

	if (!CanAcceptSourceItem(SourceItemInstance))
	{
		return false;
	}

	const bool bPlaced = Super::TryPlaceItem_Implementation(PlacedActorClass, SourceItemInstance, InteractingCharacter);
	if (!bPlaced)
	{
		return false;
	}

	if (AHoneyContainerActor* ContainerActor = GetPlacedHoneyContainerActor())
	{
		ContainerActor->ApplyStateFromItemInstance(SourceItemInstance);
	}

	RequestOwningHostRefresh();
	return true;
}

void AHoneyContainerSlotActor::ClearPlacedItem_Implementation()
{
	StopOwningTransferIfActive();
	Super::ClearPlacedItem_Implementation();
	RequestOwningHostRefresh();
}

AHoneyContainerActor* AHoneyContainerSlotActor::GetPlacedHoneyContainerActor() const
{
	return Cast<AHoneyContainerActor>(GetOccupiedActor());
}

void AHoneyContainerSlotActor::SetSlotRole(EHoneyContainerSlotRole NewRole)
{
	if (SlotRole == NewRole)
	{
		return;
	}

	SlotRole = NewRole;
	ApplySlotRoleAuthoring();
	RequestOwningHostRefresh();
}

bool AHoneyContainerSlotActor::CanAcceptOccupantActor(AActor* CandidateActor) const
{
	return CandidateActor
		&& CandidateActor->IsA<AHoneyContainerActor>()
		&& Super::CanAcceptOccupantActor(CandidateActor);
}

bool AHoneyContainerSlotActor::CanAcceptSourceItem(const UItemInstance* SourceItemInstance) const
{
	const UItemDefinition* SourceDefinition = SourceItemInstance ? SourceItemInstance->GetDefinition() : nullptr;
	return DoesDefinitionMatchAcceptedQuery(SourceDefinition);
}

bool AHoneyContainerSlotActor::DoesDefinitionMatchAcceptedQuery(const UItemDefinition* ItemDefinition) const
{
	if (!Cast<UHoneyContainerItemDefinition>(ItemDefinition))
	{
		return false;
	}

	return AcceptedItemTagQuery.IsEmpty() || AcceptedItemTagQuery.Matches(ItemDefinition->GameplayTags);
}

void AHoneyContainerSlotActor::ApplySlotRoleAuthoring()
{
	if (!SlotMeshComponent)
	{
		return;
	}

	static const FName SourceAreaId(TEXT("HoneyTransfer.SourceSlot"));
	static const FName TargetAreaId(TEXT("HoneyTransfer.TargetSlot"));
	const FName CurrentAreaId = SlotMeshComponent->GetConfiguredAreaId();
	if (CurrentAreaId.IsNone() || CurrentAreaId == SourceAreaId || CurrentAreaId == TargetAreaId)
	{
		SlotMeshComponent->SetAreaId(SlotRole == EHoneyContainerSlotRole::Source ? SourceAreaId : TargetAreaId);
	}
}

void AHoneyContainerSlotActor::RequestOwningHostRefresh() const
{
	if (AActor* HostActor = GetAttachParentActor())
	{
		static const FName RebuildFuncName(TEXT("RebuildCursorPartFocusDescriptors"));
		if (UFunction* RebuildFunction = HostActor->FindFunction(RebuildFuncName))
		{
			HostActor->ProcessEvent(RebuildFunction, nullptr);
		}

		if (UCursorItemUseAreaScopeComponent* ItemUseAreaScope = HostActor->FindComponentByClass<UCursorItemUseAreaScopeComponent>())
		{
			ItemUseAreaScope->RebuildItemUseAreaDescriptors();
		}
	}
}

void AHoneyContainerSlotActor::StopOwningTransferIfActive() const
{
	AActor* HostActor = GetAttachParentActor();
	UHoneyTransferComponent* TransferComponent = HostActor ? HostActor->FindComponentByClass<UHoneyTransferComponent>() : nullptr;
	if (TransferComponent && TransferComponent->IsTransferActive())
	{
		TransferComponent->StopTransfer(true);
	}
}

FText AHoneyContainerSlotActor::ResolveContainerDisplayName(const AHoneyContainerActor* ContainerActor) const
{
	if (!ContainerActor)
	{
		return FText::GetEmpty();
	}

	if (const UPlacementOccupantComponent* Occupant = ContainerActor->GetPlacementOccupantComponent())
	{
		if (const UItemDefinition* ReturnDefinition = Occupant->GetReturnItemDefinition())
		{
			return ReturnDefinition->DisplayName;
		}
	}

	return FText::FromString(ContainerActor->GetName());
}
