// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldActors/Beehive.h"

#include "Environment/GameTimeBucketSubsystem.h"
#include "Focus/AnchoredFocusCursorActionComponent.h"
#include "Components/ChildActorComponent.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Focus/CursorItemUseAreaScopeComponent.h"
#include "Focus/ItemUseAreaMeshProviderComponent.h"
#include "Focus/ChildCursorPartFocusProviderComponent.h"
#include "Focus/CursorPartFocusActionComponent.h"
#include "Focus/CursorPartFocusRegistrationComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Focus/CursorPartFocusScopeComponent.h"
#include "WorldActors/BeehiveDualSwarmActor.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/BeehiveCombSlotActor.h"
#include "WorldActors/BeehiveCombLiftComponent.h"
#include "WorldActors/ItemPlacementSlot.h"
#include "WorldActors/QueenBeeActor.h"
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

	QueenBeeChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("QueenBeeChildActor"));
	QueenBeeChildActor->SetupAttachment(Root);

	CombRackRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CombRackRoot"));
	CombRackRoot->SetupAttachment(Root);

	CombLiftTargetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CombLiftTargetRoot"));
	CombLiftTargetRoot->SetupAttachment(Root);

	CombLiftComponent = CreateDefaultSubobject<UBeehiveCombLiftComponent>(TEXT("CombLiftComponent"));

	CombActorClass = ABeehiveCombActor::StaticClass();
	CombSlotActorClass = ABeehiveCombSlotActor::StaticClass();
	QueenBeeActorClass = AQueenBeeActor::StaticClass();

	CursorPartFocusScope = CreateDefaultSubobject<UCursorPartFocusScopeComponent>(TEXT("CursorPartFocusScope"));
	LidPartFocusAction = CreateDefaultSubobject<UCursorPartFocusActionComponent>(TEXT("LidPartFocusAction"));
	CursorPartFocusRegistration = CreateDefaultSubobject<UCursorPartFocusRegistrationComponent>(TEXT("CursorPartFocusRegistration"));
	ChildCursorPartFocusProvider = CreateDefaultSubobject<UChildCursorPartFocusProviderComponent>(TEXT("ChildCursorPartFocusProvider"));
	ItemUseAreaScope = CreateDefaultSubobject<UCursorItemUseAreaScopeComponent>(TEXT("ItemUseAreaScope"));
	ItemUseAreaMeshProvider = CreateDefaultSubobject<UItemUseAreaMeshProviderComponent>(TEXT("ItemUseAreaMeshProvider"));
	if (LidPartFocusAction)
	{
		LidPartFocusAction->SetEngageMode(ECursorPartFocusEngageMode::PersistentAction);
		FGameplayTagContainer ProvidedTags;
		ProvidedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Beehive.LidOpen")), false));
		LidPartFocusAction->SetProvidedStateTags(ProvidedTags);
	}
}

void ABeehive::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureDualSwarmChildActorClass();
	EnsureQueenBeeChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
	RebuildCursorPartFocusDescriptors();
}

void ABeehive::BeginPlay()
{
	Super::BeginPlay();
	EnsureDualSwarmChildActorClass();
	EnsureQueenBeeChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
	ApplyInitialCombSetupForBeginPlay();
	RebuildCursorPartFocusDescriptors();

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

	const int32 SpawnAmount = CalculateAttractionSwarmSpawnAmount();
	AttractionSwarmNiagara->SetVariableInt(BeehiveAttractionSwarmNames::SpawnAmount, SpawnAmount);
	if (LastAppliedAttractionSwarmSpawnAmount != INDEX_NONE && LastAppliedAttractionSwarmSpawnAmount != SpawnAmount)
	{
		AttractionSwarmNiagara->ReinitializeSystem();
	}
	LastAppliedAttractionSwarmSpawnAmount = SpawnAmount;
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
	RefreshCombSpawnAmounts(false);
}

void ABeehive::ApplyColonyPopulationUpdate()
{
	const float IncreaseAmount = CalculateBeeIncreaseAmount();
	const float DecreaseAmount = CalculateBeeDecreaseAmount();
	const float RawNewBeeCount = static_cast<float>(FMath::Max(0, ColonyBeeCount)) + IncreaseAmount - DecreaseAmount;
	const int32 NewBeeCount = FMath::Max(0, FMath::RoundToInt(RawNewBeeCount));

	ColonyBeeCount = NewBeeCount;
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombSpawnAmounts(true);
}

