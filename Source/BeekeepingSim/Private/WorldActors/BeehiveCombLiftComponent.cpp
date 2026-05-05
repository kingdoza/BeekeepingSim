#include "WorldActors/BeehiveCombLiftComponent.h"

#include "Character/BeekeeperCharacter.h"
#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "WorldActors/Beehive.h"
#include "WorldActors/BeehiveCombActor.h"

UBeehiveCombLiftComponent::UBeehiveCombLiftComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBeehiveCombLiftComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ActiveMoveTask.bActive)
	{
		SetComponentTickEnabled(false);
		return;
	}

	UChildActorComponent* SlotComponent = ActiveMoveTask.SlotComponent.Get();
	if (!SlotComponent)
	{
		StopMoveTask();
		return;
	}

	const float Duration = FMath::Max(0.0f, ActiveMoveTask.Duration);
	if (Duration <= KINDA_SMALL_NUMBER)
	{
		ApplyRelativeTransformImmediately(SlotComponent, ActiveMoveTask.SlotIndex, ActiveMoveTask.TargetRelativeTransform, ActiveMoveTask.bClearLiftedIndexOnComplete);
		return;
	}

	ActiveMoveTask.Elapsed = FMath::Min(ActiveMoveTask.Elapsed + FMath::Max(0.0f, DeltaTime), Duration);
	const float Alpha = FMath::Clamp(ActiveMoveTask.Elapsed / Duration, 0.0f, 1.0f);

	const FVector Location = FMath::Lerp(
		ActiveMoveTask.StartRelativeTransform.GetLocation(),
		ActiveMoveTask.TargetRelativeTransform.GetLocation(),
		Alpha);
	const FQuat Rotation = FQuat::Slerp(
		ActiveMoveTask.StartRelativeTransform.GetRotation(),
		ActiveMoveTask.TargetRelativeTransform.GetRotation(),
		Alpha).GetNormalized();
	const FVector Scale = FMath::Lerp(
		ActiveMoveTask.StartRelativeTransform.GetScale3D(),
		ActiveMoveTask.TargetRelativeTransform.GetScale3D(),
		Alpha);

	SlotComponent->SetRelativeTransform(FTransform(Rotation, Location, Scale));
	if (Alpha >= 1.0f)
	{
		ApplyRelativeTransformImmediately(SlotComponent, ActiveMoveTask.SlotIndex, ActiveMoveTask.TargetRelativeTransform, ActiveMoveTask.bClearLiftedIndexOnComplete);
	}
}

void UBeehiveCombLiftComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AbortAllLiftedCombsImmediately();
	Super::EndPlay(EndPlayReason);
}

bool UBeehiveCombLiftComponent::LiftComb(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter)
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || !CombActor || !InteractingCharacter)
	{
		return false;
	}

	const int32 SlotIndex = OwnerBeehive->FindManagedCombSlotIndex(CombActor);
	if (SlotIndex == INDEX_NONE)
	{
		return false;
	}

	if (LiftedCombSlotIndex != INDEX_NONE && LiftedCombSlotIndex != SlotIndex)
	{
		if (UChildActorComponent* ExistingLiftedSlot = OwnerBeehive->GetCombSlotComponentByIndex(LiftedCombSlotIndex))
		{
			FTransform ExistingRestTransform;
			if (OwnerBeehive->BuildCombSlotRestRelativeTransform(LiftedCombSlotIndex, ExistingRestTransform))
			{
				ApplyRelativeTransformImmediately(ExistingLiftedSlot, LiftedCombSlotIndex, ExistingRestTransform, true);
			}
		}
		LiftedCombSlotIndex = INDEX_NONE;
	}

	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(SlotIndex);
	if (!SlotComponent)
	{
		return false;
	}

	FTransform TargetRelativeTransform;
	if (!BuildLiftTargetRelativeTransform(SlotIndex, InteractingCharacter, TargetRelativeTransform))
	{
		return false;
	}

	StartMoveTask(
		SlotComponent,
		SlotIndex,
		SlotComponent->GetRelativeTransform(),
		TargetRelativeTransform,
		false);

	LiftedCombSlotIndex = SlotIndex;
	return true;
}

