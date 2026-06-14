#include "Inventory/ItemInstance.h"

#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Inventory/BeeCarrierItemDefinition.h"
#include "Inventory/HoneyContainerItemDefinition.h"
#include "Inventory/HoldItemUseAction.h"
#include "Inventory/ItemAction.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/QueenCageItemDefinition.h"
#include "WorldActors/QueenBeeActor.h"

void UItemInstance::InitializeFromDefinition(UItemDefinition* InDefinition, int32 InStackCount, float InDurability)
{
	Definition = InDefinition;
	InstanceId = FGuid::NewGuid();
	StackCount = 0;
	if (Definition && Definition->bUsesDurability)
	{
		const float MaxDurability = FMath::Max(0.0f, Definition->MaxDurability);
		if (InDurability < 0.0f)
		{
			Durability = MaxDurability;
		}
		else
		{
			Durability = FMath::Clamp(InDurability, 0.0f, MaxDurability);
		}
	}
	else
	{
		Durability = (InDurability < 0.0f) ? 1.0f : InDurability;
	}
	SetStackCount(InStackCount);
	ClearBeehiveCombState();
	ClearHoneyContainerState();
	ClearBeeCarrierState();
	ClearQueenCageState();
	if (const UHoneyContainerItemDefinition* HoneyContainerDefinition = Cast<UHoneyContainerItemDefinition>(Definition))
	{
		const float SanitizedMaxVolume = FMath::Max(0.0f, HoneyContainerDefinition->MaxVolumeMl);
		const float SanitizedCurrentVolume = FMath::Clamp(HoneyContainerDefinition->DefaultCurrentVolumeMl, 0.0f, SanitizedMaxVolume);
		SetHoneyContainerState(
			SanitizedCurrentVolume,
			HoneyContainerDefinition->DefaultHoneyDensity,
			HoneyContainerDefinition->DefaultHoneyRipeness);
	}
	if (const UBeeCarrierItemDefinition* BeeCarrierDefinition = Cast<UBeeCarrierItemDefinition>(Definition))
	{
		SetBeeCarrierState(BeeCarrierDefinition->DefaultCapturedBeeAmount);
	}
	if (Cast<UQueenCageItemDefinition>(Definition))
	{
		SetQueenCageEmptyState();
	}
	RebuildActions();
}

FText UItemInstance::GetDisplayName() const
{
	return Definition ? Definition->DisplayName : FText::GetEmpty();
}

FText UItemInstance::GetDescription() const
{
	return Definition ? Definition->Description : FText::GetEmpty();
}

UTexture2D* UItemInstance::GetIcon() const
{
	return Definition ? Definition->Icon : nullptr;
}

UStaticMesh* UItemInstance::GetWorldMesh() const
{
	return Definition ? Definition->WorldMesh : nullptr;
}

TSubclassOf<AItemPresentationActor> UItemInstance::GetHeldPresentationActorClass() const
{
	return Definition ? Definition->HeldPresentationActorClass : nullptr;
}

void UItemInstance::SetStackCount(int32 NewStackCount)
{
	const bool bForceSingleStack = Cast<UHoneyContainerItemDefinition>(Definition)
		|| Cast<UBeeCarrierItemDefinition>(Definition)
		|| Cast<UQueenCageItemDefinition>(Definition);
	const int32 MaxStack = bForceSingleStack ? 1 : (Definition ? FMath::Max(1, Definition->MaxStack) : 1);
	StackCount = FMath::Clamp(NewStackCount, 0, MaxStack);
}

void UItemInstance::SetDurability(float NewDurability)
{
	if (HasDurability())
	{
		Durability = FMath::Clamp(NewDurability, 0.0f, GetMaxDurability());
		return;
	}

	Durability = NewDurability;
}

void UItemInstance::SetBeehiveCombState(float HoneyAmount, bool bIsFrontFaceVisible)
{
	SetBeehiveCombStateWithRipeness(HoneyAmount, 0.0f, bIsFrontFaceVisible);
}

