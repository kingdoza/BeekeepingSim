# Colony swarming site selection implementation prompt

## Goal

Implement real colony swarming so the hive chooses a world occupancy site itself.

Confirmed direction:

- Keep `ABeehive::BeginSwarmingAtTransform(...)` and `BeginSwarmingAtActor(...)` as state-neutral test/presentation APIs.
- Replace the real colony swarming target-input API with `ABeehive::BeginColonySwarming()` with no target parameter.
- Real colony swarming must select one available swarm cluster site from the world, with closer sites more likely to be chosen.
- Add a reusable world occupancy site base so future non-swarm actors can use the same reservation/occupation model.

## Required reading

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## Relevant source files

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- New: `Source/BeekeepingSim/Public/WorldActors/WorldOccupancySiteActor.h`
- New: `Source/BeekeepingSim/Private/WorldActors/WorldOccupancySiteActor.cpp`
- New: `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterSiteActor.h`
- New: `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterSiteActor.cpp`

## New generic site actor

Add `AWorldOccupancySiteActor`.

Responsibilities:

- Represents a reusable world actor occupancy site.
- Owns generic reservation and occupation state.
- Provides an occupant spawn transform.
- Optionally releases occupation when the occupant actor is destroyed.

Recommended state:

```cpp
UENUM(BlueprintType)
enum class EWorldOccupancySiteState : uint8
{
    Available,
    Reserved,
    Occupied
};
```

Recommended components:

```cpp
USceneComponent* Root;
USceneComponent* OccupantSpawnPoint;
```

Recommended properties:

```cpp
bool bEnabled = true;
bool bAutoReleaseWhenOccupantDestroyed = true;
FGameplayTagContainer SiteTags;
TSubclassOf<AActor> AcceptedOccupantClass;

UPROPERTY(Transient)
TObjectPtr<AActor> ReservedByActor;

UPROPERTY(Transient)
TObjectPtr<AActor> OccupyingActor;
```

Recommended API:

```cpp
EWorldOccupancySiteState GetSiteState() const;
bool IsAvailable() const;
bool CanAcceptOccupant(AActor* Candidate) const;
bool TryReserve(AActor* RequestedBy);
bool ReleaseReservation(AActor* RequestedBy);
bool TryOccupy(AActor* RequestedBy, AActor* Occupant);
bool ClearOccupant(AActor* Occupant);
FTransform GetOccupantSpawnTransform() const;
AActor* GetReservedByActor() const;
AActor* GetOccupyingActor() const;
```

Rules:

- `Available`: enabled, not reserved, no valid occupant.
- `Reserved`: no occupant, reserved by a valid requester.
- `Occupied`: has a valid occupying actor.
- `TryReserve` succeeds only when the site is available.
- `TryOccupy` succeeds when available or reserved by the same requester.
- `TryOccupy` must call `CanAcceptOccupant`.
- If `bAutoReleaseWhenOccupantDestroyed` is true, bind to the occupant's destroy event and clear the site when the occupant is destroyed.
- Do not depend on inventory placement APIs. This is separate from `AItemPlacementSlotActor`.

## New swarm-specific site actor

Add `ABeeSwarmClusterSiteActor : public AWorldOccupancySiteActor`.

Responsibilities:

- Represents a valid real colony swarming destination.
- Accepts `ABeeSwarmClusterActor` occupants by default.
- Computes hive-distance weighted selection weight.

Recommended settings:

```cpp
float SelectionWeightMultiplier = 1.0f;
float DistanceWeightScaleCm = 3000.0f;
float DistanceWeightExponent = 2.0f;
bool bUse2DDistanceForSelection = true;
float MaxSelectionDistanceCm = 0.0f; // 0 means unlimited
```

Recommended API:

```cpp
float CalculateSelectionWeightForHive(const ABeehive* Hive) const;
```

Weight formula:

```cpp
Weight = SelectionWeightMultiplier / Pow(1.0f + Distance / DistanceWeightScaleCm, DistanceWeightExponent);
```

Rules:

- Use the hive `SwarmExitPoint` location when available; otherwise use hive actor location.
- If `MaxSelectionDistanceCm > 0` and distance is greater than it, weight is `0`.
- Clamp invalid `DistanceWeightScaleCm` and `DistanceWeightExponent` to safe positive values.
- Weight <= 0 candidates are not selectable.

## Beehive API change

Keep test APIs:

```cpp
bool BeginSwarmingAtTransform(const FTransform& TargetTransform);
bool BeginSwarmingAtActor(AActor* TargetActor);
```

Change real colony API to:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Colony Swarming")
bool BeginColonySwarming();
```

Remove the no-longer-needed real colony target-input APIs:

```cpp
BeginColonySwarmingAtTransform(...)
BeginColonySwarmingAtActor(...)
```

This deletion is intentional per current design. Before deleting, search for references and report Blueprint migration impact. Do not modify or resave `Content/` assets in this implementation pass.

## Beehive selection flow

`ABeehive::BeginColonySwarming()` should:

1. Validate `bHasQueenBee`.
2. Validate `ColonyBeeCount > 0`.
3. Compute outgoing bee count using existing min/max loss ratio settings.
4. Find all world `ABeeSwarmClusterSiteActor` instances.
5. Keep only available sites with positive selection weight.
6. Select one site by weighted random.
7. Reserve the selected site for this hive.
8. Start the existing route flow using `SelectedSite->GetOccupantSpawnTransform()`.
9. If route start fails before colony impact commit, release the reservation and do not mutate hive state.
10. If route start succeeds, apply the existing colony impact: reduce `ColonyBeeCount` and remove the queen.

Weighted random rules:

- Sum candidate weights.
- Pick a random threshold in `[0, TotalWeight]`.
- Walk candidates until cumulative weight reaches the threshold.
- If total weight is not positive, fail cleanly.

## Route/session integration

Add transient session references on `ABeehive`:

```cpp
UPROPERTY(Transient)
TObjectPtr<ABeeSwarmClusterSiteActor> PendingSwarmClusterSite;

