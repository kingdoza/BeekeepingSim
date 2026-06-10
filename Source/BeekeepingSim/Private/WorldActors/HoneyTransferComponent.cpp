#include "WorldActors/HoneyTransferComponent.h"

#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "WorldActors/HoneyContainerActor.h"
#include "WorldActors/HoneyContainerSlotActor.h"

UHoneyTransferComponent::UHoneyTransferComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UHoneyTransferComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopTransfer(true);
	Super::EndPlay(EndPlayReason);
}

void UHoneyTransferComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TransferState == EHoneyTransferState::Idle)
	{
		return;
	}

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	switch (TransferState)
	{
	case EHoneyTransferState::GrowingDrop:
		TickGrowingDrop(DeltaTime);
		break;
	case EHoneyTransferState::Transferring:
		TickTransfer(DeltaTime);
		break;
	default:
		break;
	}
}

void UHoneyTransferComponent::ConfigureSlots(AHoneyContainerSlotActor* SourceSlot, AHoneyContainerSlotActor* TargetSlot)
{
	ConfiguredSourceSlot = SourceSlot;
	ConfiguredTargetSlot = TargetSlot;
	if (TransferState != EHoneyTransferState::Idle && !ValidateActiveTransfer())
	{
		StopTransfer(true);
	}
}

void UHoneyTransferComponent::SetHoneyStreamNiagara(UNiagaraComponent* NiagaraComponent)
{
	LegacyHoneyStreamNiagara = NiagaraComponent;
	if (LegacyHoneyStreamNiagara && TransferState == EHoneyTransferState::Idle)
	{
		DeactivateHoneyStream(true);
	}
}

bool UHoneyTransferComponent::CanStartTransfer() const
{
	AHoneyContainerActor* SourceContainer = nullptr;
	AHoneyContainerActor* TargetContainer = nullptr;
	return ValidateTransferConfiguration(SourceContainer, TargetContainer);
}

bool UHoneyTransferComponent::StartTransfer()
{
	AHoneyContainerActor* SourceContainer = nullptr;
	AHoneyContainerActor* TargetContainer = nullptr;
	if (!ValidateTransferConfiguration(SourceContainer, TargetContainer))
	{
		return false;
	}

	ActiveSourceContainer = SourceContainer;
	ActiveTargetContainer = TargetContainer;
	CurrentDropLengthCm = 0.0f;
	TargetDropLengthCm = ResolveTargetDropLengthCm();

	ApplyNiagaraTransferParameters(CurrentDropLengthCm);
	ActivateHoneyStream();
	TransferState = EHoneyTransferState::GrowingDrop;
	return true;
}

void UHoneyTransferComponent::StopTransfer(bool bImmediateVfx)
{
	if (TransferState == EHoneyTransferState::Idle)
	{
		ApplyDropLengthParameter(0.0f);
		DeactivateHoneyStream(bImmediateVfx);
		return;
	}

	TransferState = EHoneyTransferState::Idle;
	CurrentDropLengthCm = 0.0f;
	TargetDropLengthCm = 0.0f;
	ApplyDropLengthParameter(0.0f);
	DeactivateHoneyStream(bImmediateVfx);
	ActiveSourceContainer = nullptr;
	ActiveTargetContainer = nullptr;
}

bool UHoneyTransferComponent::ToggleTransferFromNozzle(AHoneyContainerActor* SourceContainer)
{
	if (!SourceContainer)
	{
		return false;
	}

	if (TransferState != EHoneyTransferState::Idle)
	{
		if (ActiveSourceContainer == SourceContainer)
		{
			StopTransfer(true);
			return true;
		}

		return false;
	}

	AHoneyContainerActor* ConfiguredSourceContainer = nullptr;
	AHoneyContainerActor* ConfiguredTargetContainer = nullptr;
	if (!ValidateTransferConfiguration(ConfiguredSourceContainer, ConfiguredTargetContainer))
	{
		return false;
	}

	if (ConfiguredSourceContainer != SourceContainer)
	{
		return false;
	}

	return StartTransfer();
}

