using helengine.gamecube.builder.tests.Builders;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the explicit Wiimms ISO Tools configuration boundary before real packaging logic is added.
/// </summary>
public sealed class GameCubeWiimmsIsoToolsImagePackagerTests {
    /// <summary>
    /// Ensures the packager rejects a missing configured executable path.
    /// </summary>
    [Fact]
    public void Package_WhenExecutablePathIsBlank_ThrowsInvalidOperationException() {
        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(string.Empty),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, string.Empty, string.Empty)));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
            packager.Package(CreateLayout(), Path.Combine(Path.GetTempPath(), "game.gcm"), CancellationToken.None));

        Assert.Contains("wit", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Ensures the packager rejects a configured executable path that does not exist.
    /// </summary>
    [Fact]
    public void Package_WhenExecutablePathDoesNotExist_ThrowsFileNotFoundException() {
        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(Path.Combine(Path.GetTempPath(), "missing", "wit.exe")),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, string.Empty, string.Empty)));

        Assert.Throws<FileNotFoundException>(() =>
            packager.Package(CreateLayout(), Path.Combine(Path.GetTempPath(), "game.gcm"), CancellationToken.None));
    }

    /// <summary>
    /// Ensures the packager surfaces non-zero Wiimms ISO Tools exits as build failures.
    /// </summary>
    [Fact]
    public void Package_WhenWitReturnsNonZero_ThrowsInvalidOperationException() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
        string executablePath = Path.Combine(workingRootPath, "wit.exe");
        Directory.CreateDirectory(workingRootPath);
        File.WriteAllText(executablePath, "fake");

        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(executablePath),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(7, "stdout", "stderr")));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
            packager.Package(CreateLayout(), Path.Combine(workingRootPath, "game.gcm"), CancellationToken.None));

        Assert.Contains("exit code", exception.Message, StringComparison.OrdinalIgnoreCase);
        Assert.Contains("stderr", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Ensures the packager accepts a successful Wiimms ISO Tools process when an image artifact is materialized.
    /// </summary>
    [Fact]
    public void Package_WhenWitSucceedsAndImageExists_WritesGameCubeImageArtifact() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
        string executablePath = Path.Combine(workingRootPath, "wit.exe");
        string outputImagePath = Path.Combine(workingRootPath, "game.gcm");
        Directory.CreateDirectory(workingRootPath);
        File.WriteAllText(executablePath, "fake");

        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(executablePath),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, "ok", string.Empty)),
            static (layout, destinationImagePath) => File.WriteAllText(destinationImagePath, "gcm"));

        packager.Package(CreateLayout(), outputImagePath, CancellationToken.None);

        Assert.True(File.Exists(outputImagePath));
    }

    /// <summary>
    /// Ensures the packager forces plain ISO output so <c>wit</c> does not silently emit a WBFS container for GameCube images.
    /// </summary>
    [Fact]
    public void Package_WhenWitRunsDirectly_RequestsIsoOutputMode() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
        string executablePath = Path.Combine(workingRootPath, "wit.exe");
        string outputImagePath = Path.Combine(workingRootPath, "game.gcm");
        Directory.CreateDirectory(workingRootPath);
        File.WriteAllText(executablePath, "fake");
        FakeGameCubeProcessRunner processRunner = new(new GameCubeProcessRunResult(0, "ok", string.Empty));

        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(executablePath),
            processRunner,
            static (layout, destinationImagePath) => File.WriteAllText(destinationImagePath, "gcm"));

        packager.Package(CreateLayout(), outputImagePath, CancellationToken.None);

        Assert.Contains("--iso", processRunner.LastStartInfo.ArgumentList);
        Assert.Equal("COPY", processRunner.LastStartInfo.ArgumentList[0]);
    }

    /// <summary>
    /// Ensures the packager fails early with an actionable diagnostic when the extracted-disc source is missing required GameCube system files.
    /// </summary>
    [Fact]
    public void Package_WhenDiscSourceIsMissingApploaderImage_ThrowsInvalidOperationException() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
        string executablePath = Path.Combine(workingRootPath, "wit.exe");
        Directory.CreateDirectory(workingRootPath);
        File.WriteAllText(executablePath, "fake");

        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(executablePath),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, "ok", string.Empty)));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
            packager.Package(CreateLayout(includeApploaderImage: false), Path.Combine(workingRootPath, "game.gcm"), CancellationToken.None));

        Assert.Contains("sys/apploader.img", exception.Message, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures Linux-hosted builds can invoke a Windows wit executable through the Windows host shell when the configured path points at a mounted Windows file.
    /// </summary>
    [Fact]
    public void Package_WhenLinuxUsesWindowsWitExecutable_UsesPowershellWrapper() {
        if (!OperatingSystem.IsLinux()) {
            return;
        }

        string workingRootPath = Path.Combine("/mnt/c/dev/helworks/helengine-gc/tmp", "gamecube-wit-wrapper-tests", Guid.NewGuid().ToString("N"));
        string executablePath = Path.Combine(workingRootPath, "wit.exe");
        string outputImagePath = Path.Combine(workingRootPath, "game.gcm");
        Directory.CreateDirectory(workingRootPath);
        File.WriteAllText(executablePath, "fake");
        FakeGameCubeProcessRunner processRunner = new(new GameCubeProcessRunResult(0, "ok", string.Empty));

        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(executablePath),
            processRunner,
            static (layout, destinationImagePath) => File.WriteAllText(destinationImagePath, "gcm"));

        packager.Package(CreateLayout(workingRootPath), outputImagePath, CancellationToken.None);

        Assert.Equal("powershell.exe", processRunner.LastStartInfo.FileName);
        Assert.Contains("-NoProfile", processRunner.LastStartInfo.ArgumentList);
        Assert.Contains("C:\\dev\\helworks\\helengine-gc\\tmp", processRunner.LastStartInfo.ArgumentList[2], StringComparison.Ordinal);
        Assert.Contains("--iso", processRunner.LastStartInfo.ArgumentList[2], StringComparison.Ordinal);
    }

    /// <summary>
    /// Creates a minimal extracted-disc layout for packager tests.
    /// </summary>
    /// <returns>Minimal extracted-disc layout.</returns>
    static GameCubeDiscLayoutResult CreateLayout(bool includeApploaderImage = true) {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
        return CreateLayout(rootPath, includeApploaderImage);
    }

    /// <summary>
    /// Creates a minimal extracted-disc layout for packager tests at the supplied root path.
    /// </summary>
    /// <param name="rootPath">Root path that receives the extracted-disc layout.</param>
    /// <param name="includeApploaderImage">Whether to include the required apploader image.</param>
    /// <returns>Minimal extracted-disc layout.</returns>
    static GameCubeDiscLayoutResult CreateLayout(string rootPath, bool includeApploaderImage = true) {
        string discRootPath = Path.Combine(rootPath, "disc");
        string bootBinPath = Path.Combine(discRootPath, "sys", "boot.bin");
        string bi2BinPath = Path.Combine(discRootPath, "sys", "bi2.bin");
        string apploaderImagePath = Path.Combine(discRootPath, "sys", "apploader.img");
        string mainDolPath = Path.Combine(discRootPath, "sys", "main.dol");
        string scenePath = Path.Combine(discRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset");
        Directory.CreateDirectory(Path.GetDirectoryName(mainDolPath) ?? throw new InvalidOperationException("Main DOL directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(scenePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
        File.WriteAllText(bootBinPath, "boot");
        File.WriteAllText(bi2BinPath, "bi2");
        if (includeApploaderImage) {
            File.WriteAllText(apploaderImagePath, "apploader");
        }
        File.WriteAllText(mainDolPath, "dol");
        File.WriteAllText(scenePath, "scene");
        return new GameCubeDiscLayoutResult(
            discRootPath,
            mainDolPath,
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
                ["cooked/scenes/rendering/cube_test.hasset"] = "files/cooked/scenes/rendering/cube_test.hasset"
            });
    }
}
