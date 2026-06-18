# Queen cell spawn relative transform implementation prompt

## Goal

Add an authoring-time relative transform offset for queen cell spawn visuals/use-areas on `ABeehiveCombActor`.

The feature lets the comb Blueprint adjust spawned queen cell placement without changing the existing queen cell sampling, lifecycle, removal, retrieval blocking, or runtime persistence contracts.

## Confirmed design

- Queen cells remain runtime component groups owned by `ABeehiveCombActor`; they are not actors.
- `UQueenCellSpawnAreaComponent` still samples the base placement:
  - face: `EBeehiveCombVisibleFace`
  - surface coordinate: spawn-area local `Y/Z`
  - random local rotation
  - placement scale
- Add one comb authoring property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Spawn")
FTransform QueenCellSpawnRelativeTransform = FTransform::Identity;
```

- The property is an authoring-time default. Runtime changes after a queen cell has spawned are out of scope.
- Do not add a refresh helper for already-spawned queen cells.
- Apply the transform to `QueenCellRoot`, not only to `QueenCellVisual`.
- `QueenCellVisual` stays identity relative to `QueenCellRoot`.
- `QueenCellUseArea` keeps identity location/rotation but receives an extra relative scale multiplier.
- Front/back use the same transform in the sampled face-local frame.
- Default `Identity` and `FVector::OneVector` values must preserve current behavior exactly.

## Required reading

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

## Relevant files

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

Do not modify `Content/` assets for this task.
Do not modify `Config/DefaultEngine.ini`.

## Behavioral constraints

Keep these behaviors unchanged:

- `FQueenCellPlacement` fields and meaning stay unchanged:
  - `QueenCellId`
  - `Face`
  - `AreaLocalYZ`
  - `LocalRotationDegrees`
  - `Scale`
- Queen cell placement is still runtime-only and is not written into `FBeehiveCombItemState`.
- `ApplyStateFromItemInstance()` still clears runtime queen cells so item state and queen cell runtime state do not mix.
- `TrySpawnQueenCell()` still uses `UQueenCellSpawnAreaComponent::TrySampleQueenCellPlacement(...)`.
- `CanSpawnQueenCell()` still depends on max count, mesh availability, spawn area availability, and sample availability.
- `RemoveQueenCell(...)` still resolves/removes by cell id and notifies the owning hive.
- `ResolveQueenCellIdFromUseArea(...)` and the `QueenCellUseAreaToId` mapping remain valid.
- Queen cell use-area tag remains `Item.UseArea.Beehive.QueenCell`.
- Comb retrieval remains blocked while `QueenCellCount > 0`.
- No UCLASS/USTRUCT/UENUM rename.
- No existing Blueprint API delete/rename.
- No Core Redirect.

## Implementation tasks

### 1. Add authoring property

In `ABeehiveCombActor`, add:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Spawn")
FTransform QueenCellSpawnRelativeTransform = FTransform::Identity;
```

Place it near the existing queen cell visual/use-area authoring properties.

Add the use-area-only scale multiplier near the existing queen cell use-area authoring properties:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
FVector QueenCellUseAreaScaleMultiplier = FVector::OneVector;
```

Do not add it to `FQueenCellPlacement`.
Do not add item-state persistence for it.

### 2. Compose it into the queen cell root transform

Update `ABeehiveCombActor::BuildQueenCellSpawnAreaRelativeTransform(const FQueenCellPlacement& Placement) const`.

Current base behavior should remain the source of the sampled surface transform:

```cpp
const FVector Extent = QueenCellSpawnArea ? QueenCellSpawnArea->GetUnscaledBoxExtent() : FVector::ZeroVector;
const float SurfaceX = Placement.Face == EBeehiveCombVisibleFace::Front ? Extent.X : -Extent.X;
const FVector RelativeLocation(SurfaceX, Placement.AreaLocalYZ.X, Placement.AreaLocalYZ.Y);
const float FaceYaw = Placement.Face == EBeehiveCombVisibleFace::Front ? 0.0f : 180.0f;
const FRotator RelativeRotation(0.0f, FaceYaw, Placement.LocalRotationDegrees);
const FVector RelativeScale(FMath::Max(0.01f, Placement.Scale));
const FTransform BaseTransform(RelativeRotation, RelativeLocation, RelativeScale);
```

Then compose the authoring offset in the sampled face-local frame:

```cpp
return SanitizedQueenCellSpawnRelativeTransform * BaseTransform;
```

Use the same ordering intentionally. This makes the offset behave relative to the sampled surface orientation, so front/back share one authored transform while still following their face frame.

### 3. Sanitize the authoring transform scale

Avoid zero or invalid scale on runtime components.

Either add a small private helper or sanitize inline in `BuildQueenCellSpawnAreaRelativeTransform(...)`.

Recommended minimum:

- Non-finite scale component -> `1.0f`
- Scale component below `0.01f` -> `0.01f`
- Preserve finite location and rotation as authored, unless the existing codebase already has a local transform sanitize pattern to follow.

Do not silently change the default identity behavior.

### 4. Apply use-area-only scale multiplier

Do not move the root offset to child components.

The visual child should remain identity:

```cpp
Visual->SetupAttachment(CellRoot);
Visual->SetRelativeTransform(FTransform::Identity);
```

The use-area child should keep identity location/rotation and apply only `QueenCellUseAreaScaleMultiplier` as relative scale:

```cpp
UseArea->SetupAttachment(Visual);
UseArea->SetRelativeTransform(FTransform(
    FRotator::ZeroRotator,
    FVector::ZeroVector,
    SanitizedQueenCellUseAreaScaleMultiplier));
