#include "WorldActors/BeeSplineSwarmActor.h"

#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogBeekeepingBeeSwarm);

namespace BeeSplineSwarmActorNames
{
	static const FName StartShapeExtent(TEXT("User.StartShapeExtent"));
	static const FName EndShapeExtent(TEXT("User.EndShapeExtent"));
	static const FName SpawnAmount(TEXT("User.SpawnAmount"));
	static const FName SpeedMin(TEXT("User.SpeedMin"));
	static const FName SpeedMax(TEXT("User.SpeedMax"));
	static const FName SplineLength(TEXT("User.SplineLength"));
	static const FName SwarmSpline(TEXT("User.SwarmSpline"));
}

ABeeSplineSwarmActor::ABeeSplineSwarmActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SwarmSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SwarmSpline"));
	SwarmSpline->SetupAttachment(Root);

	SwarmNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SwarmNiagara"));
	SwarmNiagara->SetupAttachment(Root);
}

void ABeeSplineSwarmActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!ShouldApplyParametersInEditor())
	{
		return;
	}
#endif

	ApplySplineLengthParameter();

	if (ControlMode == EBeeSplineSwarmControlMode::ExternalControlled && !bWarnedMissingExternalParameters)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is ExternalControlled but no external parameters were applied yet."), *GetName());
		bWarnedMissingExternalParameters = true;
	}
}

void ABeeSplineSwarmActor::BeginPlay()
{
	Super::BeginPlay();
	ApplySplineLengthParameter();
}

void ABeeSplineSwarmActor::SetControlMode(EBeeSplineSwarmControlMode NewMode)
{
	ControlMode = NewMode;
	ApplySplineLengthParameter();
}

void ABeeSplineSwarmActor::ApplyExternalSwarmParameters(const FBeeSplineSwarmAppliedParameters& Parameters)
{
	SetControlMode(EBeeSplineSwarmControlMode::ExternalControlled);
	LastAppliedExternalParameters = Parameters;
	bHasLastAppliedExternalParameters = true;
	ApplyExternalSwarmParametersInternal(Parameters);
}

void ABeeSplineSwarmActor::StopExternalSwarmEmission()
{
	FBeeSplineSwarmAppliedParameters Parameters = bHasLastAppliedExternalParameters
		? LastAppliedExternalParameters
		: FBeeSplineSwarmAppliedParameters();
	Parameters.SpawnAmount = 0.0f;
	ApplyExternalSwarmParameters(Parameters);
}

void ABeeSplineSwarmActor::ApplySplineLengthParameter()
{
	if (!SwarmNiagara)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing SwarmNiagara component."), *GetName());
		return;
	}

	if (!SwarmSpline)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing SwarmSpline component."), *GetName());
	}

	SwarmNiagara->SetVariableObject(BeeSplineSwarmActorNames::SwarmSpline, SwarmSpline);
	SwarmNiagara->SetVariableFloat(BeeSplineSwarmActorNames::SplineLength, GetSplineLength());
}

float ABeeSplineSwarmActor::GetSplineLength() const
{
	return SwarmSpline ? SwarmSpline->GetSplineLength() : 0.0f;
}

void ABeeSplineSwarmActor::ApplySwarmParameters()
{
	ApplySplineLengthParameter();
}

void ABeeSplineSwarmActor::ApplyHour24(float Hour24)
{
	ApplySplineLengthParameter();
}

void ABeeSplineSwarmActor::ApplyOverrideSettings(const FBeeSplineSwarmOverrideSettings& OverrideSettings, float Hour24)
{
	(void)OverrideSettings;
	(void)Hour24;
	UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s ApplyOverrideSettings is deprecated and ignored in control-mode workflow."), *GetName());
	ApplySplineLengthParameter();
}

void ABeeSplineSwarmActor::ClearOverrideSettings(float Hour24)
{
	(void)Hour24;
	UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s ClearOverrideSettings is deprecated and ignored in control-mode workflow."), *GetName());
	ApplySplineLengthParameter();
}

void ABeeSplineSwarmActor::ApplyExternalSwarmParametersInternal(const FBeeSplineSwarmAppliedParameters& Parameters)
{
	if (!SwarmNiagara)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing SwarmNiagara component."), *GetName());
		return;
	}

	float SpeedMin = FMath::Max(0.0f, Parameters.SpeedMin);
	float SpeedMax = FMath::Max(0.0f, Parameters.SpeedMax);
	if (SpeedMin > SpeedMax)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s has SpeedMin > SpeedMax in external parameters. Values will be swapped. Min:%f Max:%f"), *GetName(), SpeedMin, SpeedMax);
		Swap(SpeedMin, SpeedMax);
	}

	SwarmNiagara->SetVariableVec3(BeeSplineSwarmActorNames::StartShapeExtent, Parameters.StartShapeExtent);
	SwarmNiagara->SetVariableVec3(BeeSplineSwarmActorNames::EndShapeExtent, Parameters.EndShapeExtent);
	SwarmNiagara->SetVariableFloat(BeeSplineSwarmActorNames::SpawnAmount, FMath::Max(0.0f, Parameters.SpawnAmount));
	SwarmNiagara->SetVariableFloat(BeeSplineSwarmActorNames::SpeedMin, SpeedMin);
	SwarmNiagara->SetVariableFloat(BeeSplineSwarmActorNames::SpeedMax, SpeedMax);
	SwarmNiagara->SetVariableObject(BeeSplineSwarmActorNames::SwarmSpline, SwarmSpline);
	SwarmNiagara->SetVariableFloat(BeeSplineSwarmActorNames::SplineLength, GetSplineLength());

	bWarnedMissingExternalParameters = false;
}

#if WITH_EDITOR
bool ABeeSplineSwarmActor::ShouldApplyParametersInEditor() const
{
	if (IsTemplate())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return World->WorldType != EWorldType::EditorPreview;
}

void ABeeSplineSwarmActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!ShouldApplyParametersInEditor())
	{
		return;
	}

	ApplySplineLengthParameter();
}
#endif
