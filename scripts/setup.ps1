param(
    [string]$QtVersion = "6.8.3",
    [string]$QtArch = "win64_msvc2022_64",
    [string]$QtInstallRoot = ".qt",
    [string]$ConanVersion = "2.11",
    [string]$NodeEditorRepo = "https://github.com/paceholder/nodeeditor.git"
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
Invoke-Uvx "--from" "conan==$ConanVersion" "conan" "profile" "detect" "--force"

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

Write-Host "Installing Conan dependencies for Debug..."
Invoke-Uvx "--from" "conan==$ConanVersion" "conan" "install" "." "--output-folder=.conan\Debug" "--build=missing" "--settings=build_type=Debug" "--settings=arch=x86_64"

Write-Host "Installing Conan dependencies for Release..."
Invoke-Uvx "--from" "conan==$ConanVersion" "conan" "install" "." "--output-folder=.conan\Release" "--build=missing" "--settings=build_type=Release" "--settings=arch=x86_64"

Write-Host ""
Write-Host "Setup complete."
Write-Host "Qt directory: $((Resolve-Path $qtDir).Path)"
Write-Host "Python user scripts: $userScriptsDir"
Write-Host "uv executable: $script:UvExe"
if ($script:UseUvToolRunFallback) {
    Write-Host "uvx command: unavailable (using 'uv tool run' fallback)"
}
else {
    Write-Host "uvx executable: $script:UvxExe"
}
