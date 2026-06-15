# Colony swarming implementation prompt

## Goal

Add a real colony swarming API while preserving the existing state-neutral manual/test swarming API.

Confirmed direction:

- Existing `ABeehive::BeginSwarmingAtTransform(...)` and `BeginSwarmingAtActor(...)` remain test/presentation APIs.
- These existing APIs must keep their current contract: no queen removal, no `ColonyBeeCount` mutation, no active comb bee-count mutation.
- Add separate real colony swarming APIs that use the same route-arrival cluster presentation but apply actual hive state impact.

Real colony swarming behavior:

1. The source hive loses its queen.
2. The source hive loses a random `Min~Max` percentage of its current `ColonyBeeCount`.
3. The removed bee count becomes the spawned swarm cluster bee count.
4. Route emission duration is based on the removed bee count, not the authored test `SwarmClusterSpawnAmount`.

## Required reading

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## Relevant source files

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`

## Current code premises

- `ABeehive::BeginSwarmingAtTransform` currently starts a route-only session and spawns the cluster at route arrival.
- `ABeehive::HandleActiveSwarmRouteArrived` currently initializes the cluster with `SwarmClusterSpawnAmount`.
- Route emission duration is currently computed as:

```cpp
RouteEmissionDurationSeconds = SwarmClusterSpawnAmount / SwarmRouteParameters.SpawnAmount;
```

- `SetColonyBeeCount(...)` is the correct source-of-truth mutation path for colony bee count because it refreshes:
  - dual swarm settings
  - attraction swarm settings
  - active comb spawn amounts while preserving target ratios
- `SetHasQueenBee(false)` is the correct source-of-truth mutation path for removing the hive queen because it clears/rebuilds the queen child actor and item-use-area descriptors.
- Current architecture explicitly says manual swarming test does not modify `ColonyBeeCount`, `QueenBeeChildActor`, active comb bee count/target count, or bucket subscriptions. This must remain true for the existing test APIs.

## Public API additions

Add separate real colony swarming APIs on `ABeehive`.

Recommended names:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Colony Swarming")
bool BeginColonySwarmingAtTransform(const FTransform& TargetTransform);

UFUNCTION(BlueprintCallable, Category = "Beehive|Colony Swarming")
bool BeginColonySwarmingAtActor(AActor* TargetActor);
```

Do not rename or delete:

```cpp
BeginSwarmingAtTransform(...)
BeginSwarmingAtActor(...)
```

Those remain manual/test APIs.

## New authored settings

Add real swarming loss ratio settings to `ABeehive`.

Recommended names:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Swarming", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float ColonySwarmingBeeLossRatioMin = 0.3f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Swarming", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float ColonySwarmingBeeLossRatioMax = 0.6f;
```

Rules:

- Values represent `0.0~1.0`, not `0~100`.
- Clamp both to `0.0~1.0` before use.
- If min is greater than max, sort them at calculation time rather than failing.
- If both effectively produce zero removed bees, real swarming start should fail cleanly.

## Shared start flow

Do not duplicate the entire route-start implementation.

Refactor into a private shared helper with a policy/input struct or clear parameters.

Recommended shape:

```cpp
enum class EBeehiveSwarmingStartMode : uint8
{
    TestPresentation,
    ColonyImpact
};

struct FBeehiveSwarmingStartOptions
{
    EBeehiveSwarmingStartMode Mode = EBeehiveSwarmingStartMode::TestPresentation;
    int32 ClusterSpawnAmount = 0;
    bool bApplyColonyImpact = false;
};

bool BeginSwarmingAtTransformInternal(const FTransform& TargetTransform, const FBeehiveSwarmingStartOptions& Options);
```

Simple alternatives are acceptable, but keep these invariants:

- Existing test APIs call the shared helper with:
  - `ClusterSpawnAmount = SwarmClusterSpawnAmount`
  - `bApplyColonyImpact = false`
- New colony APIs call the shared helper with:
  - `ClusterSpawnAmount = OutgoingBeeCount`
  - `bApplyColonyImpact = true`
- Route timing and cluster arrival use the session cluster spawn amount, not always `SwarmClusterSpawnAmount`.

## Real swarming start validation

`BeginColonySwarmingAtTransform(...)` must validate before route start:

```cpp
if (!bHasQueenBee)
{
    NotifySwarmingStartFailed();
    return false;
}

