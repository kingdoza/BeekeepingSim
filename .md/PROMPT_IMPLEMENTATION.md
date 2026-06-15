# Swarming route-arrival cluster spawn implementation prompt

## Goal

Change manual swarming test presentation so the route swarm appears first, and the swarm cluster is created only when the route swarm reaches the target.

Current behavior creates the route actor and the cluster actor immediately. The new behavior must be:

1. `ABeehive::BeginSwarmingAtTransform` starts only the route actor.
2. The route actor spline runs from `SwarmExitPoint` to the requested target transform location.
3. The cluster actor is spawned at the target only after the route travel delay expires.
4. Cluster bees and the separate swarm queen are created together by `ABeeSwarmClusterActor::InitializeSwarmClusterFromDensity(...)`.
5. Route new-spawn/emission lasts only `SwarmClusterSpawnAmount / SwarmRouteParameters.SpawnAmount` seconds.
6. The route actor remains alive until the last emitted route bees can reach the target: `RouteArrivalDelaySeconds + RouteEmissionDurationSeconds`.
7. After cluster spawn, the cluster `AliveRadius` grows from `0` to the target radius over `RouteEmissionDurationSeconds`.
8. The swarm cluster host cannot enter FocusEngaged until intro growth is complete.

The user confirmed policy option 1:

- `RouteEmissionDurationSeconds` controls how long the route keeps emitting/new-spawning bees.
- Route actor lifetime is longer, so already-emitted particles can finish traveling after emission stops.

## Required reading

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## Current code premises

- Relevant source files:
  - `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
  - `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
  - `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`
  - `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
  - `Source/BeekeepingSim/Private/WorldActors/BeeSplineSwarmActor.cpp`
  - `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
  - `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `ABeehive::BeginSwarmingAtTransform` currently spawns and initializes `ABeeSwarmClusterActor` immediately.
- `ABeehiveSwarmRouteActor::ConfigureRoute(...)` builds the route spline and `ABeeSplineSwarmActor::GetSplineLength()` returns route length in Unreal centimeters.
- `FBeeSplineSwarmAppliedParameters` contains:
  - `SpawnAmount`
  - `SpeedMin`
  - `SpeedMax`
- `ABeeSplineSwarmActor::ApplyExternalSwarmParameters(...)` applies route Niagara parameters.
- `ABeeSwarmClusterActor::InitializeSwarmClusterFromDensity(int32, float)` already owns cluster initialization, density-derived radius, queen child creation/random rotation, capture use-area activation, and Niagara parameter application.
- `ABeeSwarmClusterActor` capture progression uses bee amount as source of truth and derives `AliveRadius` from volume ratio. The intro growth must not replace `CapturedBeeAmount` as capture state.
- `UFocusTargetComponent` does not currently expose an enabled flag. FocusEngaged availability should be gated through the cluster's focus action `CanBeginFocusAction(...)`, not by assuming component activation disables focus.

## Timing rules

Use two distinct route timing concepts.

```cpp
const float RouteSpawnAmount = FMath::Max(0.0f, SwarmRouteParameters.SpawnAmount);
const float RouteSpeedMin = FMath::Max(0.0f, SwarmRouteParameters.SpeedMin);
const float RouteSpeedMax = FMath::Max(0.0f, SwarmRouteParameters.SpeedMax);
const float AverageRouteSpeed = (RouteSpeedMin + RouteSpeedMax) * 0.5f;
const float SplineLength = RouteActor->GetSplineLength();