float ABeehive::CalculateBeeIncreaseAmount() const
{
	const AQueenBeeActor* QueenBee = GetQueenBeeActor();
	const float BaseEggLayingPower = QueenBee ? FMath::Max(0.0f, QueenBee->GetBaseEggLayingPower()) : 0.0f;
	const float ItemEggLayingBonus = FMath::Max(0.0f, GetItemEggLayingBonus());
	const float TemperatureScore = FMath::Max(0.0f, GetTemperatureScore());
	const float IncreaseCoefficient = FMath::Max(0.0f, BeeIncreaseCoefficient);
	return BaseEggLayingPower * ItemEggLayingBonus * TemperatureScore * IncreaseCoefficient;
}

float ABeehive::CalculateBeeDecreaseAmount() const
{
	const float CurrentBeeCount = static_cast<float>(FMath::Max(0, ColonyBeeCount));
	const float DecreaseCoefficient = FMath::Max(0.0f, BeeDecreaseCoefficient);
	const float ItemLifespanBonus = FMath::Max(KINDA_SMALL_NUMBER, GetItemLifespanBonus());
	const float TemperatureScore = FMath::Max(KINDA_SMALL_NUMBER, GetTemperatureScore());
	return CurrentBeeCount * DecreaseCoefficient / ItemLifespanBonus / TemperatureScore;
}

float ABeehive::GetItemEggLayingBonus() const
{
	return 1.0f;
}

float ABeehive::GetItemLifespanBonus() const
{
	return 1.0f;
}

float ABeehive::GetTemperatureScore() const
{
	return 1.0f;
}

void ABeehive::ApplyHoneyProductionUpdate()
{
	const float TotalHoneyIncrease = CalculateTotalHoneyIncreaseAmount();
	DistributeHoneyIncreaseToCombs(TotalHoneyIncrease);
}

float ABeehive::CalculateTotalHoneyIncreaseAmount() const
{
	const int32 SafeBeeCount = FMath::Max(0, ColonyBeeCount);
	const float SafeCoefficient = FMath::Max(0.0f, HoneyProductionCoefficient);
	return static_cast<float>(SafeBeeCount) * SafeCoefficient;
}

void ABeehive::IncreaseSanitation(float Delta)
{
	if (Delta <= 0.0f)
	{
		return;
	}

	SetSanitationValue(SanitationValue + Delta);
}

void ABeehive::SetSanitationValue(float NewValue)
{
	const float SafeMax = FMath::Max(0.0f, MaxSanitationValue);
	SanitationValue = FMath::Clamp(NewValue, 0.0f, SafeMax);
}

