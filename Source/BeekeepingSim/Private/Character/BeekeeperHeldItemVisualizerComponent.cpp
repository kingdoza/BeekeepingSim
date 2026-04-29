#include "Character/BeekeeperHeldItemVisualizerComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Character/BeekeeperCharacter.h"
#include "Focus/BeekeeperFocusComponent.h"
#include "Inventory/BeekeeperHotbarComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/ItemPresentationActor.h"

UBeekeeperHeldItemVisualizerComponent::UBeekeeperHeldItemVisualizerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBeekeeperHeldItemVisualizerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABeekeeperCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		SetComponentTickEnabled(false);
		return;
	}

	HotbarComponent = OwnerCharacter->GetBeekeeperHotbar();
	FocusComponent = OwnerCharacter->GetBeekeeperFocus();
	OwnerCamera = OwnerCharacter->GetFirstPersonCamera();

	if (HotbarComponent)
	{
		HotbarComponent->OnHotbarChanged.AddDynamic(this, &UBeekeeperHeldItemVisualizerComponent::HandleHotbarChanged);
	}

	if (FocusComponent)
	{
		FocusComponent->OnFocusRuleChanged.AddDynamic(this, &UBeekeeperHeldItemVisualizerComponent::HandleFocusRuleChanged);
	}

	bWasRunningLocally = ShouldRunLocally();
	RefreshVisualization();
}

void UBeekeeperHeldItemVisualizerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HotbarComponent)
	{
		HotbarComponent->OnHotbarChanged.RemoveDynamic(this, &UBeekeeperHeldItemVisualizerComponent::HandleHotbarChanged);
	}

	if (FocusComponent)
	{
		FocusComponent->OnFocusRuleChanged.RemoveDynamic(this, &UBeekeeperHeldItemVisualizerComponent::HandleFocusRuleChanged);
	}

	DestroyPresentationActor();

	CurrentVisualizedItem = nullptr;
	CurrentPresentationClass = nullptr;
	CurrentFallbackMesh = nullptr;
	OwnerCamera = nullptr;
	FocusComponent = nullptr;
	HotbarComponent = nullptr;
	OwnerCharacter = nullptr;
	bWasRunningLocally = false;

	Super::EndPlay(EndPlayReason);
}

void UBeekeeperHeldItemVisualizerComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bShouldRunLocally = ShouldRunLocally();
	if (!bShouldRunLocally)
	{
		DestroyPresentationActor();
		bWasRunningLocally = false;
		SetComponentTickInterval(0.25f);
		return;
	}

	SetComponentTickInterval(0.0f);

	if (!bWasRunningLocally)
	{
		bWasRunningLocally = true;
		RefreshVisualization();
	}

	if (!HeldPresentationActor || !HotbarComponent || HotbarComponent->GetPresentationMode() != EHotbarPresentationMode::OnCursor)
	{
		return;
	}

	UpdateCursorPresentation();
}

void UBeekeeperHeldItemVisualizerComponent::HandleHotbarChanged()
{
	RefreshVisualization();
}

void UBeekeeperHeldItemVisualizerComponent::HandleFocusRuleChanged(bool bHasFocusTarget, FFocusItemRule FocusItemRule)
{
	RefreshVisualization();
}

void UBeekeeperHeldItemVisualizerComponent::RefreshVisualization()
{
	if (!ShouldRunLocally())
	{
		DestroyPresentationActor();
		return;
	}

	UpdatePresentationForSelectedItem();
	UpdateVisibilityAndTransform();
}

void UBeekeeperHeldItemVisualizerComponent::UpdateCursorPresentation()
{
	if (!HeldPresentationActor || !OwnerCamera || !HotbarComponent)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter ? OwnerCharacter->GetController() : nullptr);
	if (!PlayerController)
	{
		HeldPresentationActor->SetPresentationHidden(true);
		return;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return;
	}

	float CursorX = 0.0f;
	float CursorY = 0.0f;
	if (!PlayerController->GetMousePosition(CursorX, CursorY))
	{
		return;
	}

	FVector RayOrigin = FVector::ZeroVector;
	FVector RayDirection = FVector::ZeroVector;
	if (!PlayerController->DeprojectScreenPositionToWorld(CursorX, CursorY, RayOrigin, RayDirection))
	{
		return;
	}

	const FVector SafeRayDirection = RayDirection.GetSafeNormal();
	if (SafeRayDirection.IsNearlyZero())
	{
		return;
	}

	const FVector CameraLocation = OwnerCamera->GetComponentLocation();
	const FVector CameraForward = OwnerCamera->GetForwardVector();
	const FVector CameraRight = OwnerCamera->GetRightVector();
	const FVector CameraUp = OwnerCamera->GetUpVector();

	constexpr float MinCursorPlaneDistance = 1.0f;
	const float PlaneDistance = FMath::Max(MinCursorPlaneDistance, OnCursorPlaneDistance);
	const FVector PlanePoint = CameraLocation + CameraForward * PlaneDistance;

	const float DirectionDot = FVector::DotProduct(SafeRayDirection, CameraForward);
	constexpr float MinPlaneDot = KINDA_SMALL_NUMBER;
	if (DirectionDot <= MinPlaneDot)
	{
		return;
	}

	const float RayDistanceToPlane = FVector::DotProduct(PlanePoint - RayOrigin, CameraForward) / DirectionDot;
	if (RayDistanceToPlane <= 0.0f)
	{
		return;
	}

	const FVector TargetWorldLocation = RayOrigin + SafeRayDirection * RayDistanceToPlane;

	const FVector WorldOffset =
		CameraForward * OnCursorLocalOffset.X +
		CameraRight * OnCursorLocalOffset.Y +
		CameraUp * OnCursorLocalOffset.Z;

	const FVector FinalWorldLocation = TargetWorldLocation + WorldOffset;
	const FVector RelativeLocation = OwnerCamera->GetComponentTransform().InverseTransformPosition(FinalWorldLocation);

	HeldPresentationActor->SetActorRelativeLocation(RelativeLocation);
	HeldPresentationActor->SetActorRelativeRotation(OnCursorLocalRotation);
	HeldPresentationActor->SetActorRelativeScale3D(OnCursorRelativeScale);
}

