# Beehive comb Blueprint editor delay implementation prompt

## Goal

Fix the delay that occurs when opening the beehive comb Blueprint or changing details values, while keeping gameplay behavior the same.

Confirmed cause:

- `ABeehiveCombActor` owns wax capping mask byte arrays and transient capping mask textures.
- The mask arrays are currently serialized into the comb Blueprint default object because they are `UPROPERTY(VisibleAnywhere)` without `Transient`.
- `OnConstruction()` and `PostEditChangeProperty()` refresh capping mask textures in editor-time paths.
- With `CappingMaskLongSideResolution = 512`, the two face masks are about `512 * 512 * 2 = 524,288` bytes before other asset overhead.
- `Content/Beehive/BP_HoneyComb.uasset` already contains `FrontWaxCappingMask` and `BackWaxCappingMask`, and is much larger than the lightweight parent Blueprint.

The fix should preserve runtime capping/uncapping behavior, item state preservation, honey visual behavior, and material parameter contracts.

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

Do not modify `Content/` assets unless the user explicitly asks for the manual cleanup/resave step.

## Behavioral constraints

Keep these behaviors unchanged:

- Runtime wax capping masks still exist per comb actor.
- `ApplyWaxCappingBrush(...)` still updates the correct face mask and texture.
- `TryRegenerateWaxCapping()` still restores capping masks when the honey/ripeness rules allow it.
- `ApplyStateFromItemInstance(...)` still restores capping masks from `FBeehiveCombItemState`.
- `WriteStateToItemInstance(...)` still writes capping masks to `FBeehiveCombItemState`.
- Capping material still receives texture parameter `WaxCappingMask`.
- Honey material parameters `HoneyAmount` and `HoneyRipeness` remain unchanged.
- No Blueprint API rename, UCLASS/USTRUCT/UENUM rename, or Core Redirect should be needed.

## Implementation tasks

### 1. Stop serializing heavy mask runtime state into Blueprint defaults

In `ABeehiveCombActor`, make the capping mask runtime fields transient.

Target properties:

```cpp
CappingMaskWidth
CappingMaskHeight
FrontWaxCappingMask
BackWaxCappingMask
```

Recommended declaration style:

```cpp
UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
int32 CappingMaskWidth = 0;

UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
int32 CappingMaskHeight = 0;

UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
TArray<uint8> FrontWaxCappingMask;

UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Wax Capping")
TArray<uint8> BackWaxCappingMask;
```

Notes:

- The arrays are runtime state, not Blueprint authoring data.
- `FBeehiveCombItemState` remains the state persistence path for inventory/item movement.
- Existing serialized values in `BP_HoneyComb` will only be removed after a later manual Blueprint compile/save or resave. Do not perform that content step in this implementation unless explicitly requested.

### 2. Avoid editor-time capping texture refresh

Do not rebuild capping mask textures in editor-only construction/details-change paths.

Update `ABeehiveCombActor::OnConstruction(...)`:

- Continue sanitizing scalar/editor-visible state.
- Continue applying Niagara user parameters and honey visual transform/material-safe editor behavior as needed.
- Only run `EnsureCappingMaskState()` and `RefreshCappingMaskTextures()` when the actor is in a game world.

Update `ABeehiveCombActor::PostEditChangeProperty(...)`:

- Continue sanitizing editor-visible state.
- Continue applying lightweight visual updates.
- Do not allocate/update transient capping textures in the editor details path.
- If the changed property can affect runtime mask dimensions, it is enough for the new dimensions to be applied on the next game-world initialization or item-state application.

Use the existing `BeehiveCombActorNames::IsGameWorldContext(...)` helper instead of inventing another world-type check.

### 3. Guard capping mask material parameter application

`ApplyWaxCappingMaskMaterialParameters()` currently calls `EnsureCappingMaskTextures()` before checking whether runtime dynamic material instances exist.

Change it so editor-time calls with no dynamic material instances return without allocating textures.

Recommended shape:

```cpp
void ABeehiveCombActor::ApplyWaxCappingMaskMaterialParameters()
{
    if (!FrontWaxCappingMaterialInstance && !BackWaxCappingMaterialInstance)
    {
        return;
    }

    EnsureCappingMaskTextures();

    ...
}
```

Rationale:

- In editor worlds, `EnsureHoneyMaterialInstances()` intentionally avoids creating dynamic material instances.
- Without this guard, editor-time visual refresh can still allocate capping textures indirectly.

### 4. Keep runtime texture refresh paths intact

Do not remove runtime texture refresh from these paths:

- `BeginPlay()`
- `ApplyCombBeeParameters(...)`
- `SetTotalSpawnAmountAndResetTargetBeeCounts(...)`
- `SetTotalSpawnAmountPreservingTargetRatios(...)`
- `ApplyWaxCappingBrush(...)`
- `TryRegenerateWaxCapping()`
- `ApplyStateFromItemInstance(...)`

If you factor helper functions, keep the same runtime call order and outputs.

## Documentation updates

Update only the relevant architecture docs:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

Document the clarified contract:

- `ABeehiveCombActor` capping masks are transient runtime actor state.
- Inventory/item persistence remains `FBeehiveCombItemState`.
- Blueprint class defaults should not serialize the large face mask arrays.
- Editor construction/details changes should avoid transient capping texture allocation/update.
- Runtime still creates/updates transient `UTexture2D` masks and applies `WaxCappingMask` to capping materials.

Do not rewrite unrelated swarming, queen cell, honey container, or focus documentation.

## Validation commands

Run diff check:

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md
```

Focused search:

```powershell
rg -n "CappingMaskWidth|CappingMaskHeight|FrontWaxCappingMask|BackWaxCappingMask|RefreshCappingMaskTextures|ApplyWaxCappingMaskMaterialParameters|PostEditChangeProperty|OnConstruction" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that build could not be run.

## Manual editor checks

These require the Unreal Editor and may be performed by the user if the implementation agent cannot safely open/resave assets.

1. Open `Content/Beehive/BP_HoneyComb`.
2. Confirm opening the Blueprint no longer has the previous delay.
3. Change ordinary details values on the comb Blueprint and confirm the previous delay is gone or significantly reduced.
4. Compile/save the Blueprint once to remove old serialized transient mask data from the asset.
5. Confirm `BP_HoneyComb.uasset` size drops after compile/save.

## Manual PIE checks

1. Spawn or place a beehive comb through the existing beehive/uncapping table flow.
2. Confirm honey fill/ripeness visuals still update.
3. Confirm full honey capping visuals still appear.
4. Use the uncapping action and confirm brush strokes still remove capping.
5. Retrieve the comb into inventory, place it again, and confirm capping mask state is restored from item state.
6. Confirm wax capping regeneration still restores capping when the existing honey/ripeness rules allow it.

## Final report requirements

- Changed files
- Exact capping mask property changes
- Exact editor-time guard changes
- Confirmation that runtime capping behavior is unchanged
- Confirmation that `FBeehiveCombItemState` remains the persistence path
- Architecture document updates
- Diff/build validation results
- Manual Content cleanup/resave still required, if not performed
