// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/FocusTargetComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Public/BeekeeperCharacter.h"
#include "Public/FocusInteractable.h"

UFocusTargetComponent::UFocusTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionKeyText = FText::FromString(TEXT("F"));
}

void UFocusTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveOutlineComponents();
	ApplyOutlineState(false);
}

void UFocusTargetComponent::SetFocused(bool bInFocused)
{
	if (bIsFocused == bInFocused)
	{
		return;
	}

	bIsFocused = bInFocused;
	ApplyOutlineState(bIsFocused);
}

void UFocusTargetComponent::SetDisplayName(const FText& NewDisplayName)
{
	DisplayName = NewDisplayName;
}

void UFocusTargetComponent::SetInteractionKeyText(const FText& NewInteractionKeyText)
{
	InteractionKeyText = NewInteractionKeyText;
}

FFocusPromptData UFocusTargetComponent::GetPromptData() const
{
	FFocusPromptData PromptData;
	PromptData.bIsValid = true;
	PromptData.DisplayName = DisplayName.IsEmpty() && GetOwner() ? FText::FromString(GetOwner()->GetName()) : DisplayName;
	PromptData.InteractionKeyText = InteractionKeyText;
	return PromptData;
}

void UFocusTargetComponent::NotifyFocusEnter(ABeekeeperCharacter* InteractingCharacter)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->GetClass()->ImplementsInterface(UFocusInteractable::StaticClass()))
	{
		return;
	}

	IFocusInteractable::Execute_OnFocusEnter(OwnerActor, InteractingCharacter);
}

void UFocusTargetComponent::NotifyFocusExit(ABeekeeperCharacter* InteractingCharacter)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->GetClass()->ImplementsInterface(UFocusInteractable::StaticClass()))
	{
		return;
	}

	IFocusInteractable::Execute_OnFocusExit(OwnerActor, InteractingCharacter);
} 

void UFocusTargetComponent::NotifyFocusConfirm(ABeekeeperCharacter* InteractingCharacter)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->GetClass()->ImplementsInterface(UFocusInteractable::StaticClass()))
	{
		return;
	}

	IFocusInteractable::Execute_OnFocusConfirm(OwnerActor, InteractingCharacter);
}

void UFocusTargetComponent::NotifyFocusCancel(ABeekeeperCharacter* InteractingCharacter)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->GetClass()->ImplementsInterface(UFocusInteractable::StaticClass()))
	{
		return;
	}

	IFocusInteractable::Execute_OnFocusCancel(OwnerActor, InteractingCharacter);
}

void UFocusTargetComponent::ApplyOutlineState(bool bEnabled)
{
	if (!bUseCustomDepthOutline)
	{
		return;
	}

	for (UPrimitiveComponent* PrimitiveComponent : ResolvedOutlineComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetRenderCustomDepth(bEnabled);
		if (bEnabled)
		{
			PrimitiveComponent->SetCustomDepthStencilValue(CustomDepthStencilValue);
		}
	}
}

void UFocusTargetComponent::ResolveOutlineComponents()
{
	ResolvedOutlineComponents.Reset();

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	if (!GetOwner())
	{
		return;
	}

	GetOwner()->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent || !PrimitiveComponent->IsRegistered())
		{
			continue;
		}

		if (OutlineComponentTags.Num() > 0)
		{
			bool bMatchesAnyTag = false;
			for (const FName& OutlineComponentTag : OutlineComponentTags)
			{
				if (OutlineComponentTag.IsNone())
				{
					continue;
				}

				if (PrimitiveComponent->ComponentHasTag(OutlineComponentTag))
				{
					bMatchesAnyTag = true;
					break;
				}
			}

			if (!bMatchesAnyTag)
			{
				continue;
			}
		}

		ResolvedOutlineComponents.AddUnique(PrimitiveComponent);
	}
}