float ABeehive::GetSanitationRatio() const
{
	const float SafeMax = FMath::Max(0.0f, MaxSanitationValue);
	if (SafeMax <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(SanitationValue / SafeMax, 0.0f, 1.0f);
}

void ABeehive::RebuildCursorPartFocusDescriptors()
{
	if (!CursorPartFocusScope)
	{
		return;
	}

	CursorPartFocusScope->ClearRegisteredParts();

	UPrimitiveComponent* LidComponent = FindPrimitiveComponentByTag(LidPartComponentTag);
	if (!LidComponent)
	{
		LidComponent = BeehiveMesh;
	}

	if (LidComponent)
	{
		FCursorPartFocusPartDescriptor LidDescriptor;
		LidDescriptor.PartId = FName(TEXT("Beehive.Lid"));
		LidDescriptor.OwnerActor = this;
		LidDescriptor.HitComponent = LidComponent;
		LidDescriptor.OutlineComponents.AddUnique(LidComponent);
		if (!LidOutlineComponentTag.IsNone())
		{
			LidDescriptor.OutlineComponentTags.Add(LidOutlineComponentTag);
		}
		LidDescriptor.ActionHandler = LidPartFocusAction;
		LidDescriptor.EngageMode = LidPartFocusAction ? LidPartFocusAction->GetEngageMode() : ECursorPartFocusEngageMode::PersistentAction;
		LidDescriptor.PromptData.bIsValid = true;
		LidDescriptor.PromptData.DisplayName = LidPartDisplayName.IsEmpty() ? FText::FromString(TEXT("Lid")) : LidPartDisplayName;
		LidDescriptor.PromptData.InteractionKeyText = LidPartInteractionKeyText.IsEmpty() ? FText::FromString(TEXT("Click")) : LidPartInteractionKeyText;
		CursorPartFocusScope->RegisterPartDescriptor(LidDescriptor);
	}

	RegisterCombPartsToScope();

	if (PreviewOnlyPartComponentTags.Num() > 0)
	{
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}

			bool bMatches = false;
			for (const FName& Tag : PreviewOnlyPartComponentTags)
			{
				if (!Tag.IsNone() && PrimitiveComponent->ComponentHasTag(Tag))
				{
					bMatches = true;
					break;
				}
			}

			if (!bMatches)
			{
				continue;
			}

			FCursorPartFocusPartDescriptor PreviewOnlyDescriptor;
			PreviewOnlyDescriptor.PartId = FName(*FString::Printf(TEXT("PreviewOnly.%s"), *PrimitiveComponent->GetName()));
			PreviewOnlyDescriptor.OwnerActor = this;
			PreviewOnlyDescriptor.HitComponent = PrimitiveComponent;
			PreviewOnlyDescriptor.OutlineComponents.Add(PrimitiveComponent);
			PreviewOnlyDescriptor.EngageMode = ECursorPartFocusEngageMode::PreviewOnly;
			PreviewOnlyDescriptor.PromptData.bIsValid = false;
			CursorPartFocusScope->RegisterPartDescriptor(PreviewOnlyDescriptor);
		}
	}

	if (CursorPartFocusRegistration)
	{
		CursorPartFocusRegistration->AppendCursorPartFocusDescriptorsToScope();
	}
}

void ABeehive::SetLidOpenForPartFocus(bool bOpen)
{
	if (bIsLidOpen == bOpen)
	{
		return;
	}

	bIsLidOpen = bOpen;
	ReceiveLidPartFocusStateChanged(bIsLidOpen);
}

void ABeehive::RefreshCombStateFromSlots()
{
	RefreshCurrentCombCountFromSlots();

	if (CombLiftComponent && CombLiftComponent->GetLiftedCombSlotIndex() != INDEX_NONE && !GetLiftedCombActor())
	{
		CombLiftComponent->ReturnAllLiftedCombs();
	}

	RefreshCombSpawnAmounts(false);
	RebuildCursorPartFocusDescriptors();
	RebuildItemUseAreaDescriptorsIfAvailable();
}

int32 ABeehive::FindManagedCombSlotIndex(const ABeehiveCombActor* CombActor) const
{
	if (!CombActor)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		const ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		if (SlotActor && SlotActor->GetPlacedCombActor() == CombActor)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

UChildActorComponent* ABeehive::GetCombSlotComponentByIndex(int32 Index) const
{
	if (!CombSlotComponents.IsValidIndex(Index))
	{
		return nullptr;
	}

	return CombSlotComponents[Index];
}

ABeehiveCombSlotActor* ABeehive::GetCombSlotActorByIndex(int32 Index) const
{
	const UChildActorComponent* SlotComponent = GetCombSlotComponentByIndex(Index);
	return SlotComponent ? Cast<ABeehiveCombSlotActor>(SlotComponent->GetChildActor()) : nullptr;
}

int32 ABeehive::GetOccupiedCombCount() const
{
	int32 OccupiedCount = 0;
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		const ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		if (SlotActor && SlotActor->GetPlacedCombActor())
		{
			++OccupiedCount;
		}
	}

	return OccupiedCount;
}

ABeehiveCombActor* ABeehive::GetLiftedCombActor() const
{
	const int32 LiftedSlotIndex = CombLiftComponent ? CombLiftComponent->GetLiftedCombSlotIndex() : INDEX_NONE;
	if (LiftedSlotIndex == INDEX_NONE)
	{
		return nullptr;
	}

	const ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(LiftedSlotIndex);
	return SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
}

