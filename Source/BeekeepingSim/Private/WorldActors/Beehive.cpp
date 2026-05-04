// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldActors/Beehive.h"

#include "Environment/GameTimeBucketSubsystem.h"
#include "Focus/AnchoredFocusCursorActionComponent.h"
#include "Components/ChildActorComponent.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "WorldActors/BeehiveDualSwarmActor.h"
#include "WorldActors/BeehiveCombActor.h"
#include "Curves/CurveFloat.h"

namespace BeehiveAttractionSwarmNames
{
	static const FName AttractionPower(TEXT("User.AttractionPower"));
	static const FName NoisePower(TEXT("User.NoisePower"));
	static const FName SpawnSphereRadius(TEXT("User.SpawnSphereRadius"));
	static const FName SpawnAmount(TEXT("User.SpawnAmount"));
}

namespace BeehiveCombSlotNames
{
	static const TCHAR* SlotPrefix = TEXT("CombRuntimeSlot_");
	static const TCHAR* DeprecatedSlotPrefix = TEXT("CombSlot_");
}

namespace
{
	bool IsTemplateCombSlotComponent(const UChildActorComponent* Component)
	{
		if (!Component)
		{
			return false;
		}

		const UObject* Archetype = Component->GetArchetype();
		const UObject* ArchetypeOuter = Archetype ? Archetype->GetOuter() : nullptr;
		return ArchetypeOuter && ArchetypeOuter->IsTemplate();
	}

	bool ParseCombSlotIndex(const FString& ComponentName, const TCHAR* Prefix, int32& OutIndex)
	{
		if (!ComponentName.StartsWith(Prefix))
		{
			return false;
		}

		const FString IndexString = ComponentName.RightChop(FCString::Strlen(Prefix));
		if (!IndexString.IsNumeric())
		{
			return false;
		}

		OutIndex = FCString::Atoi(*IndexString);
		return OutIndex >= 0;
	}
}

ABeehive::ABeehive()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BeehiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeehiveMesh"));
	BeehiveMesh->SetupAttachment(Root);

	FocusTarget = CreateDefaultSubobject<UFocusTargetComponent>(TEXT("FocusTarget"));
	FocusAction = CreateDefaultSubobject<UAnchoredFocusCursorActionComponent>(TEXT("FocusAction"));

	SwarmSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SwarmSpline"));
	SwarmSpline->SetupAttachment(Root);

	BeehiveSwarmChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("BeehiveSwarmChildActor"));
	BeehiveSwarmChildActor->SetupAttachment(Root);

	AttractionSwarmNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttractionSwarmNiagara"));
	AttractionSwarmNiagara->SetupAttachment(Root);

	CombRackRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CombRackRoot"));
	CombRackRoot->SetupAttachment(Root);

	CombActorClass = ABeehiveCombActor::StaticClass();
}

void ABeehive::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureDualSwarmChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
}

void ABeehive::BeginPlay()
{
	Super::BeginPlay();
	EnsureDualSwarmChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();

	if (UWorld* World = GetWorld())
	{
		if (UGameTimeBucketSubsystem* BucketSubsystem = World->GetSubsystem<UGameTimeBucketSubsystem>())
		{
			BucketSubsystem->RegisterListener(this);
		}
	}
}

