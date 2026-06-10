#include "WorldActors/HoneyNozzlePartFocusActionComponent.h"

#include "WorldActors/HoneyContainerActor.h"
#include "WorldActors/HoneyContainerSlotActor.h"
#include "WorldActors/HoneyTransferComponent.h"
#include "WorldActors/PlacementOccupantComponent.h"

UHoneyNozzlePartFocusActionComponent::UHoneyNozzlePartFocusActionComponent()
{
	SetEngageMode(ECursorPartFocusEngageMode::InstantAction);
	SetPrimaryPromptActionText(FText::FromString(TEXT("배출")));
	SetEngagedPrimaryPromptActionText(FText::FromString(TEXT("정지")));
}

bool UHoneyNozzlePartFocusActionComponent::CanBeginPartFocusAction(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter) const
{
	(void)ScopeComponent;
	(void)InteractingCharacter;
	return CanToggleTransfer();
}

bool UHoneyNozzlePartFocusActionComponent::BeginPartFocusAction(
	UCursorPartFocusScopeComponent* ScopeComponent,
	ABeekeeperCharacter* InteractingCharacter)
{
	(void)ScopeComponent;
	(void)InteractingCharacter;

	AHoneyContainerActor* SourceContainer = nullptr;
	AHoneyContainerSlotActor* SourceSlot = nullptr;
	UHoneyTransferComponent* TransferComponent = nullptr;
	if (!ResolveTransferContext(SourceContainer, SourceSlot, TransferComponent))
	{
		return false;
	}

	return TransferComponent->ToggleTransferFromNozzle(SourceContainer);
}

FText UHoneyNozzlePartFocusActionComponent::ResolvePrimaryPromptActionText() const
{
	AHoneyContainerActor* SourceContainer = nullptr;
	AHoneyContainerSlotActor* SourceSlot = nullptr;
	UHoneyTransferComponent* TransferComponent = nullptr;
	if (ResolveTransferContext(SourceContainer, SourceSlot, TransferComponent)
		&& TransferComponent->IsTransferActive()
		&& TransferComponent->GetActiveSourceContainer() == SourceContainer)
	{
		return GetEngagedPrimaryPromptActionText();
	}

	return GetPrimaryPromptActionText();
}

bool UHoneyNozzlePartFocusActionComponent::ResolveTransferContext(
	AHoneyContainerActor*& OutSourceContainer,
	AHoneyContainerSlotActor*& OutSourceSlot,
	UHoneyTransferComponent*& OutTransferComponent) const
{
	OutSourceContainer = Cast<AHoneyContainerActor>(GetOwner());
	OutSourceSlot = nullptr;
	OutTransferComponent = nullptr;
	if (!OutSourceContainer)
	{
		return false;
	}

	const UPlacementOccupantComponent* Occupant = OutSourceContainer->GetPlacementOccupantComponent();
	OutSourceSlot = Occupant ? Cast<AHoneyContainerSlotActor>(Occupant->GetOwningPlacementSlotActor()) : nullptr;
	if (!OutSourceSlot || OutSourceSlot->GetSlotRole() != EHoneyContainerSlotRole::Source)
	{
		return false;
	}

	if (OutSourceSlot->GetPlacedHoneyContainerActor() != OutSourceContainer)
	{
		return false;
	}

	TArray<AActor*> CandidateHosts;
	CandidateHosts.Add(OutSourceSlot->GetAttachParentActor());
	CandidateHosts.Add(OutSourceSlot->GetOwner());
	CandidateHosts.Add(OutSourceSlot);
	for (AActor* CandidateHost : CandidateHosts)
	{
		if (!CandidateHost)
		{
			continue;
		}

		OutTransferComponent = CandidateHost->FindComponentByClass<UHoneyTransferComponent>();
		if (OutTransferComponent)
		{
			break;
		}
	}

	return OutTransferComponent != nullptr;
}

bool UHoneyNozzlePartFocusActionComponent::CanToggleTransfer() const
{
	AHoneyContainerActor* SourceContainer = nullptr;
	AHoneyContainerSlotActor* SourceSlot = nullptr;
	UHoneyTransferComponent* TransferComponent = nullptr;
	if (!ResolveTransferContext(SourceContainer, SourceSlot, TransferComponent))
	{
		return false;
	}

	if (TransferComponent->IsTransferActive())
	{
		return TransferComponent->GetActiveSourceContainer() == SourceContainer;
	}

	return TransferComponent->CanStartTransfer();
}