bool ABeehive::GetCombSlotWorldTransformByIndex(int32 Index, FTransform& OutTransform) const
{
	const UChildActorComponent* Slot = GetCombSlotComponentByIndex(Index);
	if (!Slot)
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	OutTransform = Slot->GetComponentTransform();
	return true;
}

bool ABeehive::BuildCombSlotRestRelativeTransform(int32 Index, FTransform& OutTransform) const
{
	if (Index < 0)
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	const int32 SafeMaxCombCount = FMath::Max(0, MaxCombCount);
	if (SafeMaxCombCount <= 0)
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	const float HalfSpan = static_cast<float>(SafeMaxCombCount - 1) * 0.5f * CombSlotSpacing;
	const float SlotY = -HalfSpan + (static_cast<float>(Index) * CombSlotSpacing);
	OutTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, SlotY, 0.0f), FVector::OneVector);
	return true;
}

int32 ABeehive::CalculateCombSpawnAmount() const
{
	const int32 OccupiedCombCount = GetOccupiedCombCount();
	if (OccupiedCombCount <= 0)
	{
		return 0;
	}

	const int32 SafeBeeCount = FMath::Max(0, ColonyBeeCount);
	const float SpawnRatio = FMath::Clamp(CombSpawnAmountRatio, 0.0f, 1.0f);
	const float RawSpawnAmount = static_cast<float>(SafeBeeCount) * SpawnRatio / static_cast<float>(OccupiedCombCount);
	return FMath::Max(0, FMath::RoundToInt(RawSpawnAmount));
}

void ABeehive::ReduceAllCombTargetBeeCountsByConfiguredRatio()
{
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		ReduceCombTargetBeeCountByConfiguredRatio(Index);
	}
}

void ABeehive::ReduceCombTargetBeeCountByConfiguredRatio(int32 CombIndex)
{
	ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(CombIndex);
	ABeehiveCombActor* CombActor = SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
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

	FGameTimeBucketSubscription QueenSubscription;
	QueenSubscription.BucketMinutes = FMath::Clamp(QueenBeeLocationBucketMinutes, 1, 1440);
	QueenSubscription.bApplyImmediatelyOnBeginPlay = bUpdateQueenBeeLocationOnBeginPlayBucket;
	QueenSubscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
	QueenSubscription.SubscriptionTag = FName(TEXT("QueenBeeLocation"));
	OutSubscriptions.Add(QueenSubscription);

	FGameTimeBucketSubscription HoneySubscription;
	HoneySubscription.BucketMinutes = FMath::Clamp(HoneyProductionBucketMinutes, 1, 1440);
	HoneySubscription.bApplyImmediatelyOnBeginPlay = bApplyHoneyProductionOnBeginPlayBucket;
	HoneySubscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
	HoneySubscription.SubscriptionTag = FName(TEXT("HoneyProduction"));
	OutSubscriptions.Add(HoneySubscription);

	FGameTimeBucketSubscription PopulationSubscription;
	PopulationSubscription.BucketMinutes = FMath::Clamp(ColonyPopulationBucketMinutes, 1, 1440);
	PopulationSubscription.bApplyImmediatelyOnBeginPlay = bApplyColonyPopulationOnBeginPlayBucket;
	PopulationSubscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
	PopulationSubscription.SubscriptionTag = FName(TEXT("ColonyPopulation"));
	OutSubscriptions.Add(PopulationSubscription);
}