void ABeehive::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameTimeBucketSubsystem* BucketSubsystem = World->GetSubsystem<UGameTimeBucketSubsystem>())
		{
			BucketSubsystem->UnregisterListener(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ABeehive::ApplyBeeSwarmSettings()
{
	ApplySettingsToDualSwarmChildActor();
}

void ABeehive::ApplyBeeSwarmHour24(float Hour24)
{
	BeeSwarmHour24 = NormalizeHour24(Hour24);
	ApplySettingsToDualSwarmChildActor();
	UE_LOG(LogBeekeepingBeeSwarm, Log, TEXT("%s BeeSwarmHour24 set to %f"), *GetName(), BeeSwarmHour24);
}

void ABeehive::ApplyAttractionSwarmSettings()
{
	if (!AttractionSwarmNiagara)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing AttractionSwarmNiagara component."), *GetName());
		return;
	}

	AttractionSwarmNiagara->SetVariableFloat(BeehiveAttractionSwarmNames::AttractionPower, FMath::Max(0.0f, AttractionSwarmSettings.AttractionPower));
	AttractionSwarmNiagara->SetVariableFloat(BeehiveAttractionSwarmNames::NoisePower, FMath::Max(0.0f, AttractionSwarmSettings.NoisePower));
	AttractionSwarmNiagara->SetVariableFloat(BeehiveAttractionSwarmNames::SpawnSphereRadius, FMath::Max(0.0f, AttractionSwarmSettings.SpawnSphereRadius));
	AttractionSwarmNiagara->SetVariableInt(BeehiveAttractionSwarmNames::SpawnAmount, CalculateAttractionSwarmSpawnAmount());
}

int32 ABeehive::CalculateAttractionSwarmSpawnAmount() const
{
	const int32 BeeCount = FMath::Max(0, ColonyBeeCount);
	const float SpawnScale = FMath::Max(0.0f, AttractionSwarmSettings.SpawnAmountScale);
	const int32 MaxSpawnAmount = FMath::Max(0, AttractionSwarmSettings.MaxSpawnAmount);
	const float RawSpawnAmount = static_cast<float>(BeeCount) * SpawnScale;
	const int32 RoundedSpawnAmount = FMath::RoundToInt(RawSpawnAmount);
	return FMath::Clamp(RoundedSpawnAmount, 0, MaxSpawnAmount);
}

void ABeehive::SetColonyBeeCount(int32 NewBeeCount)
{
	ColonyBeeCount = FMath::Max(0, NewBeeCount);
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombSpawnAmounts();
}

void ABeehive::IncreaseCurrentCombCountForTest()
{
	SetCurrentCombCountForTest(CurrentCombCount + 1);
}

void ABeehive::DecreaseCurrentCombCountForTest()
{
	SetCurrentCombCountForTest(CurrentCombCount - 1);
}

void ABeehive::SetCurrentCombCountForTest(int32 NewCount)
{
	CurrentCombCount = NewCount;
	ClampCurrentCombCount();
	bCombCountInitialized = true;
	RefreshCombLayoutAndParameters();
}

int32 ABeehive::CalculateCombSpawnAmount() const
{
	if (CurrentCombCount <= 0)
	{
		return 0;
	}

	const int32 SafeBeeCount = FMath::Max(0, ColonyBeeCount);
	const float SpawnRatio = FMath::Clamp(CombSpawnAmountRatio, 0.0f, 1.0f);
	const float RawSpawnAmount = static_cast<float>(SafeBeeCount) * SpawnRatio / static_cast<float>(CurrentCombCount);
	return FMath::Max(0, FMath::RoundToInt(RawSpawnAmount));
}

void ABeehive::ReduceAllCombTargetBeeCountsByConfiguredRatio()
{
	for (int32 Index = 0; Index < CurrentCombCount; ++Index)
	{
		ReduceCombTargetBeeCountByConfiguredRatio(Index);
	}
}

void ABeehive::ReduceCombTargetBeeCountByConfiguredRatio(int32 CombIndex)
{
	if (!CombSlotComponents.IsValidIndex(CombIndex))
	{
		return;
	}

	UChildActorComponent* Slot = CombSlotComponents[CombIndex];
	if (!Slot)
	{
		return;
	}

	ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Slot->GetChildActor());
	if (!CombActor)
	{
		return;
	}

	const float ClampedRatio = FMath::Clamp(CombTargetBeeCountReduceRatio, 0.0f, 1.0f);
	CombActor->ReduceTargetBeeCountByRatio(ClampedRatio);
}