```

This makes the final use-area scale equal to sampled placement scale * `QueenCellSpawnRelativeTransform` scale * `QueenCellUseAreaScaleMultiplier`.

Sanitize `QueenCellUseAreaScaleMultiplier` with the same minimum scale policy used for `QueenCellSpawnRelativeTransform` scale:

- Non-finite scale component -> `1.0f`
- Scale component below `0.01f` -> `0.01f`

## Documentation updates

Update only the relevant architecture docs:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

Document the clarified contract:

- `ABeehiveCombActor::QueenCellSpawnRelativeTransform` is an authoring-time offset applied to the spawned `QueenCellRoot`.
- `ABeehiveCombActor::QueenCellUseAreaScaleMultiplier` is an authoring-time use-area-only scale multiplier.
- The sampled base placement still comes from `UQueenCellSpawnAreaComponent` as face + area-local YZ + local rotation + scale.
- `FQueenCellPlacement` does not store the authoring offset.
- Visual remains identity under the root.
- Use-area keeps identity location/rotation and applies only the extra scale multiplier, so visual and hit/use area can be sized independently while staying centered/aligned.
- Runtime changes to the transform after spawn are out of scope.
- Identity/one-vector defaults preserve existing placement behavior.

Do not rewrite unrelated swarming, honey, inventory, focus, or capping mask documentation.

## Validation commands

Run diff check:

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md
```

Focused search:

```powershell
rg -n "QueenCellSpawnRelativeTransform|QueenCellUseAreaScaleMultiplier|BuildQueenCellSpawnAreaRelativeTransform|FQueenCellPlacement|QueenCellRoot|QueenCellVisual|QueenCellUseArea" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that the build could not be run.

## Manual editor checks

These require Unreal Editor access:

1. Open the comb Blueprint that derives from `ABeehiveCombActor`.
2. Confirm `QueenCellSpawnRelativeTransform` appears under `Beehive|Queen Cell|Spawn`.
3. Confirm `QueenCellUseAreaScaleMultiplier` appears under `Beehive|Queen Cell|Use Area`.
4. Set a visible location/rotation/scale offset.
5. Set a use-area scale multiplier different from one.
6. Trigger queen cell spawning through the existing swarming pressure/test path.
7. Confirm spawned queen cell visual follows the root offset.
8. Confirm removal use-area remains centered/aligned but scales by the additional multiplier.
9. Confirm front and back cells use the same authored offset relative to their face orientation.
10. Confirm leaving the transform as Identity and multiplier as OneVector matches the old placement.

## Manual PIE checks

1. Let a hive spawn queen cells through the existing lifecycle.
2. Confirm queen cell removal still works through `Item.UseArea.Beehive.QueenCell`.
3. Confirm removing a queen cell still lowers hive pressure through the existing path.
4. Confirm comb retrieval is still blocked while queen cells exist.
5. Confirm item state restore still clears runtime queen cells when applying `FBeehiveCombItemState`.

## Final report requirements

- Changed files
- Added property name, category, and default value
- Exact transform composition order
- Exact use-area scale multiplier application
- Confirmation that `FQueenCellPlacement` was not changed
- Confirmation that visual remains identity and use-area applies only the additional scale
- Confirmation that runtime transform changes after spawn are intentionally not handled
- Architecture document updates
- Diff/build validation results
- Any manual checks that could not be run