void ABeehive::OnGameTimeBucketEvent_Implementation(const FGameTimeBucketEvent& Event)
{
	if (Event.SubscriptionTag == FName(TEXT("BeeSwarm")))
	{
		ApplyBeeSwarmHour24(Event.Hour24);
	}
	else if (Event.SubscriptionTag == FName(TEXT("QueenBeeLocation")))
	{
		UpdateQueenBeeLocation();
	}
	else if (Event.SubscriptionTag == FName(TEXT("HoneyProduction")))
	{
		ApplyHoneyProductionUpdate();
	}
	else if (Event.SubscriptionTag == FName(TEXT("ColonyPopulation")))
	{
		ApplyColonyPopulationUpdate();
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

	ClampCombAuthoringCounts();
	RefreshCombSlotComponents();
	RefreshCombSlotTransforms();
	RefreshCurrentCombCountFromSlots();
	RefreshCombSpawnAmounts(false);
	RebuildCursorPartFocusDescriptors();
}

void ABeehive::ApplyInitialCombSetupForBeginPlay()
{
	if (IsTemplate())
	{
		return;
	}

	ApplyInitialCombCountToSlots();
	RefreshCurrentCombCountFromSlots();
	RefreshCombSpawnAmounts(false);
	RebuildCursorPartFocusDescriptors();
	RebuildItemUseAreaDescriptorsIfAvailable();
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

	const TSubclassOf<AActor> ActiveSlotClass = CombSlotActorClass ? TSubclassOf<AActor>(CombSlotActorClass) : TSubclassOf<AActor>(ABeehiveCombSlotActor::StaticClass());
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		UChildActorComponent* Slot = CombSlotComponents[Index];
		if (!Slot)
		{
			continue;
		}

		if (Slot->GetChildActorClass() != ActiveSlotClass)
		{
			Slot->SetChildActorClass(ActiveSlotClass);
		}
	}
}

void ABeehive::EnsureQueenBeeChildActorClass()
{
	if (!QueenBeeChildActor || !QueenBeeActorClass)
	{
		return;
	}

	if (QueenBeeChildActor->GetChildActorClass() != QueenBeeActorClass)
	{
		QueenBeeChildActor->SetChildActorClass(QueenBeeActorClass);
	}
}

void ABeehive::UpdateQueenBeeLocation()
{
	EnsureQueenBeeChildActorClass();
	if (!QueenBeeChildActor || !QueenBeeChildActor->GetChildActor())
	{
		return;
	}

	int32 SelectedSlotIndex = INDEX_NONE;
	if (!ChooseQueenBeeCombSlotIndex(SelectedSlotIndex))
	{
		return;
	}

	const bool bFrontFace = FMath::RandBool();
	USceneComponent* AttachPoint = ResolveQueenBeeAttachPoint(SelectedSlotIndex, bFrontFace);
	if (!AttachPoint)
	{
		return;
	}

	QueenBeeChildActor->AttachToComponent(AttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	QueenBeeChildActor->SetRelativeLocation(FVector::ZeroVector);
	const float RandomYaw = FMath::FRandRange(0.0f, 360.0f);
	QueenBeeChildActor->SetRelativeRotation(FRotator(0.0f, RandomYaw, 0.0f));
}

AQueenBeeActor* ABeehive::GetQueenBeeActor() const
{
	return QueenBeeChildActor ? Cast<AQueenBeeActor>(QueenBeeChildActor->GetChildActor()) : nullptr;
}

bool ABeehive::IsQueenBeeAttachedToComb(const ABeehiveCombActor* CombActor) const
{
	if (!CombActor || !QueenBeeChildActor)
	{
		return false;
	}

	const USceneComponent* AttachParent = QueenBeeChildActor->GetAttachParent();
	if (!AttachParent)
	{
		return false;
	}

	if (AttachParent == CombActor->GetQueenFrontAttachPoint() || AttachParent == CombActor->GetQueenBackAttachPoint())
	{
		return true;
	}

	return AttachParent->GetOwner() == CombActor;
}

bool ABeehive::ChooseQueenBeeCombSlotIndex(int32& OutSlotIndex) const
{
	OutSlotIndex = INDEX_NONE;

	const int32 LiftedSlotIndex = CombLiftComponent ? CombLiftComponent->GetLiftedCombSlotIndex() : INDEX_NONE;

	struct FWeightedSlot
	{
		int32 Index = INDEX_NONE;
		float Weight = 0.0f;
	};

	TArray<FWeightedSlot> WeightedSlots;
	float TotalWeight = 0.0f;

	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		if (Index == LiftedSlotIndex)
		{
			continue;
		}

		const ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		const ABeehiveCombActor* CombActor = SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
		if (!CombActor)
		{
			continue;
		}

		if (!CombActor->GetQueenFrontAttachPoint() || !CombActor->GetQueenBackAttachPoint())
		{
			continue;
		}

		const float Weight = CalculateQueenBeeCombSlotWeight(Index);
		if (Weight <= 0.0f)
		{
			continue;
		}

		FWeightedSlot& Candidate = WeightedSlots.AddDefaulted_GetRef();
		Candidate.Index = Index;
		Candidate.Weight = Weight;
		TotalWeight += Weight;
	}

	if (WeightedSlots.Num() == 0 || TotalWeight <= 0.0f)
	{
		return false;
	}

	float Remaining = FMath::FRandRange(0.0f, TotalWeight);
	for (const FWeightedSlot& Candidate : WeightedSlots)
	{
		Remaining -= Candidate.Weight;
		if (Remaining <= 0.0f)
		{
			OutSlotIndex = Candidate.Index;
			return true;
		}
	}

	OutSlotIndex = WeightedSlots.Last().Index;
	return OutSlotIndex != INDEX_NONE;
}