void UBeekeeperHeldItemVisualizerComponent::UpdatePresentationForSelectedItem()
{
	CurrentVisualizedItem = GetSelectedItemInstance();
	CurrentPresentationClass = nullptr;
	CurrentFallbackMesh = nullptr;

	if (!CurrentVisualizedItem)
	{
		DestroyPresentationActor();
		return;
	}

	CurrentPresentationClass = CurrentVisualizedItem->GetHeldPresentationActorClass();
	CurrentFallbackMesh = CurrentPresentationClass ? nullptr : CurrentVisualizedItem->GetWorldMesh();

	if (!CurrentPresentationClass && !CurrentFallbackMesh)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Held item presentation skipped. Missing class and fallback mesh for item instance '%s'."),
			*GetNameSafe(CurrentVisualizedItem));
		DestroyPresentationActor();
		return;
	}

	const UClass* DesiredClass = CurrentPresentationClass ? CurrentPresentationClass.Get() : AItemPresentationActor::StaticClass();
	const bool bClassChanged = !HeldPresentationActor || HeldPresentationActor->GetClass() != DesiredClass;
	if (bClassChanged)
	{
		DestroyPresentationActor();
		EnsurePresentationActor();
	}

	if (!HeldPresentationActor)
	{
		return;
	}

	HeldPresentationActor->InitializePresentation(OwnerCharacter, CurrentVisualizedItem);
	HeldPresentationActor->DisablePresentationCollision();
	HeldPresentationActor->ApplyFirstPersonVisibilityPolicy();
	if (!CurrentPresentationClass)
	{
		HeldPresentationActor->SetFallbackStaticMesh(CurrentFallbackMesh);
	}
}

void UBeekeeperHeldItemVisualizerComponent::UpdateVisibilityAndTransform()
{
	if (!HeldPresentationActor || !HotbarComponent)
	{
		return;
	}

	if (!ShouldRunLocally())
	{
		DestroyPresentationActor();
		return;
	}

	const EHotbarPresentationMode PresentationMode = HotbarComponent->GetPresentationMode();
	if (!CurrentVisualizedItem || PresentationMode == EHotbarPresentationMode::None)
	{
		HeldPresentationActor->SetPresentationHidden(true);
		return;
	}

	if (!CurrentPresentationClass && !CurrentFallbackMesh)
	{
		HeldPresentationActor->SetPresentationHidden(true);
		return;
	}

	HeldPresentationActor->SetPresentationHidden(false);

	if (PresentationMode == EHotbarPresentationMode::InHand)
	{
		HeldPresentationActor->SetActorRelativeLocation(InHandLocalOffset);
		HeldPresentationActor->SetActorRelativeRotation(InHandLocalRotation);
		HeldPresentationActor->SetActorRelativeScale3D(InHandRelativeScale);
		return;
	}

	HeldPresentationActor->SetActorRelativeRotation(OnCursorLocalRotation);
	HeldPresentationActor->SetActorRelativeScale3D(OnCursorRelativeScale);
	UpdateCursorPresentation();
}

bool UBeekeeperHeldItemVisualizerComponent::ShouldRunLocally() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	return PlayerController && PlayerController->IsLocalController();
}

void UBeekeeperHeldItemVisualizerComponent::EnsurePresentationActor()
{
	if (HeldPresentationActor || !ShouldRunLocally() || !OwnerCharacter || !OwnerCamera)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UClass* SpawnClass = CurrentPresentationClass ? CurrentPresentationClass.Get() : AItemPresentationActor::StaticClass();
	if (!SpawnClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags |= RF_Transient;

	AItemPresentationActor* SpawnedActor =
		World->SpawnActor<AItemPresentationActor>(SpawnClass, FTransform::Identity, SpawnParameters);
	if (!SpawnedActor)
	{
		return;
	}

	SpawnedActor->SetReplicates(false);
	SpawnedActor->AttachToComponent(OwnerCamera, FAttachmentTransformRules::KeepRelativeTransform);
	HeldPresentationActor = SpawnedActor;
}

void UBeekeeperHeldItemVisualizerComponent::DestroyPresentationActor()
{
	if (HeldPresentationActor)
	{
		HeldPresentationActor->Destroy();
		HeldPresentationActor = nullptr;
	}
}

UItemInstance* UBeekeeperHeldItemVisualizerComponent::GetSelectedItemInstance() const
{
	return HotbarComponent ? HotbarComponent->GetSelectedItemInstance() : nullptr;
}