void ABeehive::GetGameTimeBucketSubscriptions_Implementation(TArray<FGameTimeBucketSubscription>& OutSubscriptions) const
{
	FGameTimeBucketSubscription Subscription;
	Subscription.BucketMinutes = FMath::Clamp(BeeSwarmBucketMinutes, 1, 1440);
	Subscription.bApplyImmediatelyOnBeginPlay = bApplyBeeSwarmOnBeginPlayBucket;
	Subscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
	Subscription.SubscriptionTag = FName(TEXT("BeeSwarm"));
	OutSubscriptions.Add(Subscription);
}

void ABeehive::OnGameTimeBucketEvent_Implementation(const FGameTimeBucketEvent& Event)
{
	if (Event.SubscriptionTag == FName(TEXT("BeeSwarm")))
	{
		ApplyBeeSwarmHour24(Event.Hour24);
	}
}

float ABeehive::NormalizeHour24(float Hour24)
{
	const float Wrapped = FMath::Fmod(Hour24, 24.0f);
	return Wrapped < 0.0f ? Wrapped + 24.0f : Wrapped;
}

float ABeehive::EvaluateActivity(const FBeehiveDirectionalSwarmSettings& Settings, float Hour24)
{
	if (!Settings.SpawnAmountByHour)
	{
		return FMath::Max(0.0f, Settings.FallbackActivity);
	}

	const float NormalizedTime = NormalizeHour24(Hour24) / 24.0f;
	return FMath::Max(0.0f, Settings.SpawnAmountByHour->GetFloatValue(NormalizedTime));
}

FBeehiveDualSwarmNiagaraParameters ABeehive::BuildDualSwarmParameters() const
{
	FBeehiveDualSwarmNiagaraParameters Parameters;
	Parameters.StartShapeExtent = DualSwarmCommonSettings.StartShapeExtent;
	Parameters.EndShapeExtent = DualSwarmCommonSettings.EndShapeExtent;

	const float BeeCount = static_cast<float>(FMath::Max(0, ColonyBeeCount));
	const float SpawnScale = FMath::Max(0.0f, DualSwarmCommonSettings.SpawnAmountScale);
	const float MaxSpawn = FMath::Max(0.0f, DualSwarmCommonSettings.MaxSpawnAmount);

	const float OutgoingActivity = EvaluateActivity(OutgoingSwarmSettings, BeeSwarmHour24);
	const float OutgoingSpawn = OutgoingActivity * BeeCount * SpawnScale;
	Parameters.OutgoingSpawnAmount = FMath::Clamp(OutgoingSpawn, 0.0f, MaxSpawn);
	Parameters.OutgoingSpeedMin = FMath::Max(0.0f, OutgoingSwarmSettings.SpeedRange.SpeedMin);
	Parameters.OutgoingSpeedMax = FMath::Max(0.0f, OutgoingSwarmSettings.SpeedRange.SpeedMax);

	const float IngoingActivity = EvaluateActivity(IngoingSwarmSettings, BeeSwarmHour24);
	const float IngoingSpawn = IngoingActivity * BeeCount * SpawnScale;
	Parameters.IngoingSpawnAmount = FMath::Clamp(IngoingSpawn, 0.0f, MaxSpawn);
	Parameters.IngoingSpeedMin = FMath::Max(0.0f, IngoingSwarmSettings.SpeedRange.SpeedMin);
	Parameters.IngoingSpeedMax = FMath::Max(0.0f, IngoingSwarmSettings.SpeedRange.SpeedMax);

	return Parameters;
}

void ABeehive::ApplySettingsToDualSwarmChildActor()
{
	if (!BeehiveSwarmChildActor)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing BeehiveSwarmChildActor component."), *GetName());
		return;
	}

	ABeehiveDualSwarmActor* DualSwarmActor = Cast<ABeehiveDualSwarmActor>(BeehiveSwarmChildActor->GetChildActor());
	if (!DualSwarmActor)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s has no valid ABeehiveDualSwarmActor child instance."), *GetName());
		return;
	}

	DualSwarmActor->ApplySwarmSpline(SwarmSpline);
	DualSwarmActor->ApplyDualSwarmParameters(BuildDualSwarmParameters());
}

