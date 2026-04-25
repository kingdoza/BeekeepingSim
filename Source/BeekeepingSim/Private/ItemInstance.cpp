#include "Public/ItemInstance.h"

#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Public/ItemAction.h"
#include "Public/ItemDefinition.h"

void UItemInstance::InitializeFromDefinition(UItemDefinition* InDefinition, int32 InStackCount, float InDurability)
{
	Definition = InDefinition;
	InstanceId = FGuid::NewGuid();
	StackCount = 0;
	Durability = InDurability;
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
	Durability = NewDurability;
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
