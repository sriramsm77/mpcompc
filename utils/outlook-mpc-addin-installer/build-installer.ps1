<#
.SYNOPSIS
    Builds the MPC Outlook add-in in Release and packages it as one setup .exe.

.DESCRIPTION
    1. Builds outlook-mpc-addin\src\MpcOutlookAddIn with Visual Studio's MSBuild (found with
       vswhere), which also signs the ClickOnce manifests.
    2. Compiles MpcOutlookAddIn.iss with Inno Setup's ISCC.exe.

    The result is utils\outlook-mpc-addin-installer\Output\MpcOutlookAddIn-Setup-<version>.exe,
    which installs the add-in on any Windows machine with classic Outlook.

    Needs: Visual Studio 2022 with the Office/SharePoint development workload, and Inno Setup 6
    (winget install JRSoftware.InnoSetup).

.PARAMETER SkipBuild
    Package the existing bin\Release output without rebuilding.

.PARAMETER Iscc
    Path to ISCC.exe, if Inno Setup is not in a standard location.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\build-installer.ps1
#>
param(
    [switch]$SkipBuild,
    [string]$Iscc
)

$ErrorActionPreference = 'Stop'
$installerDir = $PSScriptRoot
# This folder is utils\outlook-mpc-addin-installer; the add-in is outlook-mpc-addin at the repo root.
$repoRoot = Split-Path (Split-Path $installerDir -Parent) -Parent
$addInRoot = Join-Path $repoRoot 'outlook-mpc-addin'
$project = Join-Path $addInRoot 'src\MpcOutlookAddIn\MpcOutlookAddIn.csproj'
$releaseDir = Join-Path $addInRoot 'src\MpcOutlookAddIn\bin\Release'

function Find-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) { throw 'Visual Studio was not found (no vswhere.exe). Install Visual Studio 2022 with the Office/SharePoint development workload.' }

    # Prefer an instance with the Office/SharePoint workload, which has the VSTO build targets.
    $msbuild = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Workload.Office -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) {
        $msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
        if ($msbuild) { Write-Warning 'This Visual Studio has no Office/SharePoint development workload; the VSTO build may fail.' }
    }
    if (-not $msbuild) { throw 'MSBuild.exe was not found in any Visual Studio installation.' }
    $msbuild
}

function Find-Iscc {
    if ($Iscc) {
        if (-not (Test-Path $Iscc)) { throw "ISCC.exe not found at $Iscc." }
        return $Iscc
    }

    $candidates = @()
    foreach ($hive in 'HKLM:\SOFTWARE\WOW6432Node', 'HKLM:\SOFTWARE', 'HKCU:\SOFTWARE') {
        $key = Join-Path $hive 'Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 6_is1'
        $location = (Get-ItemProperty -Path $key -Name InstallLocation -ErrorAction SilentlyContinue).InstallLocation
        if ($location) { $candidates += Join-Path $location 'ISCC.exe' }
    }
    $candidates += Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'
    $candidates += Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe'
    $candidates += Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'

    $found = $candidates | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1
    if (-not $found) {
        throw 'Inno Setup 6 was not found. Install it with "winget install JRSoftware.InnoSetup" (or from https://jrsoftware.org/isdl.php), or pass -Iscc <path to ISCC.exe>.'
    }
    $found
}

if (-not $SkipBuild) {
    $msbuild = Find-MSBuild
    Write-Host "Building the add-in (Release) with $msbuild"
    & $msbuild $project /t:Rebuild /p:Configuration=Release /p:Platform=AnyCPU /v:minimal /nologo
    if ($LASTEXITCODE -ne 0) { throw "The add-in build failed (MSBuild exit code $LASTEXITCODE)." }
}

foreach ($file in 'MpcOutlookAddIn.dll', 'MpcOutlookAddIn.dll.manifest', 'MpcOutlookAddIn.vsto',
                  'Microsoft.Office.Tools.Common.v4.0.Utilities.dll', 'Microsoft.Office.Tools.Outlook.v4.0.Utilities.dll') {
    if (-not (Test-Path (Join-Path $releaseDir $file))) {
        throw "$file is missing from $releaseDir. Build the add-in in Release first (run without -SkipBuild)."
    }
}

$version = (Get-Item (Join-Path $releaseDir 'MpcOutlookAddIn.dll')).VersionInfo.FileVersion
$compiler = Find-Iscc
Write-Host "Packaging version $version with $compiler"
& $compiler /Q "/DSourceDir=$releaseDir" "/DAppVersion=$version" (Join-Path $installerDir 'MpcOutlookAddIn.iss')
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed (ISCC exit code $LASTEXITCODE)." }

$setup = Join-Path $installerDir "Output\MpcOutlookAddIn-Setup-$version.exe"
Write-Host ''
Write-Host "Installer: $setup"
