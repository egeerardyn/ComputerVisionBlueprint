# Windows build guide

This repository supports a source build on Windows with **CMake** and **Visual Studio 2022**. The recommended workflow uses the repository `justfile`, which wraps the PowerShell setup and compile scripts.

## Requirements

- Visual Studio 2022 with C++ desktop tooling
- Python 3 on `PATH`
- Git
- CMake
- just

## Recommended workflow

### 1. Setup dependencies

```powershell
just setup
```

`just setup` runs the Windows bootstrap script and performs:

1. Installs **Conan 2.11** and **aqtinstall** with Python.
2. Clones `3rdparty\nodeeditor` if it is missing.
3. Installs **Qt 6.8.3 for MSVC 2022** with the `qtmultimedia` module into `.qt`.
4. Generates Conan outputs for both **Debug** and **Release** under `.conan\Debug`, `.conan\Release`, and target-architecture variants (e.g., `.conan\Debug-arm64`) if multiple target architectures are requested.

By default, `just setup` generates dependencies for both **x64** and **arm64** targets. To build for a specific architecture only:

```powershell
.\scripts\setup.ps1 -TargetArchitectures @("x64")
```

**Note:** Currently, ARM64 OpenCV builds are incompatible with MSVC due to SIMD baseline requirements. If you encounter OpenCV NEON errors during setup on ARM64, this is expected; only x64 dependencies will be available.

### 2. Configure and build

```powershell
just package-release
```

Useful related commands:

- `just build-debug` to build a debug configuration for the native host architecture
- `just build-release` to build a release configuration without deployment
- `just test` to build debug and run the test suite
- `just clean` to remove Windows build directories

For advanced/override options (for example `-QtDir <path>` or `-Clean`), call `scripts/compile.ps1` directly.

## Cross-compilation & architecture support

The build system supports building for different target architectures on Windows. In `just`, architecture resolution is:

1. explicit recipe argument
2. `CVB_ARCH` (or `CVB_TARGET_ARCHS` for `setup`)
3. native host architecture

Use the optional recipe argument to target `x64` or `arm64`:

### x64 (Intel/AMD) architecture

```powershell
just build-debug x64      # Build x64 Debug
just build-release x64    # Build x64 Release
just package-release x64  # Build x64 Release with deployment
just test-debug x64       # Build x64 Debug + run tests
```

### ARM64 architecture

On x64 or ARM64 machines, pass `arm64` to the same recipes:

```powershell
just build-debug arm64        # Build ARM64 Debug
just build-release arm64      # Build ARM64 Release
just package-release arm64    # Build ARM64 Release with deployment
just test-debug arm64         # Build ARM64 Debug + run tests
```

You can also drive the same selection through environment variables:

```powershell
$env:CVB_ARCH = "x64"
just setup
just build-debug

$env:CVB_TARGET_ARCHS = "x64,arm64"
just setup
```

Alternatively, call `compile.ps1` directly with `-Arch`:

```powershell
.\scripts\compile.ps1 -Type Debug -Arch arm64
.\scripts\compile.ps1 -Type Release -Arch arm64 -Deploy
```

### Supported scenarios

| Machine (host) | Target | Status |
|---|---|---|
| x64 | x64 | ✅ Fully supported |
| x64 | ARM64 | ✅ Supported (cross-compile requires MSVC 2022 ARM64 C++ tools) |
| ARM64 | ARM64 | ⚠️ OpenCV incompatibility (SIMD/NEON) — use x64 build on ARM64 instead |
| ARM64 | x64 | ✅ Supported (handled by `--settings=arch=x86_64` override in `compile.ps1`) |

**Note on ARM64 native builds:** OpenCV 4.8.1 with MSVC has known incompatibilities with SIMD baseline optimizations on ARM64 Windows. If you need to build for ARM64, it is recommended to build x64 binaries instead (which work on ARM64 via x86-64 emulation), or investigate OpenCV configuration options with your team.

### 3. Run

```powershell
just run
```

If you omit `-Deploy`, the compiled executable is located at `build\vs2022-release\Release\ComputerVisionBlueprint.exe`.

## Generated layout

- Qt install root: `.qt\6.8.3\msvc2022_64`
- Conan outputs: 
  - x64: `.conan\Debug`, `.conan\Release`
  - ARM64: `.conan\Debug-arm64`, `.conan\Release-arm64` (if build succeeds)
- Visual Studio build folders:
  - x64: `build\vs2022-debug`, `build\vs2022-release`
  - ARM64: `build\vs2022-debug-arm64`, `build\vs2022-release-arm64`
- Deploy output: `build\vs2022-release\dist\ComputerVisionBlueprint.exe` (x64) or `build\vs2022-release-arm64\dist\ComputerVisionBlueprint.exe` (ARM64)

## Notes

- `CMakePresets.json` currently defines **Ninja** presets, so the Windows workflow uses the **Visual Studio 17 2022** generator directly through `compile.ps1` (called by `just`).
- The configure step passes `-DCMAKE_POLICY_DEFAULT_CMP0091=NEW`, the Conan toolchain file, the Qt path, and `OpenCV_DIR` so `find_package(OpenCV CONFIG REQUIRED)` resolves correctly on Windows.
- `windeployqt` may warn that `dxcompiler.dll` and `dxil.dll` are not present; that warning did not prevent the application from launching in the documented Windows workflow.
