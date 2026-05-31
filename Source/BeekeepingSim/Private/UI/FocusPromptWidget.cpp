#include "UI/FocusPromptWidget.h"

#include "Character/BeekeeperCharacter.h"
#include "Components/TextBlock.h"
#include "Focus/BeekeeperFocusComponent.h"

void UFocusPromptWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UFocusPromptWidget::NativeConstruct()
{
	SetVisibility(ESlateVisibility::Collapsed);

	Super::NativeConstruct();

	ABeekeeperCharacter* BeekeeperCharacter = Cast<ABeekeeperCharacter>(GetOwningPlayerPawn());
	if (!BeekeeperCharacter)
	{
		return;
	}

	BindToFocusComponent(BeekeeperCharacter->GetBeekeeperFocus());
}

void UFocusPromptWidget::NativeDestruct()
{
	UnbindFromFocusComponent();
	Super::NativeDestruct();
}

void UFocusPromptWidget::BindToFocusComponent(UBeekeeperFocusComponent* InFocusComponent)
{
	if (BoundFocusComponent)
	{
		UnbindFromFocusComponent();
	}

	if (!InFocusComponent)
	{
		ClearPrompt();
		return;
	}

	BoundFocusComponent = InFocusComponent;
	BoundFocusComponent->OnFocusPromptChanged.AddUniqueDynamic(this, &UFocusPromptWidget::HandleFocusPromptChanged);
	SetPromptData(BoundFocusComponent->GetCurrentPromptData());
}

void UFocusPromptWidget::UnbindFromFocusComponent()
{
	if (BoundFocusComponent)
	{
		BoundFocusComponent->OnFocusPromptChanged.RemoveDynamic(this, &UFocusPromptWidget::HandleFocusPromptChanged);
		BoundFocusComponent = nullptr;
	}
}

void UFocusPromptWidget::SetPromptData(const FFocusPromptData& InPromptData)
{
	CurrentPromptData = InPromptData;

	if (!CurrentPromptData.bIsValid)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		OnPromptDataApplied(CurrentPromptData, false);
		return;
	}

	if (TargetNameText)
	{
		TargetNameText->SetText(CurrentPromptData.DisplayName);
	}

	if (KeyText)
	{
		KeyText->SetText(CurrentPromptData.InteractionKeyText);
	}

	SetVisibility(ESlateVisibility::Visible);
	OnPromptDataApplied(CurrentPromptData, true);
}

void UFocusPromptWidget::ClearPrompt()
{
	SetPromptData(FFocusPromptData());
}

void UFocusPromptWidget::HandleFocusPromptChanged(FFocusPromptData PromptData)
{
	SetPromptData(PromptData);
}
