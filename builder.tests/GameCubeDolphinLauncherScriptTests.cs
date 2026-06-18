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
        Assert.Contains("if (-not (Test-Path -LiteralPath $userDir))", scriptSource, StringComparison.Ordinal);
        Assert.DoesNotContain("Remove-Item -LiteralPath $userDir -Recurse -Force", scriptSource, StringComparison.Ordinal);
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
        Assert.Contains("persistent dedicated Dolphin user directory", readmeSource, StringComparison.OrdinalIgnoreCase);
        Assert.DoesNotContain("recreates an isolated Dolphin user directory", readmeSource, StringComparison.OrdinalIgnoreCase);
    }
}
