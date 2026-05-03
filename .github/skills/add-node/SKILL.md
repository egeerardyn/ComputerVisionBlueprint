---
name: add-node
description: 'Scaffold a new node end-to-end in ComputerVisionBlueprint. Use when adding OpenCV, Images, Video, Variables, Data Operations, Basic, or Constants nodes, including file creation, CMake wiring, registry registration, help metadata reminders, and Windows build validation.'
argument-hint: 'Node name, category, inputs/outputs, and intended behavior'
user-invocable: true
---

# Add Node Skill

Create a new node in this repository with all required wiring so it appears in the UI and builds on Windows.

## When To Use

- Adding a new node model in `src/Nodes/<Category>/`
- Adding a related Qt form (`.ui`) for node settings
- Registering a new node so it appears in the node palette
- Updating help metadata expectations for Help dock behavior

## Inputs

Collect or infer:

1. Node class name (example: `HoughCirclesModel`)
2. Category folder under `src/Nodes/` (example: `OpenCV`, `Images`)
3. Palette group shown in UI (example: `OpenCV`, `Images`, `Variables`)
4. Ports: input/output data types and labels
5. Parameters shown in embedded widget or form
6. Error behavior for invalid input datatype/shape/channels

## Procedure

1. Validate scope and naming.
   - Confirm this is a node addition task, not a generic refactor.
   - Use existing naming style: `<Feature>Model` and related `<Feature>Form.ui` where applicable.

2. Add node source files.
   - Create implementation/header in `src/Nodes/<Category>/`.
   - If needed, add a `.ui` form and wire it from the model.
   - Follow nearby node patterns in the same category.

3. Wire build system.
   - Add new files to `add_executable(ComputerVisionBlueprint ...)` in `CMakeLists.txt`.
   - Keep the source list grouped consistently by category.

4. Wire include hub.
   - Include the new node header in `src/Nodes/NodesInclude.h`.

5. Register in model registry.
   - Add `ret->registerModel<YourNode>("<Group>");` in `MainWindow::registerDataModels()` in `src/Widgets/MainWindow.cpp`.
   - Choose the palette group intentionally to match existing UX.

6. Help metadata reminder.
   - If node needs custom help summary/url, follow `src/Nodes/NodeHelpInfo.cpp` patterns.
   - If no custom metadata is added, ensure default help text is acceptable.

7. Validate on Windows.
   - Build with PowerShell:
     - `./scripts/compile.ps1 -Type Debug`
   - If runtime packaging behavior changed, also run:
     - `./scripts/compile.ps1 -Type Release -Deploy`

8. Perform quick manual checks.
   - Node appears in expected palette group.
   - Ports connect only to compatible data types.
   - Invalid input produces clear error behavior (no crash).
   - Save/load workflow preserves the node configuration.

## Decision Points

- If a similar node exists in the same category:
  - Reuse its widget/parameter/validation pattern.
- If the node introduces a new data type:
  - Add corresponding type in `src/Nodes/Data/` and include via `DataInclude.h` as needed.
- If behavior overlaps an existing node:
  - Prefer extending existing node only when asked; otherwise create minimal standalone node.
- If node needs external files or URLs:
  - Reuse patterns from Variables/File nodes and validate missing-file behavior.

## Completion Criteria

- Files created in correct `src/Nodes/<Category>/` location
- `CMakeLists.txt` updated with all new files
- `src/Nodes/NodesInclude.h` updated
- `MainWindow::registerDataModels()` updated with correct group
- Debug build succeeds on Windows
- Manual smoke-check passes (node visible, usable, no crash on bad input)

## Repository References

- Build workflow: [WINDOWS_BUILD.md](../../../WINDOWS_BUILD.md)
- Main registry location: [src/Widgets/MainWindow.cpp](../../../src/Widgets/MainWindow.cpp)
- Include hub: [src/Nodes/NodesInclude.h](../../../src/Nodes/NodesInclude.h)
- Node help utility: [src/Nodes/NodeHelpInfo.cpp](../../../src/Nodes/NodeHelpInfo.cpp)
- Source list wiring: [CMakeLists.txt](../../../CMakeLists.txt)
