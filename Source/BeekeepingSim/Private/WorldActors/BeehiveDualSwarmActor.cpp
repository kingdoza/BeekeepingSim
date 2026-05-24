#include "WorldActors/BeehiveDualSwarmActor.h"

#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"

namespace BeehiveDualSwarmActorNames
{
	static const FName SwarmSpline(TEXT("User.SwarmSpline"));
	static const FName SplineLength(TEXT("User.SplineLength"));
	static const FName StartShapeExtent(TEXT("User.StartShapeExtent"));
	static const FName EndShapeExtent(TEXT("User.EndShapeExtent"));
	static const FName SpawnAmount(TEXT("User.SpawnAmount"));
	static const FName SpeedMin(TEXT("User.SpeedMin"));
	static const FName SpeedMax(TEXT("User.SpeedMax"));
	static const FName IsReverse(TEXT("User.bIsReverse"));
}

ABeehiveDualSwarmActor::ABeehiveDualSwarmActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	OutgoingNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OutgoingNiagara"));
	OutgoingNiagara->SetupAttachment(Root);

	IngoingNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IngoingNiagara"));
	IngoingNiagara->SetupAttachment(Root);
}

void ABeehiveDualSwarmActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplySplineBindings();
}

void ABeehiveDualSwarmActor::BeginPlay()
{
	Super::BeginPlay();
	ApplySplineBindings();
}

void ABeehiveDualSwarmActor::ApplyDualSwarmParameters(const FBeehiveDualSwarmNiagaraParameters& Parameters)
{
	ApplySplineBindings();

	if (!OutgoingNiagara || !IngoingNiagara)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing niagara components. Outgoing:%s Ingoing:%s"), *GetName(), OutgoingNiagara ? TEXT("Valid") : TEXT("Null"), IngoingNiagara ? TEXT("Valid") : TEXT("Null"));
		return;
	}

	float OutgoingSpeedMin = FMath::Max(0.0f, Parameters.OutgoingSpeedMin);
	float OutgoingSpeedMax = FMath::Max(0.0f, Parameters.OutgoingSpeedMax);
	if (OutgoingSpeedMin > OutgoingSpeedMax)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s outgoing speed range has SpeedMin > SpeedMax. Values will be swapped. Min:%f Max:%f"), *GetName(), OutgoingSpeedMin, OutgoingSpeedMax);
		Swap(OutgoingSpeedMin, OutgoingSpeedMax);
	}

	float IngoingSpeedMin = FMath::Max(0.0f, Parameters.IngoingSpeedMin);
	float IngoingSpeedMax = FMath::Max(0.0f, Parameters.IngoingSpeedMax);
	if (IngoingSpeedMin > IngoingSpeedMax)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s ingoing speed range has SpeedMin > SpeedMax. Values will be swapped. Min:%f Max:%f"), *GetName(), IngoingSpeedMin, IngoingSpeedMax);
		Swap(IngoingSpeedMin, IngoingSpeedMax);
	}

	OutgoingNiagara->SetVariableVec3(BeehiveDualSwarmActorNames::StartShapeExtent, Parameters.StartShapeExtent);
	OutgoingNiagara->SetVariableVec3(BeehiveDualSwarmActorNames::EndShapeExtent, Parameters.EndShapeExtent);
	OutgoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SpawnAmount, FMath::Max(0.0f, Parameters.OutgoingSpawnAmount));
	OutgoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SpeedMin, OutgoingSpeedMin);
	OutgoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SpeedMax, OutgoingSpeedMax);

	IngoingNiagara->SetVariableVec3(BeehiveDualSwarmActorNames::StartShapeExtent, Parameters.StartShapeExtent);
	IngoingNiagara->SetVariableVec3(BeehiveDualSwarmActorNames::EndShapeExtent, Parameters.EndShapeExtent);
	IngoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SpawnAmount, FMath::Max(0.0f, Parameters.IngoingSpawnAmount));
	IngoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SpeedMin, IngoingSpeedMin);
	IngoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SpeedMax, IngoingSpeedMax);
}

void ABeehiveDualSwarmActor::ApplySwarmSpline(USplineComponent* InSwarmSpline)
{
	BoundSwarmSpline = InSwarmSpline;
	ApplySplineBindings();
}

void ABeehiveDualSwarmActor::ApplySplineBindings()
{
	if (!OutgoingNiagara || !IngoingNiagara)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s is missing niagara components. Outgoing:%s Ingoing:%s"), *GetName(), OutgoingNiagara ? TEXT("Valid") : TEXT("Null"), IngoingNiagara ? TEXT("Valid") : TEXT("Null"));
		return;
	}

	USplineComponent* SwarmSpline = BoundSwarmSpline.Get();
	if (!SwarmSpline)
	{
		UE_LOG(LogBeekeepingBeeSwarm, Warning, TEXT("%s has no externally bound SwarmSpline."), *GetName());
		return;
	}

	const float SplineLength = GetSplineLength();

	OutgoingNiagara->SetVariableObject(BeehiveDualSwarmActorNames::SwarmSpline, SwarmSpline);
	OutgoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SplineLength, SplineLength);
	OutgoingNiagara->SetVariableBool(BeehiveDualSwarmActorNames::IsReverse, false);

	IngoingNiagara->SetVariableObject(BeehiveDualSwarmActorNames::SwarmSpline, SwarmSpline);
	IngoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::SplineLength, SplineLength);
	IngoingNiagara->SetVariableBool(BeehiveDualSwarmActorNames::IsReverse, true);
}

float ABeehiveDualSwarmActor::GetSplineLength() const
{
	USplineComponent* SwarmSpline = BoundSwarmSpline.Get();
	return SwarmSpline ? SwarmSpline->GetSplineLength() : 0.0f;
}

#if WITH_EDITOR
void ABeehiveDualSwarmActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplySplineBindings();
}
#endif