void UItemInstance::SetBeehiveCombStateWithRipeness(float HoneyAmount, float HoneyRipeness, bool bIsFrontFaceVisible)
{
	BeehiveCombState.bHasState = true;
	BeehiveCombState.HoneyAmount = FMath::Max(0.0f, HoneyAmount);
	BeehiveCombState.HoneyRipeness = FMath::Max(0.0f, HoneyRipeness);
	BeehiveCombState.bIsFrontFaceVisible = bIsFrontFaceVisible;
	BeehiveCombState.CappingMaskWidth = 0;
	BeehiveCombState.CappingMaskHeight = 0;
	BeehiveCombState.FrontWaxCappingMask.Reset();
	BeehiveCombState.BackWaxCappingMask.Reset();
}

void UItemInstance::SetBeehiveCombStateWithCapping(
	float HoneyAmount,
	float HoneyRipeness,
	bool bIsFrontFaceVisible,
	int32 CappingMaskWidth,
	int32 CappingMaskHeight,
	const TArray<uint8>& FrontWaxCappingMask,
	const TArray<uint8>& BackWaxCappingMask)
{
	BeehiveCombState.bHasState = true;
	BeehiveCombState.HoneyAmount = FMath::Max(0.0f, HoneyAmount);
	BeehiveCombState.HoneyRipeness = FMath::Max(0.0f, HoneyRipeness);
	BeehiveCombState.bIsFrontFaceVisible = bIsFrontFaceVisible;
	BeehiveCombState.CappingMaskWidth = FMath::Max(0, CappingMaskWidth);
	BeehiveCombState.CappingMaskHeight = FMath::Max(0, CappingMaskHeight);
	BeehiveCombState.FrontWaxCappingMask = FrontWaxCappingMask;
	BeehiveCombState.BackWaxCappingMask = BackWaxCappingMask;
}

void UItemInstance::ClearBeehiveCombState()
{
	BeehiveCombState = FBeehiveCombItemState();
}

bool UItemInstance::HasBeehiveCombState() const
{
	return BeehiveCombState.bHasState;
}

void UItemInstance::SetHoneyContainerState(float CurrentVolumeMl, float HoneyDensity, float HoneyRipeness)
{
	const UHoneyContainerItemDefinition* HoneyContainerDefinition = Cast<UHoneyContainerItemDefinition>(Definition);
	if (!HoneyContainerDefinition)
	{
		ClearHoneyContainerState();
		return;
	}

	const float SanitizedMaxVolume = FMath::Max(0.0f, HoneyContainerDefinition->MaxVolumeMl);
	const float SanitizedDensity = FMath::Clamp(HoneyDensity, 0.0f, 1.0f);
	const float SanitizedRipeness = (SanitizedDensity < 1.0f)
		? 0.0f
		: FMath::Clamp(HoneyRipeness, 0.0f, 1.0f);

	HoneyContainerState.bHasState = true;
	HoneyContainerState.CurrentVolumeMl = FMath::Clamp(CurrentVolumeMl, 0.0f, SanitizedMaxVolume);
	HoneyContainerState.HoneyDensity = SanitizedDensity;
	HoneyContainerState.HoneyRipeness = SanitizedRipeness;
}

void UItemInstance::ClearHoneyContainerState()
{
	HoneyContainerState = FHoneyContainerItemState();
}

bool UItemInstance::HasHoneyContainerState() const
{
	return HoneyContainerState.bHasState;
}

void UItemInstance::SetBeeCarrierState(float CapturedBeeAmount)
{
	const UBeeCarrierItemDefinition* BeeCarrierDefinition = Cast<UBeeCarrierItemDefinition>(Definition);
	if (!BeeCarrierDefinition)
	{
		ClearBeeCarrierState();
		return;
	}

	const float MaxCapturedBeeAmount = FMath::Max(0.0f, BeeCarrierDefinition->MaxCapturedBeeAmount);
	BeeCarrierState.bHasState = true;
	BeeCarrierState.CapturedBeeAmount = FMath::Clamp(CapturedBeeAmount, 0.0f, MaxCapturedBeeAmount);
}