if (ColonyBeeCount <= 0)
{
    NotifySwarmingStartFailed();
    return false;
}
```

Then compute outgoing bee count:

```cpp
const int32 PreSwarmBeeCount = FMath::Max(0, ColonyBeeCount);
const float ClampedMin = FMath::Clamp(ColonySwarmingBeeLossRatioMin, 0.0f, 1.0f);
const float ClampedMax = FMath::Clamp(ColonySwarmingBeeLossRatioMax, 0.0f, 1.0f);
const float LossRatioMin = FMath::Min(ClampedMin, ClampedMax);
const float LossRatioMax = FMath::Max(ClampedMin, ClampedMax);
const float LossRatio = FMath::FRandRange(LossRatioMin, LossRatioMax);
const int32 OutgoingBeeCount = FMath::Clamp(
    FMath::RoundToInt(static_cast<float>(PreSwarmBeeCount) * LossRatio),
    0,
    PreSwarmBeeCount);
```

If `OutgoingBeeCount <= 0`, fail cleanly with `NotifySwarmingStartFailed()` and return false.

Do not remove the queen if outgoing bee count is zero.

## Colony impact commit timing

Apply colony state impact only after route actor spawn/config succeeds.

Recommended route:

1. Validate world/classes/session/rates.
2. Compute outgoing bee count for colony mode.
3. Spawn route actor.
4. Configure route and apply route parameters.
5. Compute route timings.
6. Commit colony impact if this is colony mode.
7. Store session state and schedule timers.

The commit should happen before returning true but after route start cannot trivially fail.

Recommended helper:

```cpp
void ApplyColonySwarmingImpact(int32 OutgoingBeeCount);
```

Implementation:

```cpp
void ABeehive::ApplyColonySwarmingImpact(int32 OutgoingBeeCount)
{
    SetColonyBeeCount(FMath::Max(0, ColonyBeeCount - FMath::Max(0, OutgoingBeeCount)));
    SetHasQueenBee(false);
}
```

Do not call `ReduceAllCombTargetBeeCountsByConfiguredRatio()` from this impact path. `SetColonyBeeCount()` already refreshes comb spawn amounts while preserving target ratios. Calling both would double-apply visual/target reduction.

## Session cluster spawn amount

Add transient session state so route timers and arrival logic can use the correct amount.

Recommended:

```cpp
UPROPERTY(Transient)
int32 ActiveSwarmClusterSpawnAmount = 0;
```

Rules:

- Test APIs set this to authored `SwarmClusterSpawnAmount`.
- Colony APIs set this to computed `OutgoingBeeCount`.
- `ClearActiveTestSwarm(...)` or route session cleanup resets it to `0`.
- Route emission duration uses `ActiveSwarmClusterSpawnAmount`.
- Cluster arrival initialization uses `ActiveSwarmClusterSpawnAmount`.

Replace route emission duration calculation with session amount:

```cpp
const int32 SessionClusterSpawnAmount = FMath::Max(0, Options.ClusterSpawnAmount);
const float RouteEmissionDurationSeconds = static_cast<float>(SessionClusterSpawnAmount) / RouteSpawnAmount;
```

At arrival:

```cpp
ClusterActor->InitializeSwarmClusterFromDensityWithIntroGrowth(
    ActiveSwarmClusterSpawnAmount,
    SwarmClusterBeeDensityPerCubicMeter,
    ActiveSwarmRouteEmissionDurationSeconds);