RouteArrivalDelaySeconds = SplineLength / AverageRouteSpeed;
RouteEmissionDurationSeconds = float(FMath::Max(0, SwarmClusterSpawnAmount)) / RouteSpawnAmount;
RouteDestroyDelaySeconds = RouteArrivalDelaySeconds + RouteEmissionDurationSeconds;
```

Important:

- `SplineLength / AverageRouteSpeed` is the correct seconds formula.
- Do not use `AverageRouteSpeed / SplineLength`.
- `SplineLength` is in cm and route speed is assumed to be cm/s.
- If `RouteSpawnAmount <= 0`, fail swarming start and call `ReceiveSwarmingStartFailed()`.
- If `AverageRouteSpeed <= 0`, fail swarming start and call `ReceiveSwarmingStartFailed()`.
- If `SplineLength <= 0`, allow immediate arrival with `RouteArrivalDelaySeconds = 0.0f`.
- `SpeedMin > SpeedMax` does not matter for average speed because the sum is unchanged, but still clamp each to `>= 0`.

## Required behavior

### Start flow

Update `ABeehive::BeginSwarmingAtTransform` to:

1. Resolve `World`, `SwarmRouteActorClass`, and `SwarmClusterActorClass`.
2. Clear any pending swarming timers/session state. If `bDestroyPreviousTestSwarmOnStart` is true, destroy previous active route/cluster actors.
3. Spawn only `ABeehiveSwarmRouteActor` at `SwarmExitPoint` transform.
4. Configure route from `SwarmExitPoint` location to `TargetTransform.GetLocation()`.
5. Apply `SwarmRouteParameters`.
6. Compute `RouteArrivalDelaySeconds`, `RouteEmissionDurationSeconds`, and `RouteDestroyDelaySeconds`.
7. Store the target transform as pending cluster spawn data.
8. Store the active route actor.
9. Start timers for:
   - route arrival / cluster spawn
   - route emission stop
   - route actor cleanup
10. Return true after route start succeeds.

Do not spawn or initialize the cluster during the start call.

### Arrival flow

Add a private helper on `ABeehive`, e.g.

```cpp
void HandleActiveSwarmRouteArrived();
```

On arrival:

1. If the active route/session is no longer valid, do nothing.
2. Spawn `ABeeSwarmClusterActor` at the pending target transform.
3. Call:

```cpp
ClusterActor->InitializeSwarmClusterFromDensity(
    SwarmClusterSpawnAmount,
    SwarmClusterBeeDensityPerCubicMeter);
```

4. Set `ActiveSwarmClusterActor`.
5. Start cluster intro growth for `RouteEmissionDurationSeconds`.
6. Call the existing `ReceiveSwarmingStarted(ClusterActor, ActiveSwarmRouteActor)`.

Keep `ReceiveSwarmingStarted` as the cluster-created event. This preserves the expectation that its `ClusterActor` parameter is non-null.

### Cluster intro growth

Add an explicit intro-growth path on `ABeeSwarmClusterActor`, e.g.

```cpp
UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Intro")
void BeginAliveRadiusIntroGrowth(float DurationSeconds);

UFUNCTION(BlueprintCallable, Category = "Bee Swarm Cluster|Intro")
void FinishAliveRadiusIntroGrowth();

UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Intro")
bool IsAliveRadiusIntroGrowthActive() const;
```

The arrival handler should call:

```cpp
ClusterActor->InitializeSwarmClusterFromDensity(
    SwarmClusterSpawnAmount,
    SwarmClusterBeeDensityPerCubicMeter);
ClusterActor->BeginAliveRadiusIntroGrowth(ActiveSwarmRouteEmissionDurationSeconds);
```

Required intro behavior:

1. `SpawnAmount` is the final target bee amount immediately.
2. `InitialAliveRadius` is the final target full radius immediately.
3. `SphereRadius` is the final target radius immediately.
4. Niagara `User.SpawnAmount` is set to the final target amount immediately.
5. Niagara `User.SphereRadius` is set to the final target radius immediately.
6. Only `AliveRadius` starts at `0.0f` and grows over time.

The growth is linear in bee count, not linear in radius.

```cpp
const float GrowthAlpha = FMath::Clamp(ElapsedSeconds / DurationSeconds, 0.0f, 1.0f);
const float VisualBeeAmount = static_cast<float>(FMath::Max(0, SpawnAmount)) * GrowthAlpha;
const float VisualBeeRatio = SpawnAmount > 0
    ? FMath::Clamp(VisualBeeAmount / static_cast<float>(SpawnAmount), 0.0f, 1.0f)
    : 0.0f;
AliveRadius = InitialAliveRadius * FMath::Pow(VisualBeeRatio, 1.0f / 3.0f);
```

At `GrowthAlpha=1`, `AliveRadius` must equal `InitialAliveRadius` and the intro growth is complete.

Suggested cluster state:

```cpp
UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Intro")
bool bAliveRadiusIntroGrowthActive = false;

UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Intro")
float AliveRadiusIntroGrowthDurationSeconds = 0.0f;

UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Intro")
float AliveRadiusIntroGrowthElapsedSeconds = 0.0f;
```

Tick policy:

- `ABeeSwarmClusterActor` currently does not tick.
- Enable tick only while intro growth is active, then disable it again when growth finishes.
- Do not add permanent per-frame tick cost.

Capture policy during intro:

- Keep the swarm cluster FocusEngaged unavailable while intro growth is active.
- Do not add separate BeeCarrier or QueenCage intro gates unless the implementation discovers a direct non-FocusEngaged use path. Those use-areas only matter inside the FocusEngaged item-use-area scope, so host-level FocusEngaged gating is the source of truth.
- BeeCarrier and QueenCage use-area active rules should remain tied to their domain state after FocusEngaged is available: BeeCarrier depends on `!bBeesCaptured`, QueenCage depends on queen capture state.
- `CaptureBees`, `SetCapturedBeeAmount`, and `RefreshAliveRadiusFromBeeAmounts` should finish/cancel intro growth before applying capture math if they are called unexpectedly during intro. Capture math must remain based on `CapturedBeeAmount` and target `SpawnAmount`, not visual intro bee amount.

Zero-duration behavior:

- If `DurationSeconds <= 0`, immediately finish growth and set `AliveRadius = InitialAliveRadius`.
- If `SpawnAmount <= 0`, keep `AliveRadius = 0`, finish growth immediately, and preserve existing zero-bee completion behavior.

Events:

- Keep `ReceiveSwarmClusterInitialized()` as the cluster initialization event.
- Keep `ReceiveAliveRadiusChanged(float)` for each meaningful `AliveRadius` update, including intro growth updates.
- Optional low-risk event:

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Bee Swarm Cluster|Intro")
void ReceiveAliveRadiusIntroGrowthFinished();
```

Do not make intro growth a separate source of truth for final captured/completion state.

### Cluster FocusEngaged availability

FocusEngaged for the swarm cluster must become available only after intro growth finishes.

Do not rely on `UActorComponent::SetActive(false)` unless you verify the Focus system checks component active state. The current focus action path calls `CanBeginFocusAction(...)`, so use an explicit action-level gate.

Recommended implementation:

- Add a small swarm-cluster-specific focus action component, e.g. `USwarmClusterFocusActionComponent : public UAnchoredFocusCursorActionComponent`.
- Use it as `ABeeSwarmClusterActor::FocusAction` instead of the generic `UAnchoredFocusCursorActionComponent`.
- Override `CanBeginFocusAction(ABeekeeperCharacter*) const`.
- Return false while the owning `ABeeSwarmClusterActor` reports intro growth active or FocusEngaged unavailable.

Example shape:

```cpp
bool USwarmClusterFocusActionComponent::CanBeginFocusAction(ABeekeeperCharacter* InteractingCharacter) const
{
    const ABeeSwarmClusterActor* ClusterOwner = Cast<ABeeSwarmClusterActor>(GetOwner());
    return Super::CanBeginFocusAction(InteractingCharacter)
        && ClusterOwner
        && ClusterOwner->IsSwarmClusterFocusEngagedAvailable();
}
```

Add cluster query/setter APIs:

```cpp
UFUNCTION(BlueprintPure, Category = "Bee Swarm Cluster|Intro")
bool IsSwarmClusterFocusEngagedAvailable() const;
```

Rules:

- After `InitializeSwarmClusterFromDensity(...)`, FocusEngaged availability is false if intro growth will run.
- `BeginAliveRadiusIntroGrowth(DurationSeconds)` sets FocusEngaged unavailable until growth finishes.
- `FinishAliveRadiusIntroGrowth()` sets FocusEngaged available, unless the actor is already being destroyed or captured in a state that should block focus.
- If `DurationSeconds <= 0`, FocusEngaged becomes available immediately after the radius is set to target.
- Preview focus can still show a disabled prompt during intro if the Focus system naturally displays disabled entries from `CanBeginFocusAction=false`. Do not add a new prompt system just for this.

### Route emission stop

Add a private helper on `ABeehive`, e.g.

```cpp
void StopActiveSwarmRouteEmission();
```

The route should stop new-spawning after `RouteEmissionDurationSeconds`, but the actor must not be destroyed yet.

Recommended minimal implementation:

- Add an API on `ABeeSplineSwarmActor` or `ABeehiveSwarmRouteActor` to stop emission by applying the same external parameters with `SpawnAmount = 0.0f`.
- Do not call `Destroy()` or `DeactivateImmediate()` at emission-stop time, because existing route particles should be allowed to finish.

Example helper shape:

```cpp
void ABeeSplineSwarmActor::StopExternalSwarmEmission()
{
    FBeeSplineSwarmAppliedParameters Parameters = LastAppliedExternalParameters;
    Parameters.SpawnAmount = 0.0f;
    ApplyExternalSwarmParameters(Parameters);
}
```

