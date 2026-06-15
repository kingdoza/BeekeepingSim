# Swarm cluster native FocusCollision implementation prompt

## Goal

Move the swarm cluster FocusEngaged hit proxy from Blueprint-authored `SphereCollision` to a native C++ component owned by `ABeeSwarmClusterActor`.

Decision confirmed by user:

- Remove or disable the existing Blueprint `SphereCollision`.
- Use only the new C++ `FocusCollision` for swarm cluster preview focus hit testing.

The native focus collision must:

1. Exist on every `ABeeSwarmClusterActor` instance.
2. Block the focus trace while the actor is available for preview focus.
3. Be disabled while the swarm cluster is FocusEngaged.
4. Track cluster visual radius as `AliveRadius + 5.0f`.

## Required reading

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## Relevant source files

- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`

## Current code premises

- `UBeekeeperFocusComponent::FindFocusTargetFromTrace()` line-traces on `FocusTraceChannel`, currently `ECC_Visibility`, then returns `HitActor->FindComponentByClass<UFocusTargetComponent>()`.
- `ABeeSwarmClusterActor` has `UFocusTargetComponent` and `USwarmClusterFocusActionComponent`.
- `USwarmClusterFocusActionComponent` derives from `UAnchoredFocusCursorActionComponent`.
- `UAnchoredFocusCursorActionComponent::OnFocusEngagedStarted(...)` activates item-use-area and part-focus scopes.
- FocusEngaged item-use-area cursor traces also use `ECC_Visibility`.
- Therefore, the swarm cluster focus hit proxy must not keep blocking visibility while FocusEngaged is active, otherwise it can prevent BeeCarrier/QueenCage use-area hits.
- `ABeeSwarmClusterActor::AliveRadius` changes through:
  - density initialization
  - intro growth
  - `SetAliveRadius`
  - BeeCarrier capture amount updates
  - bees captured / final captured state
- `ABeeSwarmClusterActor::ApplyClusterNiagaraParameters()` is the central current point for pushing `AliveRadius` to Niagara.

## Implementation requirements

### 1. Add native component

Add a native sphere component to `ABeeSwarmClusterActor`.

Header:

```cpp
class USphereComponent;
```

Protected component property:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<USphereComponent> FocusCollision;
```

Constructor:

```cpp
FocusCollision = CreateDefaultSubobject<USphereComponent>(TEXT("FocusCollision"));
FocusCollision->SetupAttachment(ClusterCenter);
```

Collision defaults:

- `QueryOnly`
- all channels ignore
- `ECC_Visibility` block
- overlap events disabled
- navigation disabled if available for the component
- no physics
- no visual rendering dependency

Use local helper functions rather than scattering collision setup through the class.

Suggested private helpers:

```cpp
void RefreshFocusCollisionState();
void SetFocusCollisionEnabled(bool bEnabled);
float GetFocusCollisionRadius() const;
```

Suggested private state:

```cpp
UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Focus", meta = (AllowPrivateAccess = "true"))
bool bFocusCollisionSuppressedForFocusEngaged = false;
```

If you do not need a stored bool because action state can be queried reliably, keep the implementation simpler. The key invariant is that radius updates must not accidentally re-enable collision during FocusEngaged.

### 2. Radius synchronization

The focus sphere radius must always follow:

```cpp
FocusCollisionRadius = FMath::Max(0.0f, AliveRadius) + 5.0f;
```

