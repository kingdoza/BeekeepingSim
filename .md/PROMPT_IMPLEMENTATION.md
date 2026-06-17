# Swarming pressure and queen cell implementation prompt

## Goal

Implement swarming pressure and queen cell gameplay on top of the already implemented site-based `ABeehive::BeginColonySwarming()` flow.

Confirmed direction:

- `AWorldOccupancySiteActor`, `ABeeSwarmClusterSiteActor`, and target-less `ABeehive::BeginColonySwarming()` already exist and are out of scope except for calling `BeginColonySwarming()` when pressure triggers.
- Add `ABeehive` swarming pressure driven primarily by colony population.
- Add queen cells as runtime state on active beehive comb actors.
- Queen cells spawn on comb edge surfaces when pressure is high.
- Removing queen cells reduces swarming pressure.
- If real colony swarming succeeds, reset swarming pressure to `0.0f`.
- Queen cells block comb retrieval, so queen cell state is not stored in `FBeehiveCombItemState`.

## Required reading

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## Relevant source files

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPlacementOccupantComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaMeshComponent.h`
- Item action classes under `Source/BeekeepingSim/Public/Inventory` and `Private/Inventory`
- New: `Source/BeekeepingSim/Public/WorldActors/QueenCellSpawnAreaComponent.h`
- New: `Source/BeekeepingSim/Private/WorldActors/QueenCellSpawnAreaComponent.cpp`
- New queen cell removal use action files in the existing item-use action system location.

Do not reimplement the swarm cluster site selection work.

## Beehive swarming pressure

Add swarming pressure state to `ABeehive`.

Recommended properties:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure")
float SwarmingPressure = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "1", ClampMax = "1440"))
int32 SwarmingLifecycleBucketMinutes = 30;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure")
bool bApplySwarmingLifecycleOnBeginPlayBucket = false;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "0.0"))
float ComfortBeeCountPerComb = 100.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "0.0"))
float PopulationStartRatio = 0.7f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "0.0001"))
float PopulationTriggerRatio = 1.1f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "0.0"))
float QueenCellSpawnPressureThreshold = 0.7f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "0.0001"))
float SwarmingTriggerPressure = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Pressure", meta = (ClampMin = "0.0"))
float QueenCellRemovalPressureDelta = 0.1f;
```

Recommended API:

```cpp
UFUNCTION(BlueprintPure, Category = "Beehive|Swarming Pressure")
float GetSwarmingPressure() const;

UFUNCTION(BlueprintCallable, Category = "Beehive|Swarming Pressure")
void SetSwarmingPressure(float NewPressure);

UFUNCTION(BlueprintCallable, Category = "Beehive|Swarming Pressure")
void ApplySwarmingLifecycleUpdate();

void HandleQueenCellRemoved(ABeehiveCombActor* SourceComb);
```

Pressure target calculation:

```cpp
ComfortBeeCapacity = ActiveCombCount * ComfortBeeCountPerComb;
PopulationRatio = ColonyBeeCount / ComfortBeeCapacity;
TargetPressure = (PopulationRatio - PopulationStartRatio) / (PopulationTriggerRatio - PopulationStartRatio);
```

Rules:

- Clamp/sanitize invalid denominator and capacity.
- If there is no queen, swarming lifecycle should not trigger swarming. Prefer moving pressure toward `0` or no-op; do not spawn queen cells.
- If no active combs exist, pressure target is `0`.
- `SwarmingPressure` can be clamped to `0..SwarmingTriggerPressure` before trigger, or allowed to exceed `1.0`; trigger check uses `SwarmingPressure > SwarmingTriggerPressure`.
- On successful `BeginColonySwarming()`, set `SwarmingPressure = 0.0f`.

## Bucket order

Add a new `SwarmingLifecycle` bucket subscription on `ABeehive`.

Default:

```cpp
SwarmingLifecycleBucketMinutes = 30;
```

Subscription ordering matters when buckets land on the same game-time boundary. Keep this order:

1. `HoneyProduction`
2. `ColonyPopulation`
3. `SwarmingLifecycle`
4. `PollenPattyConsumption`

In practice, add the `SwarmingLifecycle` subscription after `ColonyPopulation` and before `PollenPattyConsumption` in `GetGameTimeBucketSubscriptions_Implementation`.

`OnGameTimeBucketEvent_Implementation` should dispatch:

```cpp
if (Tag == "ColonyPopulation")
{
    ApplyColonyPopulationUpdate();
}
else if (Tag == "SwarmingLifecycle")
{
    ApplySwarmingLifecycleUpdate();
}
```

If the subsystem dispatches same-boundary subscriptions in returned order, this guarantees that population changes are visible to swarming pressure on the same boundary.

## Queen cell target count

Queen cell count is a hive-wide target derived from pressure.

Recommended properties on `ABeehive`:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0"))
int32 MaxQueenCellCountPerHive = 10;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0.0001"))
float QueenCellSpawnExponent = 1.5f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell", meta = (ClampMin = "0"))
int32 MaxQueenCellsSpawnPerBucket = 2;
```

Formula:

```cpp
Alpha = Clamp(
    (SwarmingPressure - QueenCellSpawnPressureThreshold)
    / (SwarmingTriggerPressure - QueenCellSpawnPressureThreshold),
    0.0f,
    1.0f);