float UItemInstance::AddCapturedBees(float BeeAmount)
{
	const float OldCapturedBeeAmount = GetCapturedBeeAmount();
	SetBeeCarrierState(OldCapturedBeeAmount + FMath::Max(0.0f, BeeAmount));
	return FMath::Max(0.0f, GetCapturedBeeAmount() - OldCapturedBeeAmount);
}

void UItemInstance::ClearBeeCarrierState()
{
	BeeCarrierState = FBeeCarrierItemState();
}

bool UItemInstance::HasBeeCarrierState() const
{
	return BeeCarrierState.bHasState;
}

float UItemInstance::GetCapturedBeeAmount() const
{
	return HasBeeCarrierState() ? FMath::Max(0.0f, BeeCarrierState.CapturedBeeAmount) : 0.0f;
}

int32 UItemInstance::GetCapturedBeeCountRounded() const
{
	return FMath::Max(0, FMath::RoundToInt(GetCapturedBeeAmount()));
}

float UItemInstance::GetBeeCarrierFreeCapacity() const
{
	const UBeeCarrierItemDefinition* BeeCarrierDefinition = Cast<UBeeCarrierItemDefinition>(Definition);
	if (!BeeCarrierDefinition)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, FMath::Max(0.0f, BeeCarrierDefinition->MaxCapturedBeeAmount) - GetCapturedBeeAmount());
}

void UItemInstance::SetQueenCageEmptyState()
{
	if (!Cast<UQueenCageItemDefinition>(Definition))
	{
		ClearQueenCageState();
		return;
	}

	QueenCageState = FQueenCageItemState();
	QueenCageState.bHasState = true;
}

void UItemInstance::SetQueenCageState(const FQueenCageItemState& NewState)
{
	if (!Cast<UQueenCageItemDefinition>(Definition))
	{
		ClearQueenCageState();
		return;
	}

	if (!NewState.bHasState || !NewState.bHasQueen || !NewState.CapturedQueenBeeClass)
	{
		SetQueenCageEmptyState();
		return;
	}

	QueenCageState.bHasState = true;
	QueenCageState.bHasQueen = true;
	QueenCageState.CapturedQueenBeeClass = NewState.CapturedQueenBeeClass;
	QueenCageState.BaseEggLayingPower = FMath::Max(0.0f, NewState.BaseEggLayingPower);
	QueenCageState.DiseaseValue = FMath::Clamp(NewState.DiseaseValue, 0.0f, 1.0f);
}

void UItemInstance::SetCapturedQueenBeeState(TSubclassOf<AQueenBeeActor> QueenClass, float BaseEggLayingPower, float DiseaseValue)
{
	FQueenCageItemState NewState;
	NewState.bHasState = true;
	NewState.bHasQueen = QueenClass != nullptr;
	NewState.CapturedQueenBeeClass = QueenClass;
	NewState.BaseEggLayingPower = BaseEggLayingPower;
	NewState.DiseaseValue = DiseaseValue;
	SetQueenCageState(NewState);
}

void UItemInstance::ClearQueenCageState()
{
	QueenCageState = FQueenCageItemState();
}

bool UItemInstance::HasQueenCageState() const
{
	return QueenCageState.bHasState;
}

bool UItemInstance::HasCapturedQueen() const
{
	return HasQueenCageState() && QueenCageState.bHasQueen && QueenCageState.CapturedQueenBeeClass;
}

bool UItemInstance::CanAcceptQueenBee() const
{
	return Cast<UQueenCageItemDefinition>(Definition) && HasQueenCageState() && !HasCapturedQueen();
}

