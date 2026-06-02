#include "Inventory/ItemInstance.h"

#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Inventory/HoldItemUseAction.h"
#include "Inventory/ItemAction.h"
#include "Inventory/ItemDefinition.h"

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
	const int32 MaxStack = Definition ? FMath::Max(1, Definition->MaxStack) : 1;
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
}

void UItemInstance::ClearBeehiveCombState()
{
	BeehiveCombState = FBeehiveCombItemState();
}

bool UItemInstance::HasBeehiveCombState() const
{
	return BeehiveCombState.bHasState;
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