float ABeehive::CalculateQueenBeeCombSlotWeight(int32 SlotIndex) const
{
	const int32 SlotCount = CombSlotComponents.Num();
	if (SlotCount <= 0)
	{
		return 0.0f;
	}

	const float Center = static_cast<float>(SlotCount - 1) * 0.5f;
	const float MaxDistance = FMath::Max(Center, static_cast<float>(SlotCount - 1) - Center);
	const float Distance = FMath::Abs(static_cast<float>(SlotIndex) - Center);
	const float Distance01 = MaxDistance > KINDA_SMALL_NUMBER ? Distance / MaxDistance : 0.0f;
	const float CenterFactor = 1.0f - FMath::Clamp(Distance01, 0.0f, 1.0f);
	return FMath::Lerp(1.0f, FMath::Max(1.0f, QueenBeeCenterWeightMultiplier), CenterFactor);
}

USceneComponent* ABeehive::ResolveQueenBeeAttachPoint(int32 SlotIndex, bool bFrontFace) const
{
	const ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(SlotIndex);
	const ABeehiveCombActor* CombActor = SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
	return CombActor ? CombActor->GetQueenAttachPoint(bFrontFace) : nullptr;
}

void ABeehive::DistributeHoneyIncreaseToCombs(float TotalHoneyIncrease)
{
	if (TotalHoneyIncrease <= 0.0f)
	{
		return;
	}

	TArray<ABeehiveCombActor*> ActiveCombs;
	ActiveCombs.Reserve(CombSlotComponents.Num());
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		ABeehiveCombActor* CombActor = SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
		if (CombActor)
		{
			ActiveCombs.Add(CombActor);
		}
	}

	if (ActiveCombs.Num() <= 0)
	{
		return;
	}

	const float ClampedDeviation = FMath::Clamp(HoneyDistributionDeviationRatio, 0.0f, 1.0f);
	const float MinWeight = 1.0f - ClampedDeviation;
	const float MaxWeight = 1.0f + ClampedDeviation;

	TArray<float> Weights;
	Weights.Reserve(ActiveCombs.Num());
	float WeightSum = 0.0f;
	for (int32 Index = 0; Index < ActiveCombs.Num(); ++Index)
	{
		const float Weight = FMath::FRandRange(MinWeight, MaxWeight);
		Weights.Add(Weight);
		WeightSum += Weight;
	}

	if (WeightSum <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (int32 Index = 0; Index < ActiveCombs.Num(); ++Index)
	{
		const float HoneyIncrease = TotalHoneyIncrease * Weights[Index] / WeightSum;
		ActiveCombs[Index]->AddHoneyAmount(HoneyIncrease);
	}
}

void ABeehive::ApplyInitialCombCountToSlots()
{
	const TSubclassOf<AActor> DesiredCombClass = CombActorClass ? TSubclassOf<AActor>(CombActorClass) : TSubclassOf<AActor>(ABeehiveCombActor::StaticClass());

	const int32 TargetCount = FMath::Clamp(InitialCombCount, 0, CombSlotComponents.Num());
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		if (!SlotActor)
		{
			continue;
		}

		ABeehiveCombActor* PlacedComb = SlotActor->GetPlacedCombActor();
		if (Index < TargetCount)
		{
			if (!PlacedComb)
			{
				IItemPlacementSlot::Execute_TryPlaceItem(SlotActor, DesiredCombClass, nullptr, nullptr);
			}
		}
		else if (PlacedComb)
		{
			IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
		}
	}
}