void UItemInstance::CopyRuntimeStateFrom(const UItemInstance* SourceItemInstance)
{
	if (!SourceItemInstance)
	{
		ClearBeehiveCombState();
		ClearHoneyContainerState();
		ClearBeeCarrierState();
		ClearQueenCageState();
		return;
	}

	BeehiveCombState = SourceItemInstance->BeehiveCombState;

	if (SourceItemInstance->HasHoneyContainerState())
	{
		const FHoneyContainerItemState SourceState = SourceItemInstance->GetHoneyContainerState();
		SetHoneyContainerState(SourceState.CurrentVolumeMl, SourceState.HoneyDensity, SourceState.HoneyRipeness);
	}
	else
	{
		ClearHoneyContainerState();
	}

	if (SourceItemInstance->HasBeeCarrierState())
	{
		const FBeeCarrierItemState SourceState = SourceItemInstance->GetBeeCarrierState();
		SetBeeCarrierState(SourceState.CapturedBeeAmount);
	}
	else
	{
		ClearBeeCarrierState();
	}

	if (SourceItemInstance->HasQueenCageState())
	{
		SetQueenCageState(SourceItemInstance->GetQueenCageState());
	}
	else
	{
		ClearQueenCageState();
	}
}

bool UItemInstance::HasDurability() const
{
	return Definition && Definition->bUsesDurability;
}

float UItemInstance::GetCurrentDurability() const
{
	return HasDurability() ? FMath::Max(0.0f, Durability) : 0.0f;
}

float UItemInstance::GetMaxDurability() const
{
	return HasDurability() ? FMath::Max(0.0f, Definition->MaxDurability) : 0.0f;
}

float UItemInstance::GetDurabilityRatio() const
{
	const float MaxDurability = GetMaxDurability();
	if (MaxDurability <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetCurrentDurability() / MaxDurability, 0.0f, 1.0f);
}

UItemAction* UItemInstance::FindActionByTag(FGameplayTag ActionTag) const
{
	if (!ActionTag.IsValid())
	{
		return nullptr;
	}

	for (UItemAction* Action : Actions)
	{
		if (IsValid(Action) && Action->GetActionTypeTag() == ActionTag)
		{
			return Action;
		}
	}

	return nullptr;
}

UHoldItemUseAction* UItemInstance::FindHoldItemUseAction() const
{
	for (UItemAction* Action : Actions)
	{
		if (UHoldItemUseAction* HoldAction = Cast<UHoldItemUseAction>(Action))
		{
			return HoldAction;
		}
	}

	return nullptr;
}

bool UItemInstance::HasActionByTag(FGameplayTag ActionTag) const
{
	return FindActionByTag(ActionTag) != nullptr;
}

FItemActionExecutionResult UItemInstance::ExecuteActionByTag(FGameplayTag ActionTag, const FItemActionContext& Context)
{
	UItemAction* Action = FindActionByTag(ActionTag);
	if (!Action)
	{
		FItemActionExecutionResult Result;
		Result.Message = FText::FromString(TEXT("Requested item action does not exist on this item."));
		return Result;
	}

	return Action->Execute(Context);
}

FGameplayTagContainer UItemInstance::GetHotbarItemTags_Implementation() const
{
	return Definition ? Definition->GameplayTags : FGameplayTagContainer();
}

void UItemInstance::RebuildActions()
{
	Actions.Reset();

	if (!Definition)
	{
		return;
	}

	for (const FItemActionSpec& ActionSpec : Definition->ActionSpecs)
	{
		UClass* ActionClass = ActionSpec.ActionClass.Get();
		if (!ActionClass || ActionClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}

		UItemAction* Action = NewObject<UItemAction>(this, ActionClass);
		if (!Action)
		{
			continue;
		}

		Action->InitializeAction(this, ActionSpec);
		Actions.Add(Action);
	}
}
