
# Milestones

## Short term milestones

### CI pipeline [P0]
- [ ] add a GitHub Actions workflow that builds and runs tests on `ubuntu-latest` (gcc, Release)
- [ ] add a GitHub Actions workflow that builds and runs tests on `windows-latest` (x64 MSVC, Release)
- [ ] upload build artifacts and test results as workflow artifacts on both platforms
- [ ] gate PRs on both CI workflows passing (branch protection rule)

### Cross-compilation on Windows [P1]
- [ ] building ARM64 binaries on an ARM64 machine (native ARM64 Conan profile + MSVC ARM64 toolset)
- [ ] building ARM64 binaries on an x86-64 machine (cross-compile Conan profile + MSVC ARM64 toolset)
- [ ] building x86-64 binaries on an ARM64 machine (already handled by `--settings=arch=x86_64` in setup.ps1)
- [ ] building x86-64 binaries on an x86-64 machine (already supported)
- [ ] expose a `-Arch` parameter in `compile.ps1` (`x64` or `arm64`) that sets the CMake `-A` flag and selects the matching Conan output folder
- [ ] update `setup.ps1` to generate Conan output folders per target arch (e.g. `.conan/Debug-x64`, `.conan/Release-arm64`)
- [ ] add corresponding `just` recipes: `build-debug-arm64`, `package-release-arm64`
- [ ] document all four scenarios and the new `-Arch` parameter in `WINDOWS_BUILD.md`

### VS2026 readiness [P1]
- [ ] update `compile.ps1` generator string from `"Visual Studio 17 2022"` to `"Visual Studio 18 2026"` once VS2026 releases
- [ ] update `CMakePresets.json` and `CMakeUserPresets.json` generator field accordingly
- [ ] verify Conan `compiler.version` for MSVC 194/195 toolset
- [ ] add a `just build-debug-vs2026` recipe and validate the build

### Setup script hardening [P2]
- [ ] detect `uv` version at startup in `setup.ps1` and warn if below a known-good minimum
- [ ] add a `-SkipQt` flag to `setup.ps1` for CI environments where Qt is pre-installed
- [ ] add a `-SkipConan` flag to `setup.ps1` to allow incremental dependency installs
- [ ] make `setup.sh` parity: add `uv`/`uvx` bootstrap and `--settings=arch` override equivalent to the PS1 changes

### Node library [P1/P2]
- [ ] add connected components and contour analysis nodes (labeling, area, perimeter, moments) [P1]
- [ ] add video file stream input node [P1]
- [ ] add an inspector dock that shows the currently selected port's contents (e.g. show the image if it's an image) [P1]
- [ ] add a simple matrix editor to edit small image kernels (e.g. 3x3 up to 21x21) [P2]
- [ ] add a node to load an image kernel from a CSV file [P2]
- [ ] remove the built-in matrix editor on Filter2D and instead take the kernel as an input [P2]
- [ ] add ROI/cropping and mask editing tools/nodes [P2]
- [ ] add graph presets/examples for common workflows (denoise, thresholding, edge detection) [P2]


## Long-term milestone (strategic)

- [ ] add support for cv::Mat of complex, make this in a new division to allow for the following operations: [P2]
  - [ ] complex <-> amplitude and phase
  - [ ] complex <-> real and imaginary
  - [ ] FFT and IFFT in 2 dimensions
  - [ ] fftshift and iffshift
  - [ ] filtering in the frequency domain
  - [ ] a node to pad the image to a given size

- [ ] add a node that is just a Note, i.e. contains multiple lines of text. In code generation it will generate comments [P2]
- [ ] allow to call an external script or program to take input parameters and/or an image and return results and/or an image [P2]
  - [ ] ideally such a script would be written in Python
- [ ] update direct calls to python/pip to use latest best practices such as uv; update the documentation

## Large tasks (need dedicated planning)

- [ ] add graph validation before execution (port compatibility, missing inputs, cycle detection) [P0]
- [ ] add CI pipeline to build and run tests on Windows and Linux [P0]
- [ ] add undo/redo support for graph and node parameter edits [P1]
- [ ] add autosave and crash recovery for the current graph [P1]
- [ ] export the nodes to code [P2]
  - [ ] generate code in C++
  - [ ] generate code in Python
  - [ ] add another panel to show the code, including code highlighting
- [ ] add batch processing mode to execute a graph on all images in a folder [P2]
- [ ] add performance profiling overlay (node execution time and memory usage) [P2]
- [ ] add optional OpenCV CUDA backend support for compatible nodes [P3]
- [ ] add a plugin API for third-party/custom nodes [P3]
