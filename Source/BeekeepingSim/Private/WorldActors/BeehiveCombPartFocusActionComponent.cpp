#include "WorldActors/BeehiveCombPartFocusActionComponent.h"

#include "GameplayTagsManager.h"

UBeehiveCombPartFocusActionComponent::UBeehiveCombPartFocusActionComponent()
{
	SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
	FGameplayTagContainer RequiredTags;
	RequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.LidOpen")), false));
	SetRequiredStateTags(RequiredTags);
	SetExclusiveGroup(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.CombLift")), false));
}
