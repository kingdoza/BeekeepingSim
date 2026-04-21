# AGENTS.md

## Overview
BeekeepingSim is a simulation game project built using Unreal Engine. This document provides essential knowledge for AI coding agents to be productive in this codebase. It covers the architecture, workflows, conventions, and integration points specific to this project.

---

## Project Architecture

### Key Components
- **Source/**: Contains the core C++ source code for the game and editor targets.
  - `BeekeepingSim.Target.cs` and `BeekeepingSimEditor.Target.cs`: Define build targets for the game and editor.
  - `BeekeepingSim/`: Main game logic and systems.
- **Content/**: Stores Unreal Engine assets such as Blueprints, maps, and materials.
  - `Beekeeper/`: Assets related to the beekeeper character and gameplay.
  - `Variant_Horror/` and `Variant_Shooter/`: Thematic variants of the game.
- **Config/**: Configuration files for engine and project settings.
- **Intermediate/**, **DerivedDataCache/**, **Saved/**: Generated directories for build artifacts, cached data, and runtime logs.

### Data Flow
- Game logic is primarily implemented in C++ (Source/) and extended via Blueprints (Content/).
- Assets and configurations are loaded dynamically based on the game mode or variant.

---

## Developer Workflows

### Building the Project
- Use the Unreal Engine Editor to build and run the project.
- Alternatively, build from the command line:
  ```powershell
  & "C:\Path\To\UnrealBuildTool.exe" -project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -game
  ```

### Testing
- Automated tests are not explicitly defined in the current structure. Manual testing is performed via the Unreal Editor.

### Debugging
- Use Visual Studio to debug C++ code. Attach the debugger to the Unreal Editor process.
- Unreal Engine logs are stored in `Saved/Logs/`.

---

## Project-Specific Conventions

### Blueprints
- Blueprints are organized by feature or theme (e.g., `Beekeeper/`, `Variant_Horror/`).
- Naming convention: `BP_<Feature><Type>` (e.g., `BP_BeekeeperCharacter`).

### C++ Code
- Follow Unreal Engine coding standards.
- Game-specific logic resides in `Source/BeekeepingSim/`.

### Asset Management
- Assets are grouped by functionality or theme.
- Use descriptive names and maintain folder hierarchy.

---

## Integration Points

### External Dependencies
- Unreal Engine: Ensure the correct version is installed.
- No additional dependencies are explicitly defined.

### Cross-Component Communication
- C++ classes interact with Blueprints for gameplay logic.
- Game modes (`BP_BeekeepingGameMode`) define rules and behaviors for different variants.

---

## Key Files and Directories
- `Source/BeekeepingSim/`: Core game logic.
- `Content/Beekeeper/`: Beekeeper-related assets.
- `Config/`: Project configuration files.
- `Saved/Logs/`: Runtime logs for debugging.

---

This document should be updated as the project evolves to reflect new patterns and workflows.