bool UBeehiveCombLiftComponent::ReturnComb(ABeehiveCombActor* CombActor)
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || !CombActor)
	{
		return false;
	}

	const int32 SlotIndex = OwnerBeehive->FindManagedCombSlotIndex(CombActor);
	if (SlotIndex == INDEX_NONE)
	{
		return false;
	}

	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(SlotIndex);
	if (!SlotComponent)
	{
		return false;
	}

	FTransform RestRelativeTransform;
	if (!OwnerBeehive->BuildCombSlotRestRelativeTransform(SlotIndex, RestRelativeTransform))
	{
		return false;
	}

	StartMoveTask(
		SlotComponent,
		SlotIndex,
		SlotComponent->GetRelativeTransform(),
		RestRelativeTransform,
		true);

	return true;
}

void UBeehiveCombLiftComponent::AbortCombLift(ABeehiveCombActor* CombActor)
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || !CombActor)
	{
		return;
	}

	const int32 SlotIndex = OwnerBeehive->FindManagedCombSlotIndex(CombActor);
	if (SlotIndex == INDEX_NONE)
	{
		return;
	}

	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(SlotIndex);
	FTransform RestRelativeTransform;
	if (!SlotComponent || !OwnerBeehive->BuildCombSlotRestRelativeTransform(SlotIndex, RestRelativeTransform))
	{
		return;
	}

	ApplyRelativeTransformImmediately(SlotComponent, SlotIndex, RestRelativeTransform, true);
}

void UBeehiveCombLiftComponent::ReturnAllLiftedCombs()
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || LiftedCombSlotIndex == INDEX_NONE)
	{
		return;
	}

	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(LiftedCombSlotIndex);
	FTransform RestRelativeTransform;
	if (!SlotComponent || !OwnerBeehive->BuildCombSlotRestRelativeTransform(LiftedCombSlotIndex, RestRelativeTransform))
	{
		LiftedCombSlotIndex = INDEX_NONE;
		StopMoveTask();
		return;
	}

	StartMoveTask(
		SlotComponent,
		LiftedCombSlotIndex,
		SlotComponent->GetRelativeTransform(),
		RestRelativeTransform,
		true);
}

void UBeehiveCombLiftComponent::ReapplyLiftedCombTransformAfterLayoutRefresh()
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || LiftedCombSlotIndex == INDEX_NONE)
	{
		return;
	}

	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(LiftedCombSlotIndex);
	if (!SlotComponent)
	{
		LiftedCombSlotIndex = INDEX_NONE;
		StopMoveTask();
		return;
	}

	FTransform TargetRelativeTransform;
	if (!BuildLiftTargetRelativeTransformFromStoredRotation(LiftedCombSlotIndex, TargetRelativeTransform))
	{
		return;
	}

	SlotComponent->SetRelativeTransform(TargetRelativeTransform);
	StopMoveTask();
}

ABeehive* UBeehiveCombLiftComponent::GetOwnerBeehive() const
{
	return Cast<ABeehive>(GetOwner());
}

bool UBeehiveCombLiftComponent::BuildLiftTargetRelativeTransform(int32 SlotIndex, ABeekeeperCharacter* InteractingCharacter, FTransform& OutTransform)
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || !InteractingCharacter)
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	const USceneComponent* LiftTargetRoot = OwnerBeehive->GetCombLiftTargetRoot();
	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(SlotIndex);
	USceneComponent* ParentComponent = SlotComponent ? SlotComponent->GetAttachParent() : nullptr;
	FTransform RestRelativeTransform = FTransform::Identity;
	if (!LiftTargetRoot || !SlotComponent || !ParentComponent || !OwnerBeehive->BuildCombSlotRestRelativeTransform(SlotIndex, RestRelativeTransform))
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	const FTransform TargetWorldTransform = LiftTargetRoot->GetComponentTransform();
	const FTransform ParentWorldTransform = ParentComponent->GetComponentTransform();
	const FTransform TargetRelativeTransform = TargetWorldTransform.GetRelativeTransform(ParentWorldTransform);

	OutTransform = FTransform(
		TargetRelativeTransform.GetRotation(),
		TargetRelativeTransform.GetLocation(),
		RestRelativeTransform.GetScale3D());
	return true;
}

