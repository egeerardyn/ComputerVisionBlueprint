set windows-shell := ["powershell.exe", "-NoLogo", "-NoProfile", "-Command"]

setup-script := if os_family() == "windows" { ".\\scripts\\setup.ps1" } else { "./scripts/setup.sh" }
compile-script := if os_family() == "windows" { ".\\scripts\\compile.ps1" } else { "./scripts/compile.sh" }

default:
    @just --list

# Windows recipes accept an optional architecture argument.
# Resolution order: recipe argument -> CVB_ARCH / CVB_TARGET_ARCHS -> native host architecture.

# Install dependencies and prepare Conan/Qt/nodeeditor for the current OS.
setup target_archs='':
    {{ if os_family() == "windows" { "$targetArchs = if ('" + target_archs + "' -ne '') { '" + target_archs + "' } elseif ($env:CVB_TARGET_ARCHS) { $env:CVB_TARGET_ARCHS } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $archList = @($targetArchs -split ',' | ForEach-Object { $_.Trim().ToLowerInvariant() } | Where-Object { $_ }); $archList = @($archList | ForEach-Object { if ($_ -eq 'amd64') { 'x64' } else { $_ } }); $invalidArchs = @($archList | Where-Object { $_ -notin @('x64', 'arm64') }); if ($invalidArchs.Count -gt 0) { throw \"Unsupported architecture list '$targetArchs'. Use x64 and/or arm64.\" }; & .\\scripts\\setup.ps1 -TargetArchitectures $archList" } else { setup-script } }}

# Configure and build Debug.
build-debug arch='':
    {{ if os_family() == "windows" { "$arch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $arch = $arch.ToLowerInvariant(); if ($arch -eq 'amd64') { $arch = 'x64' }; if ($arch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$arch'. Use x64 or arm64.\" }; & .\\scripts\\compile.ps1 -Type Debug -Arch $arch" } else { compile-script + " --type debug --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Configure and build Release.
build-release arch='':
    {{ if os_family() == "windows" { "$arch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $arch = $arch.ToLowerInvariant(); if ($arch -eq 'amd64') { $arch = 'x64' }; if ($arch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$arch'. Use x64 or arm64.\" }; & .\\scripts\\compile.ps1 -Type Release -Arch $arch" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Configure/build Debug and run the test suite.
test-debug arch='':
    {{ if os_family() == "windows" { "$arch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $arch = $arch.ToLowerInvariant(); if ($arch -eq 'amd64') { $arch = 'x64' }; if ($arch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$arch'. Use x64 or arm64.\" }; $buildSuffix = if ($arch -eq 'x64') { '' } else { '-arm64' }; $conanSuffix = if ($arch -eq 'x64') { 'x86_64' } else { 'armv8' }; & .\\scripts\\compile.ps1 -Type Debug -Arch $arch; cmd /c \"set PATH=%CD%\\.qt\\6.8.3\\msvc2022_64\\bin;%PATH% && call .\\.conan\\Debug$buildSuffix\\conanrunenv-debug-$conanSuffix.bat && ctest --test-dir .\\build\\vs2022-debug$buildSuffix -C Debug --output-on-failure\"" } else { compile-script + " --type debug --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\" && ctest --test-dir ./build/linux-debug --output-on-failure" } }}

# Configure/build Release and run the test suite.
test-release arch='':
    {{ if os_family() == "windows" { "$arch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $arch = $arch.ToLowerInvariant(); if ($arch -eq 'amd64') { $arch = 'x64' }; if ($arch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$arch'. Use x64 or arm64.\" }; $buildSuffix = if ($arch -eq 'x64') { '' } else { '-arm64' }; $conanSuffix = if ($arch -eq 'x64') { 'x86_64' } else { 'armv8' }; & .\\scripts\\compile.ps1 -Type Release -Arch $arch; cmd /c \"set PATH=%CD%\\.qt\\6.8.3\\msvc2022_64\\bin;%PATH% && call .\\.conan\\Release$buildSuffix\\conanrunenv-release-$conanSuffix.bat && ctest --test-dir .\\build\\vs2022-release$buildSuffix -C Release --output-on-failure\"" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\" && ctest --test-dir ./build/linux-release --output-on-failure" } }}

# Run default tests in Debug mode.
test arch='':
    just test-debug {{arch}}

# Build a runnable release package on Windows (equivalent to compile.ps1 -Type Release -Deploy).
package-release arch='':
    {{ if os_family() == "windows" { "$arch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $arch = $arch.ToLowerInvariant(); if ($arch -eq 'amd64') { $arch = 'x64' }; if ($arch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$arch'. Use x64 or arm64.\" }; & .\\scripts\\compile.ps1 -Type Release -Arch $arch -Deploy" } else { compile-script + " --type release --qt \"${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}\"" } }}

# Run the release executable built for the current OS.
run arch='':
    {{ if os_family() == "windows" { "$arch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant() }; $arch = $arch.ToLowerInvariant(); if ($arch -eq 'amd64') { $arch = 'x64' }; if ($arch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$arch'. Use x64 or arm64.\" }; $buildSuffix = if ($arch -eq 'x64') { '' } else { '-arm64' }; if (Test-Path \".\\build\\vs2022-release$buildSuffix\\dist\\ComputerVisionBlueprint.exe\") { & \".\\build\\vs2022-release$buildSuffix\\dist\\ComputerVisionBlueprint.exe\" } else { & \".\\build\\vs2022-release$buildSuffix\\Release\\ComputerVisionBlueprint.exe\" }" } else { "./build/linux-release/ComputerVisionBlueprint" } }}

clean arch='':
    {{ if os_family() == "windows" { "$requestedArch = if ('" + arch + "' -ne '') { '" + arch + "' } elseif ($env:CVB_ARCH) { $env:CVB_ARCH } else { '' }; if (-not $requestedArch) { if (Test-Path '.\\build\\vs2022-debug') { Remove-Item '.\\build\\vs2022-debug' -Recurse -Force }; if (Test-Path '.\\build\\vs2022-release') { Remove-Item '.\\build\\vs2022-release' -Recurse -Force }; if (Test-Path '.\\build\\vs2022-debug-arm64') { Remove-Item '.\\build\\vs2022-debug-arm64' -Recurse -Force }; if (Test-Path '.\\build\\vs2022-release-arm64') { Remove-Item '.\\build\\vs2022-release-arm64' -Recurse -Force } } else { $requestedArch = $requestedArch.ToLowerInvariant(); if ($requestedArch -eq 'amd64') { $requestedArch = 'x64' }; if ($requestedArch -notin @('x64', 'arm64')) { throw \"Unsupported architecture '$requestedArch'. Use x64 or arm64.\" }; $buildSuffix = if ($requestedArch -eq 'x64') { '' } else { '-arm64' }; if (Test-Path \".\\build\\vs2022-debug$buildSuffix\") { Remove-Item \".\\build\\vs2022-debug$buildSuffix\" -Recurse -Force }; if (Test-Path \".\\build\\vs2022-release$buildSuffix\") { Remove-Item \".\\build\\vs2022-release$buildSuffix\" -Recurse -Force } }" } else { "rm -rf build/linux-debug build/linux-release" } }}
