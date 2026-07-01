# GameCube Dolphin Launcher and README Alignment Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Align the GameCube README and Dolphin launcher workflow with the Wii repo pattern while keeping GameCube-specific `.gcm` terminology and behavior accurate.

**Architecture:** Add a focused checked-in Dolphin launcher script under `scripts/`, guard its explicit contract with a new builder test file, and rewrite the top of `README.md` so the day-to-day workflow matches the Wii repo shape. Keep the existing low-level GameCube build notes, but demote them below the primary editor-build and Dolphin-launch flow.

**Tech Stack:** PowerShell, .NET 9, xUnit, existing GameCube builder test project

---

## File Structure

- Create: `scripts/launch_gamecube_image_in_dolphin.ps1`
  Purpose: Explicit GameCube image launcher for Dolphin using an isolated per-repo Dolphin profile and forced logger visibility.
- Create: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`
  Purpose: Guard the launcher script contract and README documentation contract.
- Modify: `README.md`
  Purpose: Reorder the top-level developer workflow to match the Wii repo structure closely while preserving GC-specific sections.
- Reference only: `tools/build_and_launch_editor_gamecube_probe.ps1`
  Purpose: Existing heavier probe script that should remain untouched.
- Reference only: `C:\dev\helworks\helengine-wii\README.md`
  Purpose: Structural template for the GC README workflow sections.
- Reference only: `C:\dev\helworks\helengine-wii\scripts\launch_wii_iso_in_dolphin.ps1`
  Purpose: Behavioral template for the new GC launcher script.

### Task 1: Add Failing Contract Tests

**Files:**
- Create: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`
- Test: `builder.tests/helengine.gamecube.builder.tests.csproj`

- [ ] **Step 1: Write the failing launcher and README contract tests**

```csharp
namespace helengine.gamecube.builder.tests;

/// <summary>
/// Guards the developer launcher contract for running explicit GameCube disc images in Dolphin.
/// </summary>
public sealed class GameCubeDolphinLauncherScriptTests {
    /// <summary>
    /// Ensures the launcher keeps an explicit image path contract, force-closes Dolphin, prints image timestamp data, and seeds the logging profile.
    /// </summary>
    [Fact]
    public void DolphinImageLauncher_KeepsExplicitImagePathAndLoggingProfileContract() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string scriptPath = Path.Combine(repositoryRootPath, "scripts", "launch_gamecube_image_in_dolphin.ps1");

        Assert.True(File.Exists(scriptPath), "Expected scripts/launch_gamecube_image_in_dolphin.ps1 to exist.");

        string scriptSource = File.ReadAllText(scriptPath);

        Assert.Contains("[Parameter(Mandatory = $true)]", scriptSource, StringComparison.Ordinal);
        Assert.Contains("[string]$ImagePath", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Get-Process -Name 'Dolphin'", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Stop-Process", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Get-Item -LiteralPath $resolvedImagePath", scriptSource, StringComparison.Ordinal);
        Assert.Contains("LastWriteTime", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Qt.ini", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Logger.ini", scriptSource, StringComparison.Ordinal);
        Assert.Contains("GC", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Backup", scriptSource, StringComparison.Ordinal);
        Assert.Contains("ResourcePacks", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Get-Content -LiteralPath $globalLoggerPath -Raw", scriptSource, StringComparison.Ordinal);
        Assert.Contains("Set-Content -LiteralPath (Join-Path $userDir 'Config\\Logger.ini')", scriptSource, StringComparison.Ordinal);
        Assert.Contains("WriteToWindow = True", scriptSource, StringComparison.Ordinal);
        Assert.Contains("WriteToConsole = True", scriptSource, StringComparison.Ordinal);
        Assert.Contains("WriteToFile = True", scriptSource, StringComparison.Ordinal);
        Assert.Contains("logvisible=true", scriptSource, StringComparison.Ordinal);
        Assert.Contains("logconfigvisible=true", scriptSource, StringComparison.Ordinal);
        Assert.Contains("'-u', $userDir, '-e', $resolvedImagePath", scriptSource, StringComparison.Ordinal);
        Assert.Contains("PROCESS_ID=", scriptSource, StringComparison.Ordinal);
        Assert.DoesNotContain("game.gcm", scriptSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the README documents the explicit GameCube image launcher workflow for Dolphin.
    /// </summary>
    [Fact]
    public void Readme_DocumentsExplicitImageLauncherWorkflow() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string readmeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "README.md"));

        Assert.Contains("launch_gamecube_image_in_dolphin.ps1", readmeSource, StringComparison.Ordinal);
        Assert.Contains("-ImagePath", readmeSource, StringComparison.Ordinal);
        Assert.Contains("process id", readmeSource, StringComparison.OrdinalIgnoreCase);
        Assert.Contains("logger window", readmeSource, StringComparison.OrdinalIgnoreCase);
        Assert.Contains("Logger.ini", readmeSource, StringComparison.Ordinal);
        Assert.Contains("global Dolphin profile", readmeSource, StringComparison.OrdinalIgnoreCase);
    }
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "GameCubeDolphinLauncherScriptTests"
```

