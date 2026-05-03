set windows-shell := ["powershell.exe", "-NoLogo", "-NoProfile", "-Command"]

setup-script := if os_family() == "windows" { ".\\scripts\\setup.ps1" } else { "./scripts/setup.sh" }
compile-script := if os_family() == "windows" { ".\\scripts\\compile.ps1" } else { "./scripts/compile.sh" }

default:
    @just --list

# Install dependencies and prepare Conan/Qt/nodeeditor for the current OS.
setup:
    {{ setup-script }}

# Configure and build Debug.
build-debug:
    {{ if os_family() == "windows" { compile-script + " -Type Debug" } else { compile-script + " --type debug --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Configure and build Release.
build-release:
    {{ if os_family() == "windows" { compile-script + " -Type Release" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Configure/build Debug and run the test suite.
test-debug:
    {{ if os_family() == "windows" { compile-script + " -Type Debug; cmd /c \"set PATH=%CD%\\.qt\\6.8.3\\msvc2022_64\\bin;%PATH% && call .\\.conan\\Debug\\conanrunenv-debug-x86_64.bat && ctest --test-dir .\\build\\vs2022-debug -C Debug --output-on-failure\"" } else { compile-script + " --type debug --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\" && ctest --test-dir ./build/linux-debug --output-on-failure" } }}

# Configure/build Release and run the test suite.
test-release:
    {{ if os_family() == "windows" { compile-script + " -Type Release; cmd /c \"set PATH=%CD%\\.qt\\6.8.3\\msvc2022_64\\bin;%PATH% && call .\\.conan\\Release\\conanrunenv-release-x86_64.bat && ctest --test-dir .\\build\\vs2022-release -C Release --output-on-failure\"" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\" && ctest --test-dir ./build/linux-release --output-on-failure" } }}

# Run default tests in Debug mode.
test: test-debug

# Build a runnable release package on Windows (equivalent to compile.ps1 -Type Release -Deploy).
package-release:
    {{ if os_family() == "windows" { compile-script + " -Type Release -Deploy" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Run the release executable built for the current OS.
run:
    {{ if os_family() == "windows" { "if (Test-Path '.\\build\\vs2022-release\\dist\\ComputerVisionBlueprint.exe') { & '.\\build\\vs2022-release\\dist\\ComputerVisionBlueprint.exe' } else { & '.\\build\\vs2022-release\\Release\\ComputerVisionBlueprint.exe' }" } else { "./build/linux-release/ComputerVisionBlueprint" } }}

clean:
    {{ if os_family() == "windows" { "if (Test-Path '.\\build\\vs2022-debug') { Remove-Item '.\\build\\vs2022-debug' -Recurse -Force }; if (Test-Path '.\\build\\vs2022-release') { Remove-Item '.\\build\\vs2022-release' -Recurse -Force }" } else { "rm -rf build/linux-debug build/linux-release" } }}