void ABeehive::RefreshCombSlotTransforms()
{
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		if (UChildActorComponent* Slot = CombSlotComponents[Index])
		{
			FTransform RestRelativeTransform;
			if (BuildCombSlotRestRelativeTransform(Index, RestRelativeTransform))
			{
				Slot->SetRelativeTransform(RestRelativeTransform);
			}
		}
	}

	if (CombLiftComponent)
	{
		CombLiftComponent->ReapplyLiftedCombTransformAfterLayoutRefresh();
	}
}

void ABeehive::RefreshCombSpawnAmounts(bool bSkipLiftedComb)
{
	const int32 SpawnAmount = CalculateCombSpawnAmount();
	const int32 LiftedSlotIndex = (bSkipLiftedComb && CombLiftComponent) ? CombLiftComponent->GetLiftedCombSlotIndex() : INDEX_NONE;
	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		if (Index == LiftedSlotIndex)
		{
			continue;
		}

		ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		ABeehiveCombActor* CombActor = SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
		if (!CombActor)
		{
			continue;
		}

		CombActor->SetSpawnAmountAndResetTargetBeeCount(CombPlaneSize, SpawnAmount);
	}
}

void ABeehive::ClampCombAuthoringCounts()
{
	MaxCombCount = FMath::Max(0, MaxCombCount);
	InitialCombCount = FMath::Clamp(InitialCombCount, 0, MaxCombCount);
	RefreshCurrentCombCountFromSlots();
}

void ABeehive::RefreshCurrentCombCountFromSlots()
{
	CurrentCombCount = GetOccupiedCombCount();
}

void ABeehive::RegisterCombPartsToScope()
{
	if (!CursorPartFocusScope)
	{
		return;
	}

	for (int32 Index = 0; Index < CombSlotComponents.Num(); ++Index)
	{
		ABeehiveCombSlotActor* SlotActor = GetCombSlotActorByIndex(Index);
		ABeehiveCombActor* CombActor = SlotActor ? SlotActor->GetPlacedCombActor() : nullptr;
		if (!CombActor)
		{
			continue;
		}

		FCursorPartFocusPartDescriptor CombDescriptor;
		CombDescriptor.PartId = FName(*FString::Printf(TEXT("Beehive.Comb.%d"), Index));
		CombDescriptor.OwnerActor = CombActor;
		CombDescriptor.HitComponent = CombActor->GetCombMeshComponent();
		if (UPrimitiveComponent* Mesh = CombActor->GetCombMeshComponent())
		{
			CombDescriptor.OutlineComponents.Add(Mesh);
		}
		CombDescriptor.ActionHandler = CombActor->GetPartFocusActionComponent();
		BindCombPartFocusActionDelegates(CombActor, CombDescriptor.ActionHandler);
		CombDescriptor.EngageMode = CombDescriptor.ActionHandler ? CombDescriptor.ActionHandler->GetEngageMode() : ECursorPartFocusEngageMode::PersistentAction;
		if (CombDescriptor.ActionHandler)
		{
			CombDescriptor.RequiredStateTags = CombDescriptor.ActionHandler->GetRequiredStateTags();
		}
		CombDescriptor.PromptData.bIsValid = true;
		CombDescriptor.PromptData.DisplayName = FText::FromString(TEXT("Comb"));
		CombDescriptor.PromptData.InteractionKeyText = FText::FromString(TEXT("Click"));
		CursorPartFocusScope->RegisterPartDescriptor(CombDescriptor);
	}
}

void ABeehive::BindCombPartFocusActionDelegates(ABeehiveCombActor* CombActor, UCursorPartFocusActionComponent* ActionComponent)
{
	if (!CombActor || !ActionComponent)
	{
		return;
	}

	ActionComponent->OnPartFocusBegin.RemoveDynamic(this, &ABeehive::HandleCombPartFocusBegin);
	ActionComponent->OnPartFocusCancel.RemoveDynamic(this, &ABeehive::HandleCombPartFocusCancel);
	ActionComponent->OnPartFocusAbort.RemoveDynamic(this, &ABeehive::HandleCombPartFocusAbort);

	ActionComponent->OnPartFocusBegin.AddDynamic(this, &ABeehive::HandleCombPartFocusBegin);
	ActionComponent->OnPartFocusCancel.AddDynamic(this, &ABeehive::HandleCombPartFocusCancel);
	ActionComponent->OnPartFocusAbort.AddDynamic(this, &ABeehive::HandleCombPartFocusAbort);
}

