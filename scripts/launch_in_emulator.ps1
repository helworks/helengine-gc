param(
    [Parameter(Mandatory = $true)]
    [string]$ArtifactPath
)

$ErrorActionPreference = 'Stop'

function Set-IniSectionContent {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,

        [Parameter(Mandatory = $true)]
        [string]$SectionName,

        [Parameter(Mandatory = $true)]
        [string[]]$SectionLines
    )

    $normalizedSource = $Source -replace "`r`n", "`n"
    $lines = @($normalizedSource -split "`n")
    $result = New-Object System.Collections.Generic.List[string]
    $targetHeader = "[{0}]" -f $SectionName
    $replaced = $false

    for ($index = 0; $index -lt $lines.Count; $index++) {
        $line = $lines[$index]
        if ($line -eq $targetHeader) {
            $result.Add($targetHeader)
            foreach ($sectionLine in $SectionLines) {
                $result.Add($sectionLine)
            }

            $replaced = $true
            $index++
            while ($index -lt $lines.Count -and -not $lines[$index].StartsWith('[')) {
                $index++
            }

            if ($index -lt $lines.Count) {
                $index--
            }

            continue
        }

        $result.Add($line)
    }

    if (-not $replaced) {
        while ($result.Count -gt 0 -and [string]::IsNullOrWhiteSpace($result[$result.Count - 1])) {
            $result.RemoveAt($result.Count - 1)
        }

        if ($result.Count -gt 0) {
            $result.Add('')
        }

        $result.Add($targetHeader)
        foreach ($sectionLine in $SectionLines) {
            $result.Add($sectionLine)
        }
    }

    return (($result -join [Environment]::NewLine).TrimEnd() + [Environment]::NewLine)
}

$resolvedArtifactPath = [System.IO.Path]::GetFullPath($ArtifactPath)
if (-not (Test-Path -LiteralPath $resolvedArtifactPath -PathType Leaf)) {
    throw "GameCube artifact was not found: $resolvedArtifactPath"
}

$allowedExtensions = @('.gcm', '.iso', '.gcz')
$artifactExtension = [System.IO.Path]::GetExtension($resolvedArtifactPath).ToLowerInvariant()
if ($allowedExtensions -notcontains $artifactExtension) {
    throw "Expected one of .gcm, .iso, or .gcz but got '$resolvedArtifactPath'."
}

$repositoryRootPath = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$configuredDolphinPath = [Environment]::GetEnvironmentVariable('HELENGINE_GAMECUBE_DOLPHIN_PATH')
if ([string]::IsNullOrWhiteSpace($configuredDolphinPath)) {
    $dolphinPath = [System.IO.Path]::GetFullPath((Join-Path $repositoryRootPath '..\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe'))
} else {
    $dolphinPath = [System.IO.Path]::GetFullPath($configuredDolphinPath)
}

$roamingAppDataRoot = [Environment]::GetFolderPath('ApplicationData')
if ([string]::IsNullOrWhiteSpace($roamingAppDataRoot)) {
    throw 'Windows roaming AppData path could not be resolved.'
}

$configuredProfileRoot = [Environment]::GetEnvironmentVariable('HELENGINE_GAMECUBE_DOLPHIN_PROFILE_ROOT')
if ([string]::IsNullOrWhiteSpace($configuredProfileRoot)) {
    $globalProfileRoot = Join-Path $roamingAppDataRoot 'Dolphin Emulator'
} else {
    $globalProfileRoot = [System.IO.Path]::GetFullPath($configuredProfileRoot)
}

$userDir = Join-Path $repositoryRootPath 'tmp\dolphin-launcher-user'

if (-not (Test-Path -LiteralPath $dolphinPath -PathType Leaf)) {
    throw "Dolphin executable was not found: $dolphinPath"
}

if (-not (Test-Path -LiteralPath $globalProfileRoot -PathType Container)) {
    throw "Dolphin profile root was not found: $globalProfileRoot"
}

$globalQtPath = Join-Path $globalProfileRoot 'Config\Qt.ini'
if (-not (Test-Path -LiteralPath $globalQtPath -PathType Leaf)) {
    throw "Dolphin Qt.ini was not found: $globalQtPath"
}

$globalLoggerPath = Join-Path $globalProfileRoot 'Config\Logger.ini'
if (-not (Test-Path -LiteralPath $globalLoggerPath -PathType Leaf)) {
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
$loggerSource = Set-IniSectionContent -Source $loggerSource -SectionName 'Options' -SectionLines @(
    'WriteToConsole = True',
    'WriteToFile = True',
    'WriteToWindow = True',
    'Verbosity = 1'
)
Set-Content -LiteralPath (Join-Path $userDir 'Config\Logger.ini') -Value $loggerSource -Encoding ASCII

$qtSource = Get-Content -LiteralPath $globalQtPath -Raw
$qtSource = Set-IniSectionContent -Source $qtSource -SectionName 'logging' -SectionLines @(
    'wraplines=false',
    'font=0',
    'logvisible=true',
    'logconfigvisible=true'
)
Set-Content -LiteralPath (Join-Path $userDir 'Config\Qt.ini') -Value $qtSource -Encoding UTF8

$artifactItem = Get-Item -LiteralPath $resolvedArtifactPath

Write-Output ("ARTIFACT=" + $resolvedArtifactPath)
Write-Output ("ARTIFACT_LAST_WRITE_TIME=" + $artifactItem.LastWriteTime.ToString('O'))
Write-Output ("DOLPHIN=" + $dolphinPath)
Write-Output ("USER_DIR=" + $userDir)
Write-Output ("LOGGER_CONFIG=" + (Join-Path $userDir 'Config\Logger.ini'))
Write-Output ("LOG_WINDOW=enabled")

$quotedUserDir = '"' + $userDir + '"'
$quotedArtifactPath = '"' + $resolvedArtifactPath + '"'
$process = Start-Process -FilePath $dolphinPath -ArgumentList '-u', $quotedUserDir, '-e', $quotedArtifactPath -PassThru
Write-Output ("PROCESS_ID=" + $process.Id)