void ABeehive::EnsureDualSwarmChildActorClass()
{
	if (!BeehiveSwarmChildActor || !BeeSplineSwarmActorClass)
	{
		return;
	}

	if (BeehiveSwarmChildActor->GetChildActorClass() != BeeSplineSwarmActorClass)
	{
		BeehiveSwarmChildActor->SetChildActorClass(BeeSplineSwarmActorClass);
	}
}

void ABeehive::RefreshCombLayoutAndParameters()
{
	if (IsTemplate())
	{
		return;
	}

	MaxCombCount = FMath::Max(0, MaxCombCount);
	if (!bCombCountInitialized)
	{
		CurrentCombCount = MaxCombCount;
		bCombCountInitialized = true;
	}
	ClampCurrentCombCount();
	RefreshCombSlotComponents();
	RefreshCombSlotTransforms();
	RefreshCombSpawnAmounts();
}

void ABeehive::RefreshCombSlotComponents()
{
	if (!CombRackRoot)
	{
		return;
	}

	CombSlotComponents.Reset();
	TArray<UChildActorComponent*> DiscoveredSlots;
	TInlineComponentArray<UChildActorComponent*> ChildActorComponents(this);
	for (UChildActorComponent* ExistingComponent : ChildActorComponents)
	{
		if (!ExistingComponent)
		{
			continue;
		}

		if (ExistingComponent->GetAttachParent() != CombRackRoot)
		{
			continue;
		}

		const FString ComponentName = ExistingComponent->GetName();
		if (ComponentName.StartsWith(BeehiveCombSlotNames::DeprecatedSlotPrefix))
		{
			ExistingComponent->SetChildActorClass(nullptr);
			ExistingComponent->DestroyComponent();
			continue;
		}

		if (!ComponentName.StartsWith(BeehiveCombSlotNames::SlotPrefix))
		{
			continue;
		}

		DiscoveredSlots.Add(ExistingComponent);
	}

	DiscoveredSlots.Sort([](const UChildActorComponent& A, const UChildActorComponent& B)
	{
		return A.GetFName().LexicalLess(B.GetFName());
	});

	for (UChildActorComponent* ExistingSlot : DiscoveredSlots)
	{
		const FString ComponentName = ExistingSlot->GetName();
		int32 SlotIndex = INDEX_NONE;
		if (!ParseCombSlotIndex(ComponentName, BeehiveCombSlotNames::SlotPrefix, SlotIndex))
		{
			ExistingSlot->SetChildActorClass(nullptr);
			ExistingSlot->DestroyComponent();
			continue;
		}

		if (SlotIndex < 0 || SlotIndex >= MaxCombCount)
		{
			ExistingSlot->SetChildActorClass(nullptr);
			ExistingSlot->DestroyComponent();
			continue;
		}

		if (ExistingSlot->CreationMethod == EComponentCreationMethod::UserConstructionScript || IsTemplateCombSlotComponent(ExistingSlot))
		{
			ExistingSlot->SetChildActorClass(nullptr);
			ExistingSlot->DestroyComponent();
			continue;
		}

		ExistingSlot->CreationMethod = EComponentCreationMethod::Instance;
		CombSlotComponents.Add(ExistingSlot);
	}

	for (int32 Index = 0; Index < MaxCombCount; ++Index)
	{
		const FName SlotName(*FString::Printf(TEXT("%s%d"), BeehiveCombSlotNames::SlotPrefix, Index));
		UChildActorComponent* Slot = FindObjectFast<UChildActorComponent>(this, SlotName);
		if (!Slot)
		{
			Slot = NewObject<UChildActorComponent>(this, SlotName);
			if (!Slot)
			{
				continue;
			}

			Slot->CreationMethod = EComponentCreationMethod::Instance;
			Slot->SetupAttachment(CombRackRoot);
			AddInstanceComponent(Slot);
			Slot->RegisterComponent();
			Slot->SetMobility(EComponentMobility::Movable);
		}
		else if (Slot->GetAttachParent() != CombRackRoot)
		{
			Slot->AttachToComponent(CombRackRoot, FAttachmentTransformRules::KeepRelativeTransform);
		}

		CombSlotComponents.AddUnique(Slot);
	}

	CombSlotComponents.Sort([](const UChildActorComponent& A, const UChildActorComponent& B)
	{
		return A.GetFName().LexicalLess(B.GetFName());
	});

	if (CombSlotComponents.Num() > MaxCombCount)
	{
		for (int32 Index = CombSlotComponents.Num() - 1; Index >= MaxCombCount; --Index)
		{
			if (UChildActorComponent* Slot = CombSlotComponents[Index])
			{
				Slot->SetChildActorClass(nullptr);
				Slot->DestroyComponent();
			}
			CombSlotComponents.RemoveAt(Index);
		}
	}

	const TSubclassOf<AActor> ActiveCombClass = CombActorClass ? TSubclassOf<AActor>(CombActorClass) : TSubclassOf<AActor>(ABeehiveCombActor::StaticClass());
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		UChildActorComponent* Slot = CombSlotComponents[Index];
		if (!Slot)
		{
			continue;
		}

		const bool bShouldBeActive = Index < CurrentCombCount;
		const TSubclassOf<AActor> DesiredClass = (bShouldBeActive ? ActiveCombClass : TSubclassOf<AActor>(nullptr));
		if (Slot->GetChildActorClass() != DesiredClass)
		{
			Slot->SetChildActorClass(DesiredClass);
		}
	}
}

