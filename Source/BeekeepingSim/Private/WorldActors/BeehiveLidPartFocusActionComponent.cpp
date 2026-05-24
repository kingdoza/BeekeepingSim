#include "WorldActors/BeehiveLidPartFocusActionComponent.h"

#include "GameplayTagsManager.h"

UBeehiveLidPartFocusActionComponent::UBeehiveLidPartFocusActionComponent()
{
	SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
	FGameplayTagContainer ProvidedTags;
	ProvidedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.LidOpen")), false));
	SetProvidedStateTags(ProvidedTags);
}