DesiredQueenCellCount = RoundToInt(MaxQueenCellCountPerHive * Pow(Alpha, QueenCellSpawnExponent));
```

Rules:

- `DesiredQueenCellCount` is hive-wide, not per comb.
- `CurrentQueenCellCount` is the sum across active combs in the hive.
- `MissingCount = DesiredQueenCellCount - CurrentQueenCellCount`.
- `SpawnCountThisBucket = Min(MissingCount, MaxQueenCellsSpawnPerBucket)`.
- If pressure falls, do not auto-delete existing queen cells.
- Existing queen cells must be removed by player action.

## Queen cell comb distribution

Distribute `SpawnCountThisBucket` across eligible active combs.

Eligible combs:

- Active combs inside this hive.
- Exclude empty slots.
- Exclude currently lifted comb.
- Comb has a valid queen cell spawn area component.
- Comb has less than its per-comb max queen cell count.
- Comb can find a valid edge-band sample position.

Recommended comb weight:

```cpp
CombWeight = 1.0f / (1.0f + CurrentQueenCellCountOnComb);
```

Rules:

- This intentionally favors combs with fewer queen cells.
- Do not create `MissingCount` on every comb.
- For each queen cell to spawn this bucket, select one eligible comb by weighted random and ask that comb to add one queen cell.
- After spawning one queen cell, update that comb's count/weight before the next selection.

## Queen cell spawn area

Add `UQueenCellSpawnAreaComponent : public UBoxComponent`.

Attach one spawn area component to `ABeehiveCombActor`.

Purpose:

- Blueprint/editor visible rectangular box for queen cell spawn area authoring.
- One box covers both front and back surfaces.
- Sampling uses the box local `+X` surface for front and `-X` surface for back.

Axis contract:

- Local `X`: comb thickness/normal direction.
- Local `+X`: front surface.
- Local `-X`: back surface.
- Local `Y/Z`: rectangular surface coordinates.

Recommended properties:

```cpp
float EdgeBandWidthCm = 8.0f;
float EdgeInsetCm = 2.0f;
float MinQueenCellSpacingCm = 6.0f;
int32 MaxPlacementAttempts = 32;

float BottomEdgeWeight = 3.0f;
float LeftEdgeWeight = 1.0f;
float RightEdgeWeight = 1.0f;
float TopEdgeWeight = 0.5f;
```

Sampling rules:

- Choose front/back face.
- Choose one edge by edge weights.
- Sample a point on the local `Y/Z` edge band.
- Apply inset so queen cells do not sit outside the intended box.
- Reject points inside the center area.
- Reject points too close to existing queen cells on the same face using `MinQueenCellSpacingCm`.
- Store the result as `Face + AreaLocalYZ`, not world position.

Recommended placement state:

```cpp
USTRUCT(BlueprintType)
struct FQueenCellPlacement
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FGuid QueenCellId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EBeehiveCombFace Face;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D AreaLocalYZ = FVector2D::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LocalRotationDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Scale = 1.0f;
};
```

Use the existing comb face enum/name if one already exists. Do not introduce a duplicate enum.

## Queen cell runtime components

Queen cells are not actors. They are runtime component groups owned by `ABeehiveCombActor`.

Runtime structure:

```text
QueenCellRoot_N
  UStaticMeshComponent QueenCellVisual
    UItemUseAreaMeshComponent QueenCellUseArea
```

Add BP-authored visual/use-area settings on `ABeehiveCombActor`:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Visual")
TObjectPtr<UStaticMesh> QueenCellVisualMesh;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Visual")
TObjectPtr<UMaterialInterface> QueenCellVisualMaterial;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
TObjectPtr<UStaticMesh> QueenCellUseAreaMesh;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
TObjectPtr<UMaterialInterface> QueenCellUseAreaMaterial;
```

Rules:

- Mesh/material defaults are assigned in the comb Blueprint child, not hardcoded in C++.
- If `QueenCellUseAreaMesh` is missing, using `QueenCellVisualMesh` as fallback is acceptable.
- `QueenCellUseArea` should use area tag `Item.UseArea.Beehive.QueenCell`.
- Use a map to resolve hit use-area components to queen cell IDs:

```cpp
TMap<TObjectPtr<UItemUseAreaMeshComponent>, FGuid> QueenCellUseAreaToId;
```

- Do not store queen cell data in `FBeehiveCombItemState`.

## Queen cell comb APIs

Add APIs to `ABeehiveCombActor`.

Recommended:

```cpp
int32 GetQueenCellCount() const;
bool HasQueenCells() const;
bool CanSpawnQueenCell() const;
bool TrySpawnQueenCell();
bool RemoveQueenCell(const FGuid& QueenCellId);
bool ResolveQueenCellIdFromUseArea(const UItemUseAreaMeshComponent* UseArea, FGuid& OutQueenCellId) const;
```

Rules:

- `TrySpawnQueenCell()` samples an eligible front/back edge location and creates runtime components.
- `RemoveQueenCell()` destroys runtime components, removes placement state, updates use-area mapping, and notifies owning hive.
- Rebuild item-use-area descriptors after queen cell add/remove if the owning host/scope requires it.

## Queen cell removal action

Add a C++ item-use action for queen cell removal, for example:

```cpp
UQueenCellRemovalUseAction
```

Rules:

- The action is included in C++ scope.
- The actual existing item DataAsset that uses this action is assigned by BP/DataAsset work.
- Do not create/modify item assets unless explicitly requested.
- The action should require/hit `Item.UseArea.Beehive.QueenCell`.
- On hit, resolve the owning `ABeehiveCombActor` and queen cell ID.
- Call `RemoveQueenCell(QueenCellId)`.
- Return success only when a queen cell was actually removed.
- On successful removal, owning hive reduces `SwarmingPressure` by `QueenCellRemovalPressureDelta`.

## Comb retrieval condition

Queen cells block comb retrieval.

Existing comb retrieval condition:

```text
TotalTargetBeeCount == 0
queen not attached
```

New condition:

```text
TotalTargetBeeCount == 0
queen not attached
QueenCellCount == 0
```

Implementation target:

- Add the check in `UBeehiveCombPlacementOccupantComponent::ReceiveCanRetrievePlacementOccupant_Implementation` or the existing shared retrieval helper if one exists.
- Prompt disabled/failure reason should indicate that queen cells must be removed first if the prompt system supports a reason.

Because queen cells block retrieval, do not add queen cell state to `FBeehiveCombItemState`.

## Swarming lifecycle order

`ApplySwarmingLifecycleUpdate()` should run:

1. If queenless, do not spawn queen cells or start swarming.
2. Update `SwarmingPressure` from latest population/active comb state.
3. Spawn missing queen cells up to `MaxQueenCellsSpawnPerBucket`.
4. If `SwarmingPressure > SwarmingTriggerPressure`, call `BeginColonySwarming()`.
5. If `BeginColonySwarming()` returns true, set `SwarmingPressure = 0.0f` and do not spawn more queen cells in that update.

If implementation is simpler, step 3 may occur before step 4 in the same function as long as successful swarming immediately resets pressure and prevents additional same-update queen cell work.

## Gameplay tags

Add/use gameplay tag:

```text
Item.UseArea.Beehive.QueenCell
```

Follow the project's existing gameplay tag declaration/config pattern. Do not invent a separate tag system.

## Documentation updates

Update only relevant architecture docs:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

Document:

- `SwarmingPressure` source and bucket default `30` minutes.
- Same-boundary order: honey production, colony population, swarming lifecycle, pollen patty consumption.
- Queen cell count target is hive-wide and pressure-derived.
- Queen cells distribute to active combs by fewest-queen-cell weighted random.
- Single `UQueenCellSpawnAreaComponent : UBoxComponent` samples `+X/-X` surfaces and edge bands.
- Queen cells are runtime component groups, not actors.
- Queen cell visual/use-area mesh/material are assigned in comb Blueprint children.
- Queen cells block comb retrieval and are not saved in `FBeehiveCombItemState`.
- Successful colony swarming resets pressure to `0.0f`.

Do not rewrite already implemented swarm cluster site selection docs except where needed for cross-reference.

## Validation commands

Run diff check:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

Focused search:

```powershell
rg -n "SwarmingPressure|SwarmingLifecycle|QueenCell|QueenCellSpawnArea|Item.UseArea.Beehive.QueenCell|BeginColonySwarming|CanRetrievePlacementOccupant" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory .md
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that build could not be run.

## Manual PIE checks

1. In a comb Blueprint child, assign queen cell visual mesh/material and use-area mesh/material.
2. Confirm the queen cell spawn area box is visible/editable in Blueprint/editor.
3. Set lifecycle bucket to 30 minutes and confirm update dispatch.
4. Confirm population increase raises `SwarmingPressure`.
5. Confirm queen cells spawn only when pressure is above threshold.
6. Confirm queen cells appear only on `+X/-X` surfaces and only in the `Y/Z` edge band.
7. Confirm queen cells distribute across active combs with fewer queen cells favored.
8. Confirm lifted combs do not receive new queen cells.
9. Confirm queen cell use-area can be hit with the configured existing item DataAsset/action.
10. Confirm removing queen cells lowers pressure.
11. Confirm comb retrieval is disabled while queen cells exist.
12. Confirm comb retrieval works after all bees/queen/queen-cell constraints are clear.
13. Confirm pressure above trigger calls existing `BeginColonySwarming()`.
14. Confirm successful colony swarming resets `SwarmingPressure` to `0.0f`.

## Final report requirements

- Changed files
- New classes/components/actions
- New settings and default values
- Exact pressure formula
- Queen cell target-count formula
- Queen cell distribution rule
- Queen cell spawn-area axis contract
- Confirmation that queen cells are not saved in `FBeehiveCombItemState`
- Confirmation that queen cells block comb retrieval
- Blueprint/DataAsset work still required
- Architecture document updates
- Build/diff validation results
- Manual PIE checks still required