bool UBeehiveCombLiftComponent::BuildLiftTargetRelativeTransformFromStoredRotation(int32 SlotIndex, FTransform& OutTransform) const
{
	const ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive)
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	const USceneComponent* LiftTargetRoot = OwnerBeehive->GetCombLiftTargetRoot();
	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(SlotIndex);
	USceneComponent* ParentComponent = SlotComponent ? SlotComponent->GetAttachParent() : nullptr;
	FTransform RestRelativeTransform = FTransform::Identity;
	if (!LiftTargetRoot || !SlotComponent || !ParentComponent || !OwnerBeehive->BuildCombSlotRestRelativeTransform(SlotIndex, RestRelativeTransform))
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	const FTransform TargetWorldTransform = LiftTargetRoot->GetComponentTransform();
	const FTransform ParentWorldTransform = ParentComponent->GetComponentTransform();
	const FTransform TargetRelativeTransform = TargetWorldTransform.GetRelativeTransform(ParentWorldTransform);

	OutTransform = FTransform(
		TargetRelativeTransform.GetRotation(),
		TargetRelativeTransform.GetLocation(),
		RestRelativeTransform.GetScale3D());
	return true;
}

void UBeehiveCombLiftComponent::StartMoveTask(UChildActorComponent* SlotComponent, int32 SlotIndex, const FTransform& StartRelativeTransform, const FTransform& TargetRelativeTransform, bool bClearLiftedIndexOnComplete)
{
	if (!SlotComponent)
	{
		return;
	}

	ActiveMoveTask.SlotComponent = SlotComponent;
	ActiveMoveTask.StartRelativeTransform = StartRelativeTransform;
	ActiveMoveTask.TargetRelativeTransform = TargetRelativeTransform;
	ActiveMoveTask.Elapsed = 0.0f;
	ActiveMoveTask.Duration = FMath::Max(0.0f, CombLiftMoveDuration);
	ActiveMoveTask.bActive = true;
	ActiveMoveTask.bClearLiftedIndexOnComplete = bClearLiftedIndexOnComplete;
	ActiveMoveTask.SlotIndex = SlotIndex;
	SetComponentTickEnabled(true);

	if (ActiveMoveTask.Duration <= KINDA_SMALL_NUMBER)
	{
		ApplyRelativeTransformImmediately(SlotComponent, SlotIndex, TargetRelativeTransform, bClearLiftedIndexOnComplete);
	}
}

void UBeehiveCombLiftComponent::ApplyRelativeTransformImmediately(UChildActorComponent* SlotComponent, int32 SlotIndex, const FTransform& TargetRelativeTransform, bool bClearLiftedIndexNow)
{
	if (SlotComponent)
	{
		SlotComponent->SetRelativeTransform(TargetRelativeTransform);
	}

	if (bClearLiftedIndexNow && LiftedCombSlotIndex == SlotIndex)
	{
		LiftedCombSlotIndex = INDEX_NONE;
	}

	StopMoveTask();
}

void UBeehiveCombLiftComponent::StopMoveTask()
{
	ActiveMoveTask = FCombLiftMoveTask();
	SetComponentTickEnabled(false);
}

void UBeehiveCombLiftComponent::AbortAllLiftedCombsImmediately()
{
	ABeehive* OwnerBeehive = GetOwnerBeehive();
	if (!OwnerBeehive || LiftedCombSlotIndex == INDEX_NONE)
	{
		StopMoveTask();
		return;
	}

	UChildActorComponent* SlotComponent = OwnerBeehive->GetCombSlotComponentByIndex(LiftedCombSlotIndex);
	FTransform RestRelativeTransform;
	if (SlotComponent && OwnerBeehive->BuildCombSlotRestRelativeTransform(LiftedCombSlotIndex, RestRelativeTransform))
	{
		SlotComponent->SetRelativeTransform(RestRelativeTransform);
	}

	LiftedCombSlotIndex = INDEX_NONE;
	StopMoveTask();
}
