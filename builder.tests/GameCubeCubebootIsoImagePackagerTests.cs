using helengine.gamecube.builder.tests.Builders;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the cubeboot ISO image packager that stages a bootable GameCube ISO filesystem and invokes Dockerized ISO authoring.
/// </summary>
public sealed class GameCubeCubebootIsoImagePackagerTests {
    /// <summary>
    /// Ensures the packager rejects a missing generic boot image input.
    /// </summary>
    [Fact]
    public void Package_WhenGenericBootImageDoesNotExist_ThrowsFileNotFoundException() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-cubeboot-tests", Guid.NewGuid().ToString("N"));
        FakeGameCubeProcessRunner processRunner = new(new GameCubeProcessRunResult(0, string.Empty, string.Empty));
        GameCubeCubebootIsoImagePackager packager = new(
            new GameCubeCubebootIsoImagePackagerOptions(rootPath, Path.Combine(rootPath, "missing", "gbi.hdr"), "helengine-gc"),
            processRunner);

        Assert.Throws<FileNotFoundException>(() =>
            packager.Package(CreateLayout(rootPath), Path.Combine(rootPath, "game.gcm"), CancellationToken.None));
    }

    /// <summary>
    /// Ensures the packager stages a cubeboot-compatible ISO filesystem and invokes Dockerized ISO authoring.
    /// </summary>
    [Fact]
    public void Package_WhenInputsAreValid_StagesIsoFilesystemAndInvokesDockerAuthoring() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-cubeboot-tests", Guid.NewGuid().ToString("N"));
        string genericBootImagePath = Path.Combine(rootPath, "native", "gbi.hdr");
        string outputImagePath = Path.Combine(rootPath, "game.gcm");
        Directory.CreateDirectory(Path.GetDirectoryName(genericBootImagePath) ?? throw new InvalidOperationException("Generic boot image directory path could not be resolved."));
        File.WriteAllText(genericBootImagePath, "gbi");

        FakeGameCubeProcessRunner processRunner = new(new GameCubeProcessRunResult(0, "ok", string.Empty));
        GameCubeCubebootIsoImagePackager packager = new(
            new GameCubeCubebootIsoImagePackagerOptions(rootPath, genericBootImagePath, "helengine-gc"),
            processRunner,
            static (isoAuthoringRootPath, destinationImagePath) => File.WriteAllText(destinationImagePath, "gcm"));

        packager.Package(CreateLayout(rootPath), outputImagePath, CancellationToken.None);

        string isoAuthoringRootPath = Path.Combine(rootPath, ".cubeboot-iso-root");
        Assert.True(File.Exists(outputImagePath));
        Assert.True(File.Exists(Path.Combine(isoAuthoringRootPath, "bootldr.dol")));
        Assert.True(File.Exists(Path.Combine(isoAuthoringRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
        Assert.True(File.Exists(Path.Combine(isoAuthoringRootPath, "gbi.hdr")));
        Assert.Equal("rtk", processRunner.LastStartInfo.FileName);
        Assert.Contains("docker", processRunner.LastStartInfo.ArgumentList);
        Assert.Contains("xorriso", processRunner.LastStartInfo.ArgumentList);
        Assert.Contains("bootldr.dol", processRunner.LastStartInfo.ArgumentList);
        Assert.Contains("-boot-load-size", processRunner.LastStartInfo.ArgumentList);
        Assert.Contains("4", processRunner.LastStartInfo.ArgumentList);
    }

    /// <summary>
    /// Creates a minimal extracted-disc layout for cubeboot packager tests.
    /// </summary>
    /// <param name="rootPath">Root path that receives the extracted-disc layout.</param>
    /// <returns>Minimal extracted-disc layout.</returns>
    static GameCubeDiscLayoutResult CreateLayout(string rootPath) {
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
        File.WriteAllText(apploaderImagePath, "apploader");
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
