# AGENTS.md

Guidance for AI coding agents working in this repository.

## Scope

- Primary target in this repo is Windows development with Visual Studio 2022.
- Prefer minimal, surgical changes and keep existing code style.
- Treat `3rdparty/` as vendored dependencies: do not modify unless explicitly asked.

## Read First

- Project overview and cross-platform notes: [README.md](README.md)
- Windows build workflow: [WINDOWS_BUILD.md](WINDOWS_BUILD.md)
- Setup script details: [scripts/setup.ps1](scripts/setup.ps1)
- Build/deploy script details: [scripts/compile.ps1](scripts/compile.ps1)
- Current roadmap/backlog: [TASKS.md](TASKS.md)

## Windows Build Commands

Run from repository root in PowerShell.

1. One-time setup and dependency resolution:

```powershell
.\scripts\setup.ps1
```

2. Configure/build Release and deploy runtime:

```powershell
.\scripts\compile.ps1 -Type Release -Deploy
```

3. Configure/build Debug:

```powershell
.\scripts\compile.ps1 -Type Debug
```

4. Run packaged executable (when `-Deploy` used):

```powershell
.\build\vs2022-release\dist\ComputerVisionBlueprint.exe
```

## Validation Expectations

- For C++/Qt/OpenCV changes, build at least once with:
  - `./scripts/compile.ps1 -Type Debug`
- If packaging or runtime DLL behavior is touched, also run:
  - `./scripts/compile.ps1 -Type Release -Deploy`
- There is no established automated test suite yet; rely on successful build plus focused manual checks in the app.

## Architecture Map

- App entrypoint: [src/main.cpp](src/main.cpp)
- Main UI + graph setup: [src/Widgets/MainWindow.cpp](src/Widgets/MainWindow.cpp)
- Node model includes hub: [src/Nodes/NodesInclude.h](src/Nodes/NodesInclude.h)
- Node help registry utilities: [src/Nodes/NodeHelpInfo.cpp](src/Nodes/NodeHelpInfo.cpp)
- Node implementations by domain:
  - `src/Nodes/OpenCV/`
  - `src/Nodes/Images/`
  - `src/Nodes/Video/`
  - `src/Nodes/Variables/`
  - `src/Nodes/DataOperations/`
  - `src/Nodes/Basic/`
  - `src/Nodes/Constants/`
  - `src/Nodes/Data/`

## Adding A New Node

When implementing a new node, update all relevant integration points:

1. Add the model source/header (and optional `.ui`) under the appropriate `src/Nodes/<Category>/` folder.
2. Add new files to the executable source list in [CMakeLists.txt](CMakeLists.txt).
3. Include the model header in [src/Nodes/NodesInclude.h](src/Nodes/NodesInclude.h).
4. Register the model/category in `MainWindow::registerDataModels()` in [src/Widgets/MainWindow.cpp](src/Widgets/MainWindow.cpp).
5. If user-facing help should appear in the Help dock, provide/extend node help metadata via [src/Nodes/NodeHelpInfo.cpp](src/Nodes/NodeHelpInfo.cpp) patterns.

## Windows-Specific Pitfalls

- `scripts/compile.ps1` expects Conan outputs in `.conan/Debug` or `.conan/Release`; run `scripts/setup.ps1` first.
- If Qt is not in `.qt/6.8.3/msvc2022_64`, pass `-QtDir` to `compile.ps1`.
- `CMakePresets.json` uses Ninja presets; Windows script-based flow uses Visual Studio generator directly.
- If `ui_MainWindow.h` appears unresolved in editor diagnostics, perform a build so Qt AUTOUIC-generated files are created.

## Change Discipline

- Do not mix refactors with feature fixes unless requested.
- Keep new dependencies minimal and justified.
- Update documentation when behavior or workflow changes:
  - User-facing build/use changes: [README.md](README.md), [WINDOWS_BUILD.md](WINDOWS_BUILD.md)
  - Roadmap/task changes: [TASKS.md](TASKS.md)
- Keep commits small and focussed on a single task.
