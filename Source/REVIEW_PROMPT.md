Review the BeekeepingSim hotbar pickup implementation with focus on correctness, Unreal object lifecycle safety, and focus/hotbar integration.

Changed files:
- `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Public/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/FocusTargetComponent.cpp`
- `Source/BeekeepingSim/Public/WorldItemPickup.h`
- `Source/BeekeepingSim/Private/WorldItemPickup.cpp`
- `Source/BeekeepingSim/Public/PickupFocusActionComponent.h`
- `Source/BeekeepingSim/Private/PickupFocusActionComponent.cpp`
- `Source/ARCHITECTURE.md`

Change goal:
- Keep hotbar as the direct runtime item storage and add stack-aware item acquisition without introducing a separate inventory component.
- Add a world pickup actor and a pickup-specific focus action that reuses the existing focus system for item acquisition.

Core logic to inspect:
- `UBeekeeperHotbarComponent::TryAcquireItem()` merges into existing stacks by `UItemDefinition*`, creates new `UItemInstance` objects for empty slots, and returns a structured acquisition result.
- New `UItemInstance` objects are created with the hotbar component as outer and initialized through `InitializeFromDefinition()`.
- `UFocusTargetComponent` now exposes minimal runtime setters for prompt metadata used by pickup actors.
- `AWorldItemPickup` reflects `UItemDefinition` data into world mesh and focus prompt display name.
- `UPickupFocusActionComponent` performs a single confirm-time acquisition attempt, destroys the pickup on success, and leaves it in place on failure.

Review focus:
- Correctness of stack merge order, empty-slot placement, and remaining-quantity reporting.
- GC safety and ownership of newly created `UItemInstance` objects.
- Safety when pickup definition, hotbar component, or world mesh is null.
- Whether the pickup action correctly cooperates with `UBeekeeperFocusComponent`'s engaged/abort flow.
- Whether the changes preserve existing hotbar filtering and `OnHotbarChanged` behavior.
