#include "Inventory/PollenPattyUseAction.h"

#include "GameplayTagContainer.h"
#include "WorldActors/PlacedItemActor.h"

UPollenPattyUseAction::UPollenPattyUseAction()
{
	PlacedActorClass = APlacedItemActor::StaticClass();

	const FGameplayTag PollenAreaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.UseArea.Beehive.PollenPatty")), false);
	if (PollenAreaTag.IsValid())
	{
		FGameplayTagQueryExpression Expression;
		Expression.AllTagsMatch().AddTag(PollenAreaTag);
		UseAreaTagQuery.Build(Expression);
	}
}

bool UPollenPattyUseAction::BeginUse(const FItemActionContext& Context)
{
	if (!Super::BeginUse(Context))
	{
		return false;
	}

	ReceivePollenPattyUseStarted(Context);
	return true;
}

void UPollenPattyUseAction::EndUse(const FItemActionContext& Context, bool bWasCanceled)
{
	ReceivePollenPattyUseEnded(Context, bWasCanceled);
	Super::EndUse(Context, bWasCanceled);
}

FItemActionExecutionResult UPollenPattyUseAction::ApplyUseEffect(const FItemActionContext& Context, float DeltaTime)
{
	FItemActionExecutionResult Result = Super::ApplyUseEffect(Context, DeltaTime);
	if (!Result.bSucceeded)
	{
		ReceivePollenPattyInstallBlocked(Context);
		return Result;
	}

	ReceivePollenPattyInstalled(Context);
	return Result;
}
