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
if (-not (Test-Path -LiteralPath $globalQtPath)) {
    throw "Dolphin Qt.ini was not found: $globalQtPath"
}

$globalLoggerPath = Join-Path $globalProfileRoot 'Config\Logger.ini'
if (-not (Test-Path -LiteralPath $globalLoggerPath)) {
    throw "Dolphin Logger.ini was not found: $globalLoggerPath"
}

$existingDolphinProcesses = @(Get-Process -Name 'Dolphin' -ErrorAction SilentlyContinue)
foreach ($process in $existingDolphinProcesses) {
    Stop-Process -Id $process.Id -Force
}

if (-not (Test-Path -LiteralPath $userDir)) {
    New-Item -ItemType Directory -Force -Path $userDir | Out-Null

    foreach ($directoryName in @('GC', 'Backup', 'ResourcePacks', 'Load')) {
        $sourceDirectoryPath = Join-Path $globalProfileRoot $directoryName
        if (Test-Path -LiteralPath $sourceDirectoryPath) {
            Copy-Item -LiteralPath $sourceDirectoryPath -Destination (Join-Path $userDir $directoryName) -Recurse -Force
        }
    }
}

New-Item -ItemType Directory -Force -Path (Join-Path $userDir 'Config') | Out-Null

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