Expected:

```text
FAIL
Expected scripts/launch_gamecube_image_in_dolphin.ps1 to exist.
```

- [ ] **Step 3: Commit the failing-test baseline**

```bash
git add builder.tests/GameCubeDolphinLauncherScriptTests.cs
git commit -m "test: add gamecube dolphin launcher contract coverage"
```

### Task 2: Implement the Launcher Script

**Files:**
- Create: `scripts/launch_gamecube_image_in_dolphin.ps1`
- Test: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`

- [ ] **Step 1: Write the minimal launcher script to satisfy the contract**

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$ImagePath
)

$ErrorActionPreference = 'Stop'

$resolvedImagePath = [System.IO.Path]::GetFullPath($ImagePath)
if (-not (Test-Path -LiteralPath $resolvedImagePath)) {
    throw "GameCube image was not found: $resolvedImagePath"
}

$repositoryRootPath = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$dolphinPath = 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe'
$globalProfileRoot = 'C:\Users\Helena\AppData\Roaming\Dolphin Emulator'
$userDir = Join-Path $repositoryRootPath 'tmp\dolphin-launcher-user'

if (-not (Test-Path -LiteralPath $dolphinPath)) {
    throw "Dolphin executable was not found: $dolphinPath"
}

if (-not (Test-Path -LiteralPath $globalProfileRoot)) {
    throw "Dolphin profile root was not found: $globalProfileRoot"
}

$globalQtPath = Join-Path $globalProfileRoot 'Config\Qt.ini'
$globalLoggerPath = Join-Path $globalProfileRoot 'Config\Logger.ini'
if (-not (Test-Path -LiteralPath $globalQtPath)) {
    throw "Dolphin Qt.ini was not found: $globalQtPath"
}

if (-not (Test-Path -LiteralPath $globalLoggerPath)) {
    throw "Dolphin Logger.ini was not found: $globalLoggerPath"
}

$existingDolphinProcesses = @(Get-Process -Name 'Dolphin' -ErrorAction SilentlyContinue)
foreach ($process in $existingDolphinProcesses) {
    Stop-Process -Id $process.Id -Force
}

if (Test-Path -LiteralPath $userDir) {
    Remove-Item -LiteralPath $userDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $userDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $userDir 'Config') | Out-Null

foreach ($directoryName in @('GC', 'Backup', 'ResourcePacks', 'Load')) {
    $sourceDirectoryPath = Join-Path $globalProfileRoot $directoryName
    if (Test-Path -LiteralPath $sourceDirectoryPath) {
        Copy-Item -LiteralPath $sourceDirectoryPath -Destination (Join-Path $userDir $directoryName) -Recurse -Force
    }
}

$loggerSource = Get-Content -LiteralPath $globalLoggerPath -Raw
$loggerOptionsSection = @'
[Options]
WriteToConsole = True
WriteToFile = True
WriteToWindow = True
Verbosity = 1
'@
if ($loggerSource -match '(?ms)^\[Options\].*?(?=^\[|\z)') {
    $loggerSource = [System.Text.RegularExpressions.Regex]::Replace($loggerSource, '(?ms)^\[Options\].*?(?=^\[|\z)', $loggerOptionsSection + [Environment]::NewLine)
} else {
    $loggerSource = $loggerSource.TrimEnd() + [Environment]::NewLine + [Environment]::NewLine + $loggerOptionsSection + [Environment]::NewLine
}

Set-Content -LiteralPath (Join-Path $userDir 'Config\Logger.ini') -Value $loggerSource -Encoding ASCII

$qtSource = Get-Content -LiteralPath $globalQtPath -Raw
$loggingSection = @'
[logging]
wraplines=false
font=0
logvisible=true
logconfigvisible=true
'@
if ($qtSource -match '(?ms)^\[logging\].*?(?=^\[|\z)') {
    $qtSource = [System.Text.RegularExpressions.Regex]::Replace($qtSource, '(?ms)^\[logging\].*?(?=^\[|\z)', $loggingSection + [Environment]::NewLine)
} else {
    $qtSource = $qtSource.TrimEnd() + [Environment]::NewLine + [Environment]::NewLine + $loggingSection + [Environment]::NewLine
}

Set-Content -LiteralPath (Join-Path $userDir 'Config\Qt.ini') -Value $qtSource -Encoding UTF8

$imageItem = Get-Item -LiteralPath $resolvedImagePath

Write-Output ("IMAGE=" + $resolvedImagePath)
Write-Output ("IMAGE_LAST_WRITE_TIME=" + $imageItem.LastWriteTime.ToString('O'))
Write-Output ("DOLPHIN=" + $dolphinPath)
Write-Output ("USER_DIR=" + $userDir)
Write-Output ("LOGGER_CONFIG=" + (Join-Path $userDir 'Config\Logger.ini'))
Write-Output ("LOG_WINDOW=enabled")

$process = Start-Process -FilePath $dolphinPath -ArgumentList '-u', $userDir, '-e', $resolvedImagePath -PassThru
Write-Output ("PROCESS_ID=" + $process.Id)
```