```

## Queen state transfer

Minimum required behavior:

- `SetHasQueenBee(false)` removes the hive queen.
- `ABeeSwarmClusterActor` still creates its own swarm queen through existing `SwarmQueenBeeActorClass`.

Preferred behavior if low-risk:

- Before `SetHasQueenBee(false)`, capture the source hive queen state:

```cpp
FQueenCageItemState OutgoingQueenState;
if (const AQueenBeeActor* QueenBee = GetQueenBeeActor())
{
    OutgoingQueenState = QueenBee->MakeQueenCageItemState();
}
```

- Store it in transient session state.
- Pass it to `ABeeSwarmClusterActor` at arrival.
- Use it to configure the cluster queen class, base egg laying power, and disease value.

Only implement the preferred transfer if it can be done without broad API churn. If it requires new public setters on `AQueenBeeActor`, add only small, clearly named setters and document the Blueprint/API impact.

If not implementing queen state transfer in this pass, explicitly report that colony swarming removes the source queen but the spawned swarm queen still uses `SwarmQueenBeeActorClass` defaults.

## Failure and rollback policy

Before route start success:

- Do not mutate hive state.
- Fail with `NotifySwarmingStartFailed()`.

After route start success and colony impact commit:

- Do not rollback queen/bee count if route timers later fail or cluster spawn fails.
- If route arrival cluster spawn fails, call `NotifySwarmingStartFailed()` but keep the colony impact.

Rationale:

- Once real swarming starts, the queen and bees have left the source colony.
- Rolling back would require restoring queen child actor state, colony count, route state, and presentation state together.

## Existing test API behavior that must remain unchanged

For `BeginSwarmingAtTransform(...)` and `BeginSwarmingAtActor(...)`:

- Do not check `bHasQueenBee`.
- Do not check `ColonyBeeCount`.
- Do not call `SetHasQueenBee(false)`.
- Do not call `SetColonyBeeCount(...)`.
- Continue using authored `SwarmClusterSpawnAmount`.
- Continue allowing test route/cluster presentation without changing hive source-of-truth state.

## Blueprint/API/Core Redirect impact

Expected Blueprint API additions:

- `BeginColonySwarmingAtTransform`
- `BeginColonySwarmingAtActor`
- `ColonySwarmingBeeLossRatioMin`
- `ColonySwarmingBeeLossRatioMax`

No Blueprint API deletion or rename is allowed.

Core Redirect:

- Not required.
- Do not edit `Config/DefaultEngine.ini`.

Blueprint behavior:

- Existing Blueprint calls to `BeginSwarmingAtTransform/Actor` remain test-only and state-neutral.
- Blueprints that want real hive impact must switch to the new `BeginColonySwarmingAtTransform/Actor` calls.

## Documentation updates

Update `.md/0_ARCHITECTURE.md`:

- Keep existing test API contract: no colony/queen/comb state mutation.
- Add new colony swarming API contract:
  - requires queen
  - removes queen on route start success
  - removes random min/max ratio of `ColonyBeeCount`
  - removed bee count drives cluster `SpawnAmount` and route emission duration

Update `.md/Architecture/WorldActorsSystem.md`:

- `ABeehive` composition/settings:
  - add colony swarming loss ratio settings
  - add session cluster spawn amount state
- Swarming flow:
  - separate test presentation start from colony-impact start
  - document commit timing and no-rollback policy
  - document that `SetColonyBeeCount()` and `SetHasQueenBee(false)` are the mutation paths
- Update the previous "test start does not modify colony state" wording so it applies only to the existing test API, not the new colony API.

Do not update unrelated systems.

## Validation commands

Run diff check:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

Run focused searches:

```powershell
rg -n "BeginSwarmingAtTransform|BeginSwarmingAtActor|BeginColonySwarming|ColonySwarmingBeeLossRatio|ActiveSwarmClusterSpawnAmount|ApplyColonySwarmingImpact|SwarmClusterSpawnAmount" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
rg -n "SetHasQueenBee\\(false\\)|SetColonyBeeCount\\(|ReduceAllCombTargetBeeCountsByConfiguredRatio|NotifySwarmingStartFailed|ReceiveSwarmingStarted" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that build could not be run.

## Manual PIE checks

Test API:

1. Call existing `BeginSwarmingAtTransform`.
2. Confirm route/cluster presentation still works.
3. Confirm hive queen remains.
4. Confirm `ColonyBeeCount` is unchanged.
5. Confirm active comb spawn/target state is not affected except by existing presentation-only behavior.

Colony API:

1. Set `ColonyBeeCount` to a known value and `bHasQueenBee=true`.
2. Set `ColonySwarmingBeeLossRatioMin` and `Max` to the same known value, e.g. `0.5`.
3. Call `BeginColonySwarmingAtTransform`.
4. Confirm route starts.
5. Confirm hive queen is removed after route start succeeds.
6. Confirm `ColonyBeeCount` decreases by the expected percentage.
7. Confirm route emission duration uses removed bee count divided by route spawn amount.
8. Confirm spawned cluster `SpawnAmount` equals removed bee count.
9. Confirm active comb spawn amounts update through `SetColonyBeeCount()` behavior.
10. Confirm calling colony API without a queen fails and does not mutate bee count.
11. Confirm calling colony API with zero colony bees fails and does not remove queen.
12. Confirm existing final swarm cluster capture behavior still works.

## Final report requirements

- Changed files
- New API names
- New settings and default values
- Exact real swarming bee-loss formula
- Confirmation that existing test APIs remain state-neutral
- Confirmation that real colony API removes queen and reduces `ColonyBeeCount`
- Confirmation that route emission duration and cluster spawn amount use removed bee count for colony mode
- Whether queen state transfer to cluster was implemented or deferred
- Blueprint/API/Core Redirect impact
- Architecture document updates
- Build/diff validation results
- Manual PIE checks still required