bool UHoneyTransferComponent::ResolveConfiguredContainers(
	AHoneyContainerActor*& OutSourceContainer,
	AHoneyContainerActor*& OutTargetContainer) const
{
	OutSourceContainer = ConfiguredSourceSlot ? ConfiguredSourceSlot->GetPlacedHoneyContainerActor() : nullptr;
	OutTargetContainer = ConfiguredTargetSlot ? ConfiguredTargetSlot->GetPlacedHoneyContainerActor() : nullptr;
	return OutSourceContainer && OutTargetContainer;
}

bool UHoneyTransferComponent::ValidateTransferConfiguration(
	AHoneyContainerActor*& OutSourceContainer,
	AHoneyContainerActor*& OutTargetContainer) const
{
	OutSourceContainer = nullptr;
	OutTargetContainer = nullptr;

	if (!ConfiguredSourceSlot || !ConfiguredTargetSlot)
	{
		return false;
	}

	if (ConfiguredSourceSlot->GetSlotRole() != EHoneyContainerSlotRole::Source
		|| ConfiguredTargetSlot->GetSlotRole() != EHoneyContainerSlotRole::Target)
	{
		return false;
	}

	if (!ResolveConfiguredContainers(OutSourceContainer, OutTargetContainer))
	{
		return false;
	}

	return OutSourceContainer->GetCurrentVolumeMl() > 0.0f
		&& OutTargetContainer->GetFreeVolumeMl() > 0.0f;
}

bool UHoneyTransferComponent::ValidateActiveTransfer() const
{
	if (!ConfiguredSourceSlot || !ConfiguredTargetSlot || !IsValid(ActiveSourceContainer) || !IsValid(ActiveTargetContainer))
	{
		return false;
	}

	if (ConfiguredSourceSlot->GetPlacedHoneyContainerActor() != ActiveSourceContainer
		|| ConfiguredTargetSlot->GetPlacedHoneyContainerActor() != ActiveTargetContainer)
	{
		return false;
	}

	if (ConfiguredSourceSlot->GetSlotRole() != EHoneyContainerSlotRole::Source
		|| ConfiguredTargetSlot->GetSlotRole() != EHoneyContainerSlotRole::Target)
	{
		return false;
	}

	return ActiveSourceContainer->GetCurrentVolumeMl() > 0.0f
		&& ActiveTargetContainer->GetFreeVolumeMl() > 0.0f;
}

float UHoneyTransferComponent::ResolveTargetDropLengthCm() const
{
	const UNiagaraComponent* SourceHoneyStream = IsValid(ActiveSourceContainer) ? ActiveSourceContainer->GetHoneyStreamNiagaraComponent() : nullptr;
	const USceneComponent* TargetPourTarget = IsValid(ActiveTargetContainer) ? ActiveTargetContainer->GetPourTargetComponent() : nullptr;
	if (!TargetPourTarget && ConfiguredTargetSlot)
	{
		TargetPourTarget = ConfiguredTargetSlot->GetSlotPourTargetComponent();
	}

	if (!SourceHoneyStream || !TargetPourTarget)
	{
		return FMath::Max(0.0f, DefaultDropLengthCm);
	}

	const float SourceStreamZ = SourceHoneyStream->GetComponentLocation().Z;
	const float TargetPourTargetZ = TargetPourTarget->GetComponentLocation().Z;
	return FMath::Max(0.0f, SourceStreamZ - TargetPourTargetZ);
}

UNiagaraComponent* UHoneyTransferComponent::ResolveActiveHoneyStreamNiagara() const
{
	if (IsValid(ActiveSourceContainer))
	{
		if (UNiagaraComponent* SourceHoneyStream = ActiveSourceContainer->GetHoneyStreamNiagaraComponent())
		{
			return SourceHoneyStream;
		}
	}

	return LegacyHoneyStreamNiagara;
}

