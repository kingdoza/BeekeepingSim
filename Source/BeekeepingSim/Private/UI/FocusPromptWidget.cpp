#include "UI/FocusPromptWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/BeekeeperCharacter.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Focus/BeekeeperFocusComponent.h"
#include "GameFramework/PlayerController.h"

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
	UpdateCanTick();

	(void)Cast<UCanvasPanelSlot>(PromptContent ? PromptContent->Slot : nullptr);

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

void UFocusPromptWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CurrentPromptData.bIsValid || !IsVisible())
	{
		return;
	}

	UpdatePromptPosition();
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
		OnPromptEntriesApplied(CurrentPromptData, CurrentPromptData.Entries, false);
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
	UpdatePromptPosition();
	OnPromptDataApplied(CurrentPromptData, true);
	OnPromptEntriesApplied(CurrentPromptData, CurrentPromptData.Entries, true);
}

void UFocusPromptWidget::ClearPrompt()
{
	SetPromptData(FFocusPromptData());
}

void UFocusPromptWidget::HandleFocusPromptChanged(FFocusPromptData PromptData)
{
	SetPromptData(PromptData);
}

void UFocusPromptWidget::UpdatePromptPosition()
{
	if (!PromptContent)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PromptContent->Slot);
	if (!CanvasSlot)
	{
		return;
	}

	FVector2D AnchorPosition = FVector2D::ZeroVector;
	if (!TryGetPromptAnchorPosition(AnchorPosition))
	{
		return;
	}

	ForceLayoutPrepass();
	const FVector2D DesiredSize = PromptContent->GetDesiredSize();
	const FVector2D Alignment = CanvasSlot->GetAlignment();

	FVector2D FinalAnchorPosition = AnchorPosition;
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (TryGetViewportSizeInWidgetUnits(ViewportSize))
	{
		FinalAnchorPosition = ClampPromptAnchorToViewport(AnchorPosition, ViewportSize, DesiredSize, Alignment, ViewportPadding);
	}

	CanvasSlot->SetPosition(FinalAnchorPosition);
}

bool UFocusPromptWidget::TryGetPromptAnchorPosition(FVector2D& OutPosition) const
{
	switch (CurrentPromptData.AnchorMode)
	{
	case EFocusPromptAnchorMode::ScreenCenter:
	{
		FVector2D ViewportSize = FVector2D::ZeroVector;
		if (!TryGetViewportSizeInWidgetUnits(ViewportSize))
		{
			return false;
		}

		OutPosition = (ViewportSize * 0.5f) + ScreenCenterOffset;
		return true;
	}

	case EFocusPromptAnchorMode::MouseCursor:
	{
		FVector2D MousePosition = FVector2D::ZeroVector;
		if (!TryGetMousePositionInWidgetUnits(MousePosition))
		{
			return false;
		}

		OutPosition = MousePosition + MouseCursorOffset;
		return true;
	}
	}

	return false;
}

bool UFocusPromptWidget::TryGetViewportSizeInWidgetUnits(FVector2D& OutViewportSize) const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return false;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return false;
	}

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	const float Scale = ViewportScale > KINDA_SMALL_NUMBER ? ViewportScale : 1.0f;
	OutViewportSize = FVector2D(static_cast<float>(ViewportSizeX) / Scale, static_cast<float>(ViewportSizeY) / Scale);
	return true;
}

bool UFocusPromptWidget::TryGetMousePositionInWidgetUnits(FVector2D& OutMousePosition) const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return false;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		return false;
	}

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	const float Scale = ViewportScale > KINDA_SMALL_NUMBER ? ViewportScale : 1.0f;
	OutMousePosition = FVector2D(MouseX / Scale, MouseY / Scale);
	return true;
}

FVector2D UFocusPromptWidget::ClampPromptAnchorToViewport(
	const FVector2D& AnchorPosition,
	const FVector2D& ViewportSize,
	const FVector2D& DesiredSize,
	const FVector2D& Alignment,
	const FVector2D& Padding)
{
	const FVector2D SafePadding(FMath::Max(0.0f, Padding.X), FMath::Max(0.0f, Padding.Y));
	const FVector2D MinAnchor = SafePadding + (DesiredSize * Alignment);
	const FVector2D MaxAnchor = ViewportSize - SafePadding - (DesiredSize * (FVector2D(1.0f, 1.0f) - Alignment));

	const auto ClampAxis = [](float Value, float MinValue, float MaxValue)
	{
		if (MaxValue < MinValue)
		{
			return 0.5f * (MinValue + MaxValue);
		}

		return FMath::Clamp(Value, MinValue, MaxValue);
	};

	return FVector2D(
		ClampAxis(AnchorPosition.X, MinAnchor.X, MaxAnchor.X),
		ClampAxis(AnchorPosition.Y, MinAnchor.Y, MaxAnchor.Y));
}
