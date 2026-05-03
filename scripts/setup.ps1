param(
    [string]$QtVersion = "6.8.3",
    [string]$QtArch = "win64_msvc2022_64",
    [string]$QtInstallRoot = ".qt",
    [string]$ConanVersion = "2.11",
    [string]$NodeEditorRepo = "https://github.com/paceholder/nodeeditor.git",
    [string[]]$TargetArchitectures = @()
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "..")).Path
Set-Location $repoRoot

function Add-UserPythonScriptsToPath {
    $pythonVersionDir = python -c "import sys; print(f'Python{sys.version_info.major}{sys.version_info.minor}')"
    $scriptsDir = Join-Path $env:APPDATA "Python\$pythonVersionDir\Scripts"

    if (Test-Path $scriptsDir) {
        $pathEntries = $env:Path -split ";"
        if ($scriptsDir -notin $pathEntries) {
            $env:Path = "$scriptsDir;$env:Path"
        }
    }

    return $scriptsDir
}

function Get-QtFolderName {
    param([string]$Arch)

    switch ($Arch) {
        "win64_msvc2022_64" { return "msvc2022_64" }
        "win64_msvc2022_arm64_cross_compiled" { return "msvc2022_arm64_cross_compiled" }
        default { return ($Arch -replace "^win64_", "") }
    }
}

function Resolve-BuildArchitecture {
    param([string]$RequestedArch)

    if (-not $RequestedArch) {
        throw "Architecture value cannot be empty. Use x64 or arm64."
    }

    $resolvedArch = $RequestedArch.Trim().ToLowerInvariant()
    switch ($resolvedArch) {
        "amd64" { return "x64" }
        "x86_64" { return "x64" }
        "x64" { return "x64" }
        "arm64" { return "arm64" }
        default { throw "Unsupported architecture '$resolvedArch'. Use x64 or arm64." }
    }
}

function Resolve-TargetArchitectures {
    param([string[]]$RequestedArchitectures)

    $rawArchitectures = @($RequestedArchitectures | Where-Object { $_ })
    if ($rawArchitectures.Count -eq 0) {
        if ($env:CVB_TARGET_ARCHS) {
            $rawArchitectures = @($env:CVB_TARGET_ARCHS)
        }
        elseif ($env:CVB_ARCH) {
            $rawArchitectures = @($env:CVB_ARCH)
        }
        else {
            $nativeArchitecture = $env:PROCESSOR_ARCHITECTURE
            if (-not $nativeArchitecture -and $env:PROCESSOR_ARCHITEW6432) {
                $nativeArchitecture = $env:PROCESSOR_ARCHITEW6432
            }
            if (-not $nativeArchitecture) {
                try {
                    $nativeArchitecture = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString()
                }
                catch {
                }
            }
            if (-not $nativeArchitecture) {
                throw "Unable to determine native architecture. Set CVB_TARGET_ARCHS or CVB_ARCH explicitly."
            }

            $rawArchitectures = @($nativeArchitecture)
        }
    }

    $resolvedArchitectures = New-Object System.Collections.Generic.List[string]
    foreach ($entry in $rawArchitectures) {
        foreach ($candidate in ($entry -split ',')) {
            $candidate = $candidate.Trim()
            if (-not $candidate) {
                continue
            }

            $resolvedCandidate = Resolve-BuildArchitecture -RequestedArch $candidate
            if (-not $resolvedArchitectures.Contains($resolvedCandidate)) {
                $resolvedArchitectures.Add($resolvedCandidate)
            }
        }
    }

    return $resolvedArchitectures.ToArray()
}

function Ensure-Uv {
    $uvCmd = Get-Command uv -ErrorAction SilentlyContinue
    if ($uvCmd) {
        return $uvCmd.Source
    }

    Write-Host "uv not found. Installing uv with pip..."
    python -m pip install --user --upgrade uv

    Add-UserPythonScriptsToPath | Out-Null
    $uvCmd = Get-Command uv -ErrorAction SilentlyContinue
    if (-not $uvCmd) {
        throw "uv was installed but is still not available on PATH. Restart shell or add the Python Scripts folder to PATH."
    }

    return $uvCmd.Source
}

function Invoke-Uvx {
    param(
        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Args
    )

    if ($script:UseUvToolRunFallback) {
        & $script:UvExe tool run @Args
    }
    else {
        & $script:UvxExe @Args
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Command 'uvx $Args' exited with code $LASTEXITCODE"
    }
}

