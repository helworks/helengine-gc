param(
    [string]$ProjectRootPath = "C:\dev\helprojs\city",
    [string]$OutputRootPath = "C:\dev\helprojs\output\gamecube-editor-smoke",
    [string]$ProbeImagePath = "C:\dev\helworks\emus\gc-probe\game.gcm",
    [string]$DolphinExecutablePath = "C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe"
)

$ErrorActionPreference = "Stop"

$RepositoryRootPath = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$HelengineRootPath = (Resolve-Path (Join-Path $RepositoryRootPath "..\\helengine")).Path
$GameCubeBuilderProjectPath = Join-Path $RepositoryRootPath "builder\\helengine.gamecube.builder.csproj"
$EditorApplicationProjectPath = Join-Path $HelengineRootPath "helengine.ui\\helengine.editor.app\\helengine.editor.app.csproj"
$EditorApplicationPath = Join-Path $HelengineRootPath "helengine.ui\\helengine.editor.app\\bin\\Debug\\net9.0-windows\\helengine.editor.app.dll"
$BuiltDiscImagePath = Join-Path $OutputRootPath "game.gcm"
$ProbeDirectoryPath = Split-Path $ProbeImagePath -Parent
$DolphinWorkingDirectoryPath = Split-Path $DolphinExecutablePath -Parent

if (!(Test-Path $GameCubeBuilderProjectPath)) {
    throw "GameCube builder project was not found: $GameCubeBuilderProjectPath"
} elseif (!(Test-Path $EditorApplicationProjectPath)) {
    throw "Editor application project was not found: $EditorApplicationProjectPath"
} elseif (!(Test-Path $ProjectRootPath)) {
    throw "Project root was not found: $ProjectRootPath"
} elseif (!(Test-Path $DolphinExecutablePath)) {
    throw "Dolphin executable was not found: $DolphinExecutablePath"
}

New-Item -ItemType Directory -Force -Path $OutputRootPath | Out-Null
New-Item -ItemType Directory -Force -Path $ProbeDirectoryPath | Out-Null

& rtk dotnet build $GameCubeBuilderProjectPath -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
if ($LASTEXITCODE -ne 0) {
    throw "GameCube builder build failed with exit code $LASTEXITCODE."
}

& rtk dotnet build $EditorApplicationProjectPath -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
if ($LASTEXITCODE -ne 0) {
    throw "Editor application build failed with exit code $LASTEXITCODE."
}

if (!(Test-Path $EditorApplicationPath)) {
    throw "Editor application was not found after build: $EditorApplicationPath"
}

& rtk dotnet $EditorApplicationPath --project (Join-Path $ProjectRootPath "project.heproj") --build gamecube --output $OutputRootPath
if ($LASTEXITCODE -ne 0) {
    throw "Editor-owned GameCube build failed with exit code $LASTEXITCODE."
}

if (!(Test-Path $BuiltDiscImagePath)) {
    throw "Editor-owned GameCube build did not produce a disc image: $BuiltDiscImagePath"
}

Copy-Item -LiteralPath $BuiltDiscImagePath -Destination $ProbeImagePath -Force

Get-Process Dolphin -ErrorAction SilentlyContinue | Stop-Process -Force

Start-Process -FilePath $DolphinExecutablePath -WorkingDirectory $DolphinWorkingDirectoryPath -ArgumentList @("-e", $ProbeImagePath)

Get-Item $ProbeImagePath | Select-Object FullName, Length, LastWriteTime