bool ABeehive::IsManagedActiveCombActor(const ABeehiveCombActor* CombActor) const
{
	return FindManagedCombSlotIndex(CombActor) != INDEX_NONE;
}

void ABeehive::HandleCombPartFocusBegin(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	ABeehiveCombActor* CombActor = ActionComponent ? Cast<ABeehiveCombActor>(ActionComponent->GetOwner()) : nullptr;
	if (!CombActor || !IsManagedActiveCombActor(CombActor))
	{
		return;
	}

	if (CombLiftComponent)
	{
		CombLiftComponent->LiftComb(CombActor, InteractingCharacter);
	}
	RebuildItemUseAreaDescriptorsIfAvailable();

	ReceiveCombPartFocusBegin(CombActor, InteractingCharacter);
}

void ABeehive::HandleCombPartFocusCancel(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	ABeehiveCombActor* CombActor = ActionComponent ? Cast<ABeehiveCombActor>(ActionComponent->GetOwner()) : nullptr;
	if (!CombActor || !IsManagedActiveCombActor(CombActor))
	{
		return;
	}

	if (CombLiftComponent)
	{
		CombLiftComponent->ReturnComb(CombActor);
	}
	RebuildItemUseAreaDescriptorsIfAvailable();

	ReceiveCombPartFocusCancel(CombActor, InteractingCharacter);
}

void ABeehive::HandleCombPartFocusAbort(UCursorPartFocusActionComponent* ActionComponent, UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter)
{
	ABeehiveCombActor* CombActor = ActionComponent ? Cast<ABeehiveCombActor>(ActionComponent->GetOwner()) : nullptr;
	if (!CombActor || !IsManagedActiveCombActor(CombActor))
	{
		return;
	}

	if (CombLiftComponent)
	{
		CombLiftComponent->AbortCombLift(CombActor);
	}
	RebuildItemUseAreaDescriptorsIfAvailable();

	ReceiveCombPartFocusAbort(CombActor, InteractingCharacter);
}

void ABeehive::RebuildItemUseAreaDescriptorsIfAvailable()
{
	if (ItemUseAreaScope)
	{
		ItemUseAreaScope->RebuildItemUseAreaDescriptors();
	}
}

UPrimitiveComponent* ABeehive::FindPrimitiveComponentByTag(FName ComponentTag) const
{
	if (ComponentTag.IsNone())
	{
		return nullptr;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent && PrimitiveComponent->ComponentHasTag(ComponentTag))
		{
			return PrimitiveComponent;
		}
	}

	return nullptr;
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
	ReceiveFocusConfirmed(InteractingCharacter);
}

void ABeehive::OnFocusCancel_Implementation(ABeekeeperCharacter* InteractingCharacter)
{
	ReceiveFocusCanceled(InteractingCharacter);
}

#if WITH_EDITOR
void ABeehive::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureDualSwarmChildActorClass();
	EnsureQueenBeeChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
	RebuildCursorPartFocusDescriptors();
}

bool ABeehive::CanEditChange(const FProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty))
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const bool bIsGameWorld = World && World->IsGameWorld();
	if (!bIsGameWorld || !InProperty)
	{
		return true;
	}

	const FName PropertyName = InProperty->GetFName();
	return PropertyName != GET_MEMBER_NAME_CHECKED(ABeehive, InitialCombCount)
		&& PropertyName != GET_MEMBER_NAME_CHECKED(ABeehive, MaxCombCount)
		&& PropertyName != GET_MEMBER_NAME_CHECKED(ABeehive, CombActorClass)
		&& PropertyName != GET_MEMBER_NAME_CHECKED(ABeehive, CombSlotActorClass)
		&& PropertyName != GET_MEMBER_NAME_CHECKED(ABeehive, CombSlotSpacing);
}
#endif
