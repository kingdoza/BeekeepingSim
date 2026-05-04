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
#include "Curves/CurveFloat.h"

namespace BeehiveAttractionSwarmNames
{
	static const FName AttractionPower(TEXT("User.AttractionPower"));
	static const FName NoisePower(TEXT("User.NoisePower"));
	static const FName SpawnSphereRadius(TEXT("User.SpawnSphereRadius"));
	static const FName SpawnAmount(TEXT("User.SpawnAmount"));
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
}

void ABeehive::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureDualSwarmChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
}

void ABeehive::BeginPlay()
{
	Super::BeginPlay();
	EnsureDualSwarmChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();

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
}
#endif