Add an editable padding only if needed:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Swarm Cluster|Focus", meta = (ClampMin = "0.0"))
float FocusCollisionRadiusPadding = 5.0f;
```

Otherwise hard-code `5.0f` to match the user request.

Call `RefreshFocusCollisionState()` from every path that can change `AliveRadius` or captured/focus state. At minimum:

- constructor or post component setup for initial radius
- `OnConstruction`
- `BeginPlay`
- `ApplyClusterNiagaraParameters`
- `SetAliveRadius`
- `ApplyIntroAliveRadiusVisual`
- `FinishAliveRadiusIntroGrowth`
- `HandleBeesCapturedIfNeeded`
- `HandleSwarmCapturedIfNeeded`

Preferred consolidation:

- Put radius sync at the end of `ApplyClusterNiagaraParameters()`.
- Ensure `ApplyClusterNiagaraParameters()` no longer returns before focus collision refresh. If `ClusterNiagara` is null, focus collision still needs to update.

Example shape:

```cpp
void ABeeSwarmClusterActor::ApplyClusterNiagaraParameters()
{
    if (ClusterNiagara)
    {
        // existing Niagara parameter writes
    }

    RefreshFocusCollisionState();
}
```

### 3. Collision enabled rules

`FocusCollision` should be enabled only when all are true:

- component exists
- actor is not final captured
- actor is not pending kill/destroy
- focus collision is not suppressed for active FocusEngaged

`FocusCollision` should be disabled when any are true:

- FocusEngaged starts on the swarm cluster
- focus action aborts
- final captured state is reached
- actor is being destroyed

During intro growth:

- Keep `FocusCollision` enabled.
- `USwarmClusterFocusActionComponent::CanBeginFocusAction(...)` already returns false while intro growth is active.
- This allows hover/prompt to exist but FocusEngaged entry to remain unavailable until intro completes.

When bees are fully captured but queen remains:

- `AliveRadius` becomes `0.0f`, so `FocusCollision` becomes radius `5.0f`.
- This follows the strict user request `AliveRadius + 5`.
- Do not add a minimum focus radius unless the user explicitly asks later.

### 4. FocusEngaged lifecycle integration

Use the swarm-cluster-specific action component.

In `USwarmClusterFocusActionComponent`, override:

```cpp
virtual void OnFocusEngagedStarted(ABeekeeperCharacter* InteractingCharacter) override;
virtual void OnFocusReturnCompleted(ABeekeeperCharacter* InteractingCharacter) override;
virtual void OnFocusActionAborted(ABeekeeperCharacter* InteractingCharacter) override;
```

Implementation behavior:

- Call `Super` first unless there is a concrete reason not to.
- On engaged start:
  - mark focus collision suppressed
  - disable `FocusCollision`
- On return completed:
  - clear suppression
  - refresh focus collision state
- On abort:
  - clear suppression
  - refresh focus collision state

Add public or private `ABeeSwarmClusterActor` methods as needed, for example:

```cpp
void SetFocusCollisionSuppressedForFocusEngaged(bool bSuppressed);
```

Keep this API non-Blueprint unless Blueprint needs it. Do not expose new Blueprint surface unnecessarily.

### 5. Blueprint migration requirement

Existing swarm cluster Blueprint must remove or disable its authored `SphereCollision`.

Manual asset action:

- Open the swarm cluster Blueprint currently using `ABeeSwarmClusterActor`.
- Remove the Blueprint `SphereCollision`, or set it to `NoCollision`.
- Compile and save the Blueprint.

Important:

- If the BP `SphereCollision` remains with `Visibility` blocking, it can still intercept FocusEngaged internal item-use-area cursor traces even after native `FocusCollision` is disabled.
- The implementation should not try to discover and mutate arbitrary Blueprint sphere components by name. The contract is native `FocusCollision` only, and BP duplicate collision must be removed/disabled manually.

### 6. Blueprint/API/Core Redirect impact

Expected C++ API impact:

- Add `USphereComponent* FocusCollision` native component property to `ABeeSwarmClusterActor`.
- No class rename.
- No USTRUCT/UENUM rename.
- No function/property deletion.

Core Redirect:

- Not required.
- Do not edit `Config/DefaultEngine.ini`.

Blueprint impact:

- Existing Blueprint may gain a new inherited `FocusCollision` component.
- Existing BP-authored `SphereCollision` must be removed or disabled manually.
- Blueprint compile/save is required after migration.

## Documentation updates

Update `.md/0_ARCHITECTURE.md`:

- Mention that `ABeeSwarmClusterActor` owns native `FocusCollision` for preview focus hit testing.
- Mention that the focus collision radius follows `AliveRadius + 5`.
- Mention that FocusEngaged disables the focus collision so internal BeeCarrier/QueenCage item-use-area traces are not blocked.

Update `.md/Architecture/WorldActorsSystem.md`:

- Add `USphereComponent FocusCollision` to `ABeeSwarmClusterActor` composition.
- Document collision policy:
  - preview focus hit proxy
  - visibility block while not FocusEngaged/final captured
  - disabled during FocusEngaged
  - radius sync from `AliveRadius + 5`
- Add migration note: BP `SphereCollision` must be removed or set to `NoCollision`.

Update `.md/Architecture/FocusSystem.md` only if needed:

- If documenting cross-system trace behavior, add a short note that FocusEngaged hosts may disable their preview focus hit proxy during engaged mode so cursor item-use traces can hit internal use-area components.

Do not update unrelated systems.

## Validation commands

Run diff check:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

Run focused searches:

```powershell
rg -n "FocusCollision|SphereCollision|SetFocusCollision|RefreshFocusCollision|AliveRadius \\+ 5|OnFocusEngagedStarted|OnFocusReturnCompleted|OnFocusActionAborted" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
rg -n "FindFocusTargetFromTrace|FocusTraceChannel|CursorTraceChannel|ECC_Visibility|ItemUseArea" Source/BeekeepingSim/Private/Focus Source/BeekeepingSim/Public/Focus
```

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

If the engine path is missing, do not guess another engine version. Report that build could not be run.

## Manual Editor/PIE checks

1. Open the swarm cluster Blueprint.
2. Remove existing BP `SphereCollision`, or set it to `NoCollision`.
3. Compile and save the Blueprint.
4. Start a manual swarming route and wait for cluster spawn.
5. Confirm the inherited native `FocusCollision` exists.
6. Confirm preview focus can hit the swarm cluster through `FocusCollision`.
7. During intro growth, confirm prompt can show but FocusEngaged cannot begin.
8. After intro growth, confirm FocusEngaged can begin.
9. On FocusEngaged start, confirm `FocusCollision` collision becomes disabled.
10. In FocusEngaged, select BeeCarrier and confirm BeeCarrier use-area can be hit/used.
11. In FocusEngaged, select QueenCage and confirm queen use-area can be hit/used.
12. Cancel/exit FocusEngaged before final capture and confirm `FocusCollision` is restored.
13. Capture bees and confirm `FocusCollision` radius follows `AliveRadius + 5`.
14. With bees fully captured and queen still present, confirm radius is `5`.
15. Capture queen and confirm final captured actor removal still works without the old BP collision.

## Final report requirements

- Changed files
- Whether `FocusCollision` was added as native `USphereComponent`
- Exact collision profile/responses used
- Exact radius formula used
- Where radius sync is called
- Where FocusEngaged disables/restores the collision
- Confirmation that no Blueprint API was deleted or renamed
- Confirmation that no Core Redirect was needed
- Architecture document updates
- Build/diff validation results
- Manual BP migration and PIE checks still required
