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
#include "Focus/CursorPartFocusActionComponent.h"
#include "Focus/FocusTargetComponent.h"
#include "Focus/CursorPartFocusScopeComponent.h"
#include "WorldActors/BeehiveDualSwarmActor.h"
#include "WorldActors/BeehiveCombActor.h"
#include "WorldActors/BeehiveCombLiftComponent.h"
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

	CombLiftTargetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CombLiftTargetRoot"));
	CombLiftTargetRoot->SetupAttachment(Root);

	CombLiftComponent = CreateDefaultSubobject<UBeehiveCombLiftComponent>(TEXT("CombLiftComponent"));

	CombActorClass = ABeehiveCombActor::StaticClass();

	CursorPartFocusScope = CreateDefaultSubobject<UCursorPartFocusScopeComponent>(TEXT("CursorPartFocusScope"));
	LidPartFocusAction = CreateDefaultSubobject<UCursorPartFocusActionComponent>(TEXT("LidPartFocusAction"));
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
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
	RebuildCursorPartFocusDescriptors();
}

void ABeehive::BeginPlay()
{
	Super::BeginPlay();
	EnsureDualSwarmChildActorClass();
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
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

int32 ABeehive::FindManagedCombSlotIndex(const ABeehiveCombActor* CombActor) const
{
	if (!CombActor)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < CurrentCombCount; ++Index)
	{
		if (!CombSlotComponents.IsValidIndex(Index))
		{
			continue;
		}

		const UChildActorComponent* Slot = CombSlotComponents[Index];
		if (Slot && Slot->GetChildActor() == CombActor)
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
	RebuildCursorPartFocusDescriptors();
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

void ABeehive::RegisterCombPartsToScope()
{
	if (!CursorPartFocusScope)
	{
		return;
	}

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

	ReceiveCombPartFocusAbort(CombActor, InteractingCharacter);
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
	ApplyBeeSwarmSettings();
	ApplyAttractionSwarmSettings();
	RefreshCombLayoutAndParameters();
	RebuildCursorPartFocusDescriptors();
}
#endif