void UHoneyTransferComponent::ApplyNiagaraTransferParameters(float DropLengthCm) const
{
	UNiagaraComponent* HoneyStreamNiagara = ResolveActiveHoneyStreamNiagara();
	if (!HoneyStreamNiagara || !IsValid(ActiveSourceContainer))
	{
		return;
	}

	HoneyStreamNiagara->SetVariableFloat(HoneyDensityNiagaraParameterName, ActiveSourceContainer->GetHoneyDensity());
	const float SourceRipeness = (ActiveSourceContainer->GetHoneyDensity() < 1.0f) ? 0.0f : ActiveSourceContainer->GetHoneyRipeness();
	HoneyStreamNiagara->SetVariableFloat(HoneyRipenessNiagaraParameterName, SourceRipeness);
	ApplyDropLengthParameter(DropLengthCm);
}

void UHoneyTransferComponent::ApplyDropLengthParameter(float DropLengthCm) const
{
	if (UNiagaraComponent* HoneyStreamNiagara = ResolveActiveHoneyStreamNiagara())
	{
		HoneyStreamNiagara->SetVariableFloat(DropLengthNiagaraParameterName, FMath::Max(0.0f, DropLengthCm));
	}
}

void UHoneyTransferComponent::ActivateHoneyStream() const
{
	UNiagaraComponent* HoneyStreamNiagara = ResolveActiveHoneyStreamNiagara();
	if (!HoneyStreamNiagara)
	{
		return;
	}

	HoneyStreamNiagara->SetVisibility(true, true);
	HoneyStreamNiagara->Activate(true);
}

void UHoneyTransferComponent::DeactivateHoneyStream(bool bImmediateVfx) const
{
	UNiagaraComponent* HoneyStreamNiagara = ResolveActiveHoneyStreamNiagara();
	if (!HoneyStreamNiagara)
	{
		return;
	}

	if (bImmediateVfx)
	{
		HoneyStreamNiagara->DeactivateImmediate();
	}
	else
	{
		HoneyStreamNiagara->Deactivate();
	}
	HoneyStreamNiagara->SetVisibility(false, true);
}

void UHoneyTransferComponent::TickGrowingDrop(float DeltaTime)
{
	if (!ValidateActiveTransfer())
	{
		StopTransfer(true);
		return;
	}

	const float GrowDelta = FMath::Max(0.0f, DropLengthGrowSpeedCmPerSecond) * DeltaTime;
	CurrentDropLengthCm = FMath::Min(TargetDropLengthCm, CurrentDropLengthCm + GrowDelta);
	ApplyNiagaraTransferParameters(CurrentDropLengthCm);

	if (CurrentDropLengthCm >= TargetDropLengthCm - KINDA_SMALL_NUMBER)
	{
		if (!ValidateActiveTransfer())
		{
			StopTransfer(true);
			return;
		}

		TransferState = EHoneyTransferState::Transferring;
	}
}

void UHoneyTransferComponent::TickTransfer(float DeltaTime)
{
	if (!ValidateActiveTransfer())
	{
		StopTransfer(true);
		return;
	}

	const float RequestedMoveAmount = FMath::Max(0.0f, TransferRateMlPerSecond) * DeltaTime;
	const float MoveAmountMl = FMath::Min(
		FMath::Min(RequestedMoveAmount, ActiveSourceContainer->GetCurrentVolumeMl()),
		ActiveTargetContainer->GetFreeVolumeMl());
	if (MoveAmountMl <= 0.0f)
	{
		StopTransfer(true);
		return;
	}

	const float IncomingDensity = ActiveSourceContainer->GetHoneyDensity();
	const float IncomingRipeness = ActiveSourceContainer->GetHoneyRipeness();
	ActiveSourceContainer->RemoveHoneyVolume(MoveAmountMl);
	ActiveTargetContainer->AddHoneyVolume(MoveAmountMl, IncomingDensity, IncomingRipeness);
	ApplyNiagaraTransferParameters(TargetDropLengthCm);

	if (!ValidateActiveTransfer())
	{
		StopTransfer(true);
	}
}