If you add `LastAppliedExternalParameters`, keep it internal/transient. Do not expose a new authoring source unless needed.

If the Niagara system does not react to runtime `User.SpawnAmount = 0`, still implement the C++ contract and report that the Niagara asset must consume live `User.SpawnAmount` for emission-stop behavior.

### Route cleanup

Add a private helper on `ABeehive`, e.g.

```cpp
void DestroyActiveSwarmRouteAfterTravel();
```

At `RouteDestroyDelaySeconds`:

- Destroy only the active route actor.
- Set `ActiveSwarmRouteActor = nullptr`.
- Do not destroy `ActiveSwarmClusterActor`.

Rationale: route actor lifetime represents all emitted route bees reaching the target. Cluster capture gameplay continues separately.

### Clear/end play

Update `ABeehive::ClearActiveTestSwarm(bool bDestroyActors)`:

- Clear all active route timers.
- Clear pending target transform/session state.
- If `bDestroyActors` is true, destroy active route and active cluster.
- Set `ActiveSwarmRouteActor = nullptr`.
- Set `ActiveSwarmClusterActor = nullptr`.

Update `EndPlay` to rely on this cleanup and ensure timers are cleared before actor teardown.

## Suggested state additions

In `ABeehive.h`, add transient/private state similar to:

```cpp
UPROPERTY(Transient)
FTransform PendingSwarmClusterTransform;

UPROPERTY(Transient)
bool bHasPendingSwarmClusterTransform = false;

FTimerHandle ActiveSwarmRouteArrivalTimerHandle;
FTimerHandle ActiveSwarmRouteEmissionStopTimerHandle;
FTimerHandle ActiveSwarmRouteDestroyTimerHandle;

float ActiveSwarmRouteArrivalDelaySeconds = 0.0f;
float ActiveSwarmRouteEmissionDurationSeconds = 0.0f;
```

Timer handles do not need `UPROPERTY`.

## Optional Blueprint events

Do not remove or rename existing Blueprint events.

Keep:

```cpp
ReceiveSwarmingStarted(ABeeSwarmClusterActor* ClusterActor, ABeehiveSwarmRouteActor* RouteActor)
ReceiveSwarmingStartFailed()
```

Add only if useful and low-risk:

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Swarming Test")
void ReceiveSwarmingRouteStarted(ABeehiveSwarmRouteActor* RouteActor, float ArrivalDelaySeconds, float EmissionDurationSeconds);

UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Swarming Test")
void ReceiveSwarmingRouteEmissionStopped(ABeehiveSwarmRouteActor* RouteActor);

UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Swarming Test")
void ReceiveSwarmingRouteFinished(ABeehiveSwarmRouteActor* RouteActor);
```

If these are added:

- Call `ReceiveSwarmingRouteStarted` after route timers are scheduled.
- Call `ReceiveSwarmingRouteEmissionStopped` after setting route spawn amount to 0.
- Call `ReceiveSwarmingRouteFinished` immediately before or after route actor destroy. If called after destroy, pass the pointer only if still safe/valid; otherwise prefer before destroy.

## Blueprint/API/Core Redirect impact

Expected Blueprint-facing behavior change:

- `BeginSwarmingAtTransform` returns true when route start succeeds, not when cluster creation succeeds.
- `GetActiveSwarmClusterActor()` returns null until route arrival.
- `ReceiveSwarmingStarted(...)` fires later, when the cluster is actually spawned.

Expected Blueprint-facing additions if optional events are implemented:

- Route started/emission stopped/finished Blueprint events on `ABeehive`.

Do not delete or rename UCLASS/USTRUCT/UENUM/UFUNCTION/UPROPERTY symbols for this task.

Core Redirect:

- No Core Redirect should be needed.
- Do not edit `Config/DefaultEngine.ini`.

## Documentation updates

Update `.md/0_ARCHITECTURE.md`:

- Replace wording that says swarming start immediately spawns cluster and route.
- Document delayed cluster spawn at route arrival.
- Document arrival delay formula:

```text
RouteArrivalDelaySeconds = RouteSplineLength / Avg(SwarmRouteParameters.SpeedMin, SwarmRouteParameters.SpeedMax)
```

- Document route emission duration:

```text
RouteEmissionDurationSeconds = SwarmClusterSpawnAmount / SwarmRouteParameters.SpawnAmount
```

- Document route actor cleanup delay:

```text
RouteDestroyDelaySeconds = RouteArrivalDelaySeconds + RouteEmissionDurationSeconds
```

- Document cluster intro growth:

```text
ClusterIntroGrowthDurationSeconds = RouteEmissionDurationSeconds
IntroVisualBeeAmount = SwarmClusterSpawnAmount * Clamp(Elapsed / ClusterIntroGrowthDurationSeconds, 0..1)
AliveRadius = InitialAliveRadius * cbrt(IntroVisualBeeAmount / SwarmClusterSpawnAmount)
```

- Document that `SpawnAmount` and `SphereRadius` are applied at target values immediately when the cluster actor is spawned, while `AliveRadius` alone grows from `0`.

Update `.md/Architecture/WorldActorsSystem.md`:

- `ABeehive` composition/state list:
  - active route timer/session state
  - pending cluster transform
- Swarming test success flow.
- `ABeehiveSwarmRouteActor` section:
  - route actor remains alive after emission stop until last emitted route bees can reach target
  - route does not own cluster creation
- `ABeeSwarmClusterActor` section:
  - cluster is spawned only from `ABeehive` route-arrival handling
  - initialization still creates cluster bees and swarm queen together
  - intro growth keeps target `SpawnAmount`/`SphereRadius` immediately applied and grows only `AliveRadius`
  - intro growth uses linear visual bee amount with cube-root radius scaling
  - host FocusEngaged is unavailable until intro growth finishes
  - BeeCarrier/QueenCage use-areas do not need separate intro gates because they are only usable inside FocusEngaged
- Add an `Update 2026-06-15` note for route-arrival cluster spawn.

Do not update unrelated systems.

## Validation

Run diff checks:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

Run searches:

```powershell
rg -n "BeginSwarmingAtTransform|ReceiveSwarmingStarted|GetActiveSwarmClusterActor|ActiveSwarmRoute|PendingSwarmCluster|RouteArrivalDelay|RouteEmissionDuration|AliveRadiusIntroGrowth|SwarmClusterFocus" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
rg -n "SwarmClusterSpawnAmount / SwarmRouteParameters\.SpawnAmount|SplineLength / AverageRouteSpeed|AverageRouteSpeed / SplineLength|IntroVisualBeeAmount|VisualBeeAmount|Pow\(.*1\.0f / 3\.0f" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that build could not be run.

## Manual PIE checks

1. Start swarming from a beehive.
2. Confirm route actor appears immediately.
3. Confirm no cluster actor exists before route arrival delay.
4. Confirm cluster actor appears after `SplineLength / Avg(SpeedMin, SpeedMax)` seconds.
5. Confirm cluster initialization creates both cluster bees and the separate swarm queen at the same time.
6. At cluster spawn, confirm `User.SpawnAmount` and `User.SphereRadius` are already at target values.
7. At cluster spawn, confirm `User.AliveRadius` starts at `0`.
8. Confirm `AliveRadius` grows for `RouteEmissionDurationSeconds`.
9. Confirm `AliveRadius` follows cube-root volume scaling. For example, at half intro time it should be about `InitialAliveRadius * cbrt(0.5)`, not `InitialAliveRadius * 0.5`.
10. Confirm the swarm cluster cannot enter FocusEngaged during intro growth.
11. Confirm the swarm cluster can enter FocusEngaged after intro growth finishes.
12. Confirm BeeCarrier and QueenCage use-areas work after FocusEngaged becomes available, using their existing captured-state rules.
13. Confirm route new-spawning stops after `SwarmClusterSpawnAmount / SwarmRouteParameters.SpawnAmount` seconds.
14. Confirm route actor is destroyed after `ArrivalDelay + EmissionDuration`.
15. Confirm cluster remains after route actor cleanup and BeeCarrier/QueenCage capture still works.
16. Confirm `GetActiveSwarmClusterActor()` is null before arrival and valid after arrival.
17. Test invalid route `SpawnAmount=0` and invalid route speed `0/0`; both should fail cleanly via `ReceiveSwarmingStartFailed()`.

## Final report requirements

- Changed files
- Exact arrival/emission/destroy timing formulas
- Exact cluster intro growth formula
- Whether optional route Blueprint events were added
- Whether optional cluster intro Blueprint events were added
- Whether route emission stop is implemented by `User.SpawnAmount=0`
- Whether cluster FocusEngaged is disabled during intro growth
- Confirmation that BeeCarrier/QueenCage use-areas were not separately gated unless a non-FocusEngaged use path was found
- Blueprint behavior change notes
- Architecture document updates
- Validation commands and results
- Manual PIE checks still required
