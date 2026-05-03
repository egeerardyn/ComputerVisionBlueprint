set windows-shell := ["powershell.exe", "-NoLogo", "-NoProfile", "-Command"]

setup-script := if os_family() == "windows" { ".\\scripts\\setup.ps1" } else { "./scripts/setup.sh" }
compile-script := if os_family() == "windows" { ".\\scripts\\compile.ps1" } else { "./scripts/compile.sh" }

default:
    @just --list

# Windows recipes accept an optional architecture argument.
# Resolution happens inside the PowerShell scripts.

# Install dependencies and prepare Conan/Qt/nodeeditor for the current OS.
setup target_archs='':
    {{ if os_family() == "windows" { ".\\scripts\\setup.ps1 -TargetArchitectures '" + target_archs + "'" } else { setup-script } }}

# Configure and build Debug.
build-debug arch='':
    {{ if os_family() == "windows" { ".\\scripts\\compile.ps1 -Type Debug -Arch \"" + arch + "\"" } else { compile-script + " --type debug --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Configure and build Release.
build-release arch='':
    {{ if os_family() == "windows" { ".\\scripts\\compile.ps1 -Type Release -Arch \"" + arch + "\"" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Configure/build Debug and run the test suite.
test-debug arch='':
    {{ if os_family() == "windows" { ".\\scripts\\compile.ps1 -Type Debug -Arch \"" + arch + "\" -RunTests" } else { compile-script + " --type debug --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\" && ctest --test-dir ./build/linux-debug --output-on-failure" } }}

# Configure/build Release and run the test suite.
test-release arch='':
    {{ if os_family() == "windows" { ".\\scripts\\compile.ps1 -Type Release -Arch \"" + arch + "\" -RunTests" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\" && ctest --test-dir ./build/linux-release --output-on-failure" } }}

# Run default tests in Debug mode.
test arch='':
    just test-debug {{arch}}

# Build a runnable release package on Windows (equivalent to compile.ps1 -Type Release -Deploy).
package-release arch='':
    {{ if os_family() == "windows" { ".\\scripts\\compile.ps1 -Type Release -Arch \"" + arch + "\" -Deploy" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Run the release executable built for the current OS.
run arch='':
    {{ if os_family() == "windows" { ".\\scripts\\compile.ps1 -Type Release -Arch \"" + arch + "\" -RunBuiltApp" } else { "./build/linux-release/ComputerVisionBlueprint" } }}

clean:
    {{ if os_family() == "windows" { "if (Test-Path '.\\build') { Remove-Item '.\\build' -Recurse -Force }" } else { "rm -rf build" } }}