- [ ] **Step 2: Run the focused test to verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "GameCubeDolphinLauncherScriptTests"
```

Expected:

```text
PASS
```

- [ ] **Step 3: Commit the launcher script**

```bash
git add scripts/launch_gamecube_image_in_dolphin.ps1 builder.tests/GameCubeDolphinLauncherScriptTests.cs
git commit -m "feat: add gamecube dolphin launcher script"
```

### Task 3: Rewrite the README Around the Shared Workflow

**Files:**
- Modify: `README.md`
- Test: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`

- [ ] **Step 1: Rewrite the top-level README workflow sections**

```markdown
# Helengine GameCube Host

This repository contains the native GameCube host, the GameCube platform builder integration, and the GameCube-specific runtime source audits for Helengine.

## Current status

- The shared editor CLI can build GameCube packages with platform id `gamecube`.
- The packaged-disc output is a raw `game.gcm` image suitable for Dolphin launch verification.
- The repository keeps lower-level Docker and generated-core workflows for native runtime bring-up.
- Source-audit coverage exists for GameCube packaged runtime generation, disc layout staging, and image packaging.

## Editor CLI build

If your workspace keeps `helengine-gc`, `helengine`, and `helprojs` as sibling directories, use the shared wrapper like this:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform gamecube `
  -Output ..\helprojs\city\gamecube-build
```

That wrapper runs the main editor CLI with `--build gamecube` and writes the generated GameCube package to the output directory you provide.

## Launching in Dolphin

Use the checked-in launcher script:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_gamecube_image_in_dolphin.ps1 `
  -ImagePath .\tmp\packaged-disc-proof-life\game.gcm
```

The launcher requires an explicit `-ImagePath`. Before launch it force-closes any running `Dolphin.exe` processes, recreates an isolated Dolphin user directory under `tmp\`, copies the `GC`, `Backup`, `ResourcePacks`, and `Load` directories from the global Dolphin profile when present, seeds `Logger.ini` from the global Dolphin profile channel set, forces `WriteToConsole`, `WriteToFile`, and `WriteToWindow` on in the isolated profile, and forces the Dolphin logger window visible in the isolated `Qt.ini`.

The launcher prints:

- the image path
- the image last write time
- the Dolphin executable path
- the isolated user directory path
- the seeded `Logger.ini` path
- that the logger window is enabled
- the spawned Dolphin process id

It then starts Dolphin with `-u <userdir> -e <image>`.

## Verification

Run the focused GameCube audit suite:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "GameCubeDolphinLauncherScriptTests|GameCubePackagedRuntimeSourceTests|GameCubePackagedBuildWorkspaceTests|GameCubeRawImagePackagerTests"
```
```

- [ ] **Step 2: Run the focused test to verify the README contract passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "GameCubeDolphinLauncherScriptTests"
```

Expected:

```text
PASS
```

- [ ] **Step 3: Commit the README alignment**

```bash
git add README.md
git commit -m "docs: align gamecube readme with dolphin workflow"
```

### Task 4: Run Final Focused Verification

**Files:**
- Modify: `README.md`
- Modify: `scripts/launch_gamecube_image_in_dolphin.ps1`
- Modify: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`
- Test: `builder.tests/helengine.gamecube.builder.tests.csproj`

- [ ] **Step 1: Run the focused verification suite**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "GameCubeDolphinLauncherScriptTests|GameCubePackagedRuntimeSourceTests|GameCubePackagedBuildWorkspaceTests|GameCubeRawImagePackagerTests"
```

Expected:

```text
PASS
```

- [ ] **Step 2: Inspect the final diff**

Run:

```bash
git diff -- README.md scripts/launch_gamecube_image_in_dolphin.ps1 builder.tests/GameCubeDolphinLauncherScriptTests.cs
```

Expected:

```text
Shows only the README rewrite, the new launcher script, and the new contract test file.
```

- [ ] **Step 3: Commit the completed workflow**

```bash
git add README.md scripts/launch_gamecube_image_in_dolphin.ps1 builder.tests/GameCubeDolphinLauncherScriptTests.cs
git commit -m "feat: align gamecube dolphin workflow with wii pattern"
```
