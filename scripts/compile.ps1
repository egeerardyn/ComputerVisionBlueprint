param(
    [ValidateSet("Debug", "Release", "All")]
    [string]$Type = "Release",
    [string]$QtDir = "",
    [string]$Arch = "",
    [switch]$PrintResolvedArch,
    [switch]$Deploy,
    [switch]$Clean,
    [switch]$RunTests,
    [switch]$RunBuiltApp,
    [switch]$CleanOnly
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "..")).Path
Set-Location $repoRoot

function Resolve-BuildArchitecture {
    param([string]$RequestedArch)

    $resolvedArch = $RequestedArch
    if (-not $resolvedArch) {
        $resolvedArch = $env:CVB_ARCH
    }
    if (-not $resolvedArch) {
        $resolvedArch = $env:PROCESSOR_ARCHITECTURE
    }
    if (-not $resolvedArch -and $env:PROCESSOR_ARCHITEW6432) {
        $resolvedArch = $env:PROCESSOR_ARCHITEW6432
    }
    if (-not $resolvedArch) {
        try {
            $resolvedArch = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString()
        }
        catch {
        }
    }
    if (-not $resolvedArch) {
        throw "Unable to determine native architecture. Set CVB_ARCH to x64 or arm64."
    }

    $resolvedArch = $resolvedArch.Trim().ToLowerInvariant()
    switch ($resolvedArch) {
        "amd64" { return "x64" }
        "x86_64" { return "x64" }
        "x64" { return "x64" }
        "arm64" { return "arm64" }
        default { throw "Unsupported architecture '$resolvedArch'. Use x64 or arm64." }
    }
}

function Get-ArchitectureContext {
    param([string]$ResolvedArch)

    return @{
        CMakeArch = if ($ResolvedArch -eq "x64") { "x64" } else { "ARM64" }
        ConanArch = if ($ResolvedArch -eq "x64") { "x86_64" } else { "armv8" }
        ArchSuffix = if ($ResolvedArch -eq "x64") { "" } else { "-$ResolvedArch" }
    }
}

function Remove-BuildDirectories {
    param([string]$ResolvedArch)

    $context = Get-ArchitectureContext -ResolvedArch $ResolvedArch
    foreach ($config in @("debug", "release")) {
        $buildPath = "build\vs2022-$config$($context.ArchSuffix)"
        if (Test-Path $buildPath) {
            Remove-Item $buildPath -Recurse -Force
        }
    }
}

$Arch = Resolve-BuildArchitecture -RequestedArch $Arch
if ($PrintResolvedArch) {
    Write-Output $Arch
    exit 0
}

$archContext = Get-ArchitectureContext -ResolvedArch $Arch

if ($CleanOnly) {
    if ($Type -ne "All") {
        throw "-CleanOnly requires -Type All."
    }

    Remove-BuildDirectories -ResolvedArch $Arch
    exit 0
}

if ($RunBuiltApp -and $Type -eq "All") {
    throw "-RunBuiltApp requires -Type Debug or Release."
}

if ($RunTests -and $Type -eq "All") {
    throw "-RunTests requires -Type Debug or Release."
}

if (-not $QtDir) {
    $defaultQtDir = ".qt\6.8.3\msvc2022_64"
    if (Test-Path (Join-Path $defaultQtDir "bin\qmake.exe")) {
        $QtDir = $defaultQtDir
    }
}

if (-not $QtDir) {
    throw "Qt directory not provided and the default local Qt installation was not found. Use -QtDir <path>."
}

$qtDirPath = (Resolve-Path $QtDir).Path
$lowerType = $Type.ToLowerInvariant()
$buildDir = "build\vs2022-$lowerType$($archContext.ArchSuffix)"
$conanDir = ".conan\$Type$($archContext.ArchSuffix)"

if ($RunBuiltApp) {
    $distExePath = Join-Path $buildDir "dist\ComputerVisionBlueprint.exe"
    $buildExePath = Join-Path $buildDir "$Type\ComputerVisionBlueprint.exe"

    if (Test-Path $distExePath) {
        & (Resolve-Path $distExePath).Path
        exit $LASTEXITCODE
    }

    if (Test-Path $buildExePath) {
        & (Resolve-Path $buildExePath).Path
        exit $LASTEXITCODE
    }

    throw "Built application not found for $Type/$Arch. Build it first."
}

if (-not (Test-Path $conanDir)) {
    throw "Conan dependencies for $Type/$Arch not found at $conanDir. Run 'just setup' to install dependencies."
}

$toolchainPath = (Resolve-Path (Join-Path $conanDir "conan_toolchain.cmake")).Path
$conanConfigDir = (Resolve-Path $conanDir).Path

if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item $buildDir -Recurse -Force
}

Write-Host "Configuring $Type build for $Arch architecture..."
& cmake `
    -S . `
    -B $buildDir `
    -G "Visual Studio 17 2022" `
    -A $archContext.CMakeArch `
    "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW" `
    "-DCMAKE_TOOLCHAIN_FILE=$toolchainPath" `
    "-DCMAKE_PREFIX_PATH=$qtDirPath;$conanConfigDir" `
    "-DOpenCV_DIR=$conanConfigDir"

Write-Host "Building $Type..."
& cmake --build $buildDir --config $Type --parallel

if ($RunTests) {
    $qtBinPath = Join-Path $qtDirPath "bin"
    $testCommand = "set PATH=$qtBinPath;%PATH% && call .\\$conanDir\\conanrunenv-$($lowerType)-$($archContext.ConanArch).bat && ctest --test-dir .\\$buildDir -C $Type --output-on-failure"
    cmd /c $testCommand
    exit $LASTEXITCODE
}

if ($Deploy) {
    $exePath = (Resolve-Path (Join-Path $buildDir "$Type\ComputerVisionBlueprint.exe")).Path
    $qtNodesDll = (Resolve-Path (Join-Path $buildDir "bin\$Type\QtNodes.dll")).Path
    $distDir = Join-Path $buildDir "dist"
    $windeployqt = (Resolve-Path (Join-Path $qtDirPath "bin\windeployqt.exe")).Path

    if (Test-Path $distDir) {
        Remove-Item $distDir -Recurse -Force
    }

    New-Item -ItemType Directory -Path $distDir | Out-Null
    Copy-Item $exePath $distDir
    Copy-Item $qtNodesDll $distDir

    $deployArgs = @("--compiler-runtime", "--dir", $distDir)
    if ($Type -eq "Release") {
        $deployArgs = @("--release") + $deployArgs
    }
    else {
        $deployArgs = @("--debug") + $deployArgs
    }

    Write-Host "Deploying Qt runtime to $distDir..."
    & $windeployqt @deployArgs (Join-Path $distDir "ComputerVisionBlueprint.exe")
}

Write-Host ""
Write-Host "Build complete."
Write-Host "Build directory: $((Resolve-Path $buildDir).Path)"
if ($Deploy) {
    Write-Host "Runnable application: $((Resolve-Path (Join-Path $buildDir "dist\ComputerVisionBlueprint.exe")).Path)"
}