UPROPERTY(Transient)
TObjectPtr<ABeeSwarmClusterSiteActor> ActiveSwarmClusterSite;
```

Rules:

- Test presentation APIs do not use a site.
- Real colony swarming stores the reserved site as `PendingSwarmClusterSite` until route arrival.
- Route arrival spawns `ABeeSwarmClusterActor` at the pending site transform.
- On cluster spawn success, call `PendingSwarmClusterSite->TryOccupy(this, ClusterActor)` and move it to `ActiveSwarmClusterSite`.
- If cluster spawn or site occupation fails after colony impact commit, release the site, notify failure, and keep the existing no-rollback policy for queen/bee count.
- The site auto-release-on-destroy path should clear occupation when the cluster actor is destroyed after final capture.
- `ClearActiveTestSwarm(true)` should not leak site reservations or occupations. Release pending reservation and clear active occupation when the cluster is explicitly destroyed.

## Existing colony impact behavior

Keep the existing real colony swarming impact rules:

- Requires queen.
- Requires positive `ColonyBeeCount`.
- Computes outgoing bees from `ColonySwarmingBeeLossRatioMin` and `ColonySwarmingBeeLossRatioMax`.
- Route emission duration and arrival cluster `SpawnAmount` use the outgoing bee count.
- Commit impact only after route actor spawn/config/timing succeeds.
- Use `SetColonyBeeCount(...)` for bee count mutation.
- Use `SetHasQueenBee(false)` for queen removal.
- Do not call `ReduceAllCombTargetBeeCountsByConfiguredRatio()` from this path.
- Do not rollback queen/bee count after impact commit.

## Existing test API invariants

For `BeginSwarmingAtTransform(...)` and `BeginSwarmingAtActor(...)`:

- Do not check `bHasQueenBee`.
- Do not check `ColonyBeeCount`.
- Do not reserve or occupy a swarm cluster site.
- Do not call `SetHasQueenBee(false)`.
- Do not call `SetColonyBeeCount(...)`.
- Continue using authored `SwarmClusterSpawnAmount`.

## Blueprint/API/Core Redirect impact

Expected additions:

- `AWorldOccupancySiteActor`
- `ABeeSwarmClusterSiteActor`
- `EWorldOccupancySiteState`
- `ABeehive::BeginColonySwarming()`

Expected removals:

- `ABeehive::BeginColonySwarmingAtTransform(...)`
- `ABeehive::BeginColonySwarmingAtActor(...)`

Core Redirect:

- No UCLASS/USTRUCT/UENUM rename is planned.
- Do not edit `Config/DefaultEngine.ini`.

Blueprint migration:

- Existing Blueprint nodes using `BeginColonySwarmingAtTransform/Actor` must be replaced with `BeginColonySwarming`.
- Search and report references before deletion.
- Do not modify `Content/` unless the user explicitly asks for asset migration.

## Documentation updates

Update `.md/0_ARCHITECTURE.md` and `.md/Architecture/WorldActorsSystem.md`:

- Add `AWorldOccupancySiteActor`.
- Add `ABeeSwarmClusterSiteActor`.
- Document site states: available, reserved, occupied.
- Document that real colony swarming uses `BeginColonySwarming()` with internal weighted random site selection.
- Document that `BeginSwarmingAtTransform/Actor` remain test/presentation APIs.
- Document Blueprint migration impact from removed real colony target-input APIs.

Do not update unrelated systems.

## Validation commands

Run diff check:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

Run focused source searches:

```powershell
rg -n "BeginSwarmingAtTransform|BeginSwarmingAtActor|BeginColonySwarming|WorldOccupancySite|BeeSwarmClusterSite|PendingSwarmClusterSite|ActiveSwarmClusterSite" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

Search likely Blueprint/API references:

```powershell
rg -a -n "BeginColonySwarmingAtTransform|BeginColonySwarmingAtActor" Source Content Config .md
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that build could not be run.

## Manual PIE checks

Test API:

1. Call `BeginSwarmingAtTransform`.
2. Confirm route/cluster presentation still works.
3. Confirm hive queen remains.
4. Confirm `ColonyBeeCount` is unchanged.
5. Confirm no swarm cluster site is reserved or occupied.

Colony API:

1. Place multiple `ABeeSwarmClusterSiteActor` instances.
2. Call `BeginColonySwarming`.
3. Confirm an available site is selected and reserved.
4. Confirm closer sites are selected more often over repeated trials.
5. Confirm route ends at the selected site transform.
6. Confirm cluster spawn changes the site to occupied.
7. Confirm final cluster capture/destroy releases the site.
8. Confirm no available site causes clean failure with no queen/bee count mutation.
9. Confirm route-start failure releases the reserved site with no queen/bee count mutation.
10. Confirm post-commit cluster spawn failure releases the site but does not rollback queen/bee count.

## Final report requirements

- Changed files
- New classes and API names
- Removed API names and Blueprint reference search result
- Site state model summary
- Weighted random formula and defaults
- Confirmation that test APIs remain state-neutral
- Confirmation that real colony API chooses a site internally
- Architecture document updates
- Build/diff validation results
- Manual PIE checks still required