Write-Host "Ensuring uv tooling is available..."
$userScriptsDir = Add-UserPythonScriptsToPath
$script:UvExe = Ensure-Uv

$TargetArchitectures = Resolve-TargetArchitectures -RequestedArchitectures $TargetArchitectures

$uvxCmd = Get-Command uvx -ErrorAction SilentlyContinue
if ($uvxCmd) {
    $script:UvxExe = $uvxCmd.Source
    $script:UseUvToolRunFallback = $false
}
else {
    Write-Host "uvx command not found. Falling back to 'uv tool run'."
    $script:UseUvToolRunFallback = $true
}

Write-Host "Detecting Conan profile..."
# Note: conan profile detect may output warnings but still succeed; we suppress error checking for this step
$ErrorActionPreference = "SilentlyContinue"
if ($script:UseUvToolRunFallback) {
    & $script:UvExe tool run "--from" "conan==$ConanVersion" "conan" "profile" "detect" "--force" 2>$null | Out-Null
}
else {
    & $script:UvxExe "--from" "conan==$ConanVersion" "conan" "profile" "detect" "--force" 2>$null | Out-Null
}
$ErrorActionPreference = "Stop"

if (-not (Test-Path "3rdparty")) {
    New-Item -ItemType Directory -Path "3rdparty" | Out-Null
}

if (-not (Test-Path "3rdparty\nodeeditor")) {
    Write-Host "Cloning nodeeditor..."
    git clone --depth 1 $NodeEditorRepo "3rdparty\nodeeditor"
}
else {
    Write-Host "nodeeditor already present. Skipping clone."
}

$qtFolderName = Get-QtFolderName -Arch $QtArch
$qtDir = Join-Path $QtInstallRoot "$QtVersion\$qtFolderName"
$qmakePath = Join-Path $qtDir "bin\qmake.exe"

if (-not (Test-Path $qmakePath)) {
    Write-Host "Installing Qt $QtVersion ($QtArch)..."
    Invoke-Uvx "--from" "aqtinstall" "aqt" "install-qt" "windows" "desktop" $QtVersion $QtArch "-m" "qtmultimedia" "-O" $QtInstallRoot
}
else {
    Write-Host "Qt already present at $qtDir. Skipping Qt install."
}

Write-Host "Installing Conan dependencies for each target architecture..."
$ErrorActionPreference = "SilentlyContinue"
foreach ($targetArch in $TargetArchitectures) {
    $conanArch = if ($targetArch -eq "x64") { "x86_64" } else { "armv8" }
    $archSuffix = if ($targetArch -eq "x64") { "" } else { "-$targetArch" }

    Write-Host "  Installing for $targetArch (Conan arch: $conanArch)..."
    Write-Host "    Debug build..."
    Invoke-Uvx "--from" "conan==$ConanVersion" "conan" "install" "." "--output-folder=.conan\Debug$archSuffix" "--build=missing" "--settings=build_type=Debug" "--settings=arch=$conanArch"
    if ($LASTEXITCODE -ne 0 -and $targetArch -eq "arm64") {
        Write-Host "    Warning: ARM64 (armv8) OpenCV build failed. This is expected on ARM64 Windows due to SIMD incompatibilities."
        Write-Host "    Skipping ARM64 builds. Use -Arch x64 on this ARM64 machine (which works via the x86_64 override)."
        continue
    }

    Write-Host "    Release build..."
    Invoke-Uvx "--from" "conan==$ConanVersion" "conan" "install" "." "--output-folder=.conan\Release$archSuffix" "--build=missing" "--settings=build_type=Release" "--settings=arch=$conanArch"
    if ($LASTEXITCODE -ne 0 -and $targetArch -eq "arm64") {
        Write-Host "    Warning: ARM64 (armv8) OpenCV Release build failed."
        Write-Host "    Removing incomplete ARM64 folders..."
        if (Test-Path ".conan\Release$archSuffix") { Remove-Item ".conan\Release$archSuffix" -Recurse -Force }
        continue
    }
}
$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "Setup complete."
Write-Host "Qt directory: $((Resolve-Path $qtDir).Path)"
Write-Host "Target architectures: $($TargetArchitectures -join ', ')"
Write-Host "Python user scripts: $userScriptsDir"
Write-Host "uv executable: $script:UvExe"
if ($script:UseUvToolRunFallback) {
    Write-Host "uvx command: unavailable (using 'uv tool run' fallback)"
}
else {
    Write-Host "uvx executable: $script:UvxExe"
}