void ABeehive::RefreshCombSlotTransforms()
{
	const int32 SafeMaxCombCount = FMath::Max(0, MaxCombCount);
	if (SafeMaxCombCount <= 0)
	{
		return;
	}

	const float HalfSpan = static_cast<float>(SafeMaxCombCount - 1) * 0.5f * CombSlotSpacing;
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		if (UChildActorComponent* Slot = CombSlotComponents[Index])
		{
			const float SlotY = -HalfSpan + (static_cast<float>(Index) * CombSlotSpacing);
			const FVector RelativeLocation(0.0f, SlotY, 0.0f);
			Slot->SetRelativeLocation(RelativeLocation);
			Slot->SetRelativeRotation(FRotator::ZeroRotator);
			Slot->SetRelativeScale3D(FVector::OneVector);
		}
	}
}

void ABeehive::RefreshCombSpawnAmounts()
{
	const int32 SpawnAmount = CalculateCombSpawnAmount();
	for (int32 Index = 0; Index < CurrentCombCount; ++Index)
	{
		if (!CombSlotComponents.IsValidIndex(Index))
		{
			continue;
		}

		UChildActorComponent* Slot = CombSlotComponents[Index];
		if (!Slot)
		{
			continue;
		}

		ABeehiveCombActor* CombActor = Cast<ABeehiveCombActor>(Slot->GetChildActor());
		if (!CombActor)
		{
			continue;
		}

		CombActor->SetSpawnAmountAndResetTargetBeeCount(CombPlaneSize, SpawnAmount);
	}
}

void ABeehive::ClampCurrentCombCount()
{
	CurrentCombCount = FMath::Clamp(CurrentCombCount, 0, MaxCombCount);
}

void ABeehive::OnFocusEnter_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	ReceiveFocusEntered(InteractingCharacter);
}

void ABeehive::OnFocusExit_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	ReceiveFocusExited(InteractingCharacter);
}

void ABeehive::OnFocusConfirm_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	bIsLidOpen = true;
	ReceiveFocusConfirmed(InteractingCharacter);
}

void ABeehive::OnFocusCancel_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	bIsLidOpen = false;
	ReceiveFocusCanceled(InteractingCharacter);
}

#if WITH_EDITOR
void ABeehive::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureDualSwarmChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
}
#endif
