using System.Diagnostics;
using System.Reflection;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies GameCube Docker native-build invocation maps generated-core roots correctly for editor-owned builds.
/// </summary>
public sealed class GameCubeDockerNativeBuildExecutorTests {
    /// <summary>
    /// Ensures the Docker executor compiles generated-core output as emitted instead of invoking a GameCube-specific generated-code rewriter.
    /// </summary>
    [Fact]
    public void Source_DoesNotUseGeneratedCoreNormalizer() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));

        Assert.DoesNotContain("GameCubeGeneratedCoreCompatibilityNormalizer", source, StringComparison.Ordinal);
        Assert.DoesNotContain(".Normalize(paths.GeneratedCoreRootPath)", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures external generated-core roots are mounted separately into Docker and exported through one absolute container path.
    /// </summary>
    [Fact]
    public void CreateStartInfo_WhenGeneratedCoreRootIsOutsideRepositoryRoot_MountsGeneratedCoreSeparately() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "gamecube-docker-generated-core", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(generatedCoreRootPath);

        try {
            GameCubeBuilderPaths paths = new(
                repositoryRootPath,
                generatedCoreRootPath,
                Path.Combine(repositoryRootPath, "tmp", "staged-content"),
                Path.Combine(repositoryRootPath, "tmp", "disc"),
                Path.Combine(repositoryRootPath, "tmp", "game.gcm"),
                Path.Combine(repositoryRootPath, "tmp", "native", "helengine_gc.dol"),
                Path.Combine(repositoryRootPath, "tmp", "native", "apploader.img"),
                Path.Combine(repositoryRootPath, "tmp", "native", "gbi.hdr"));
            MethodInfo createStartInfoMethod = typeof(GameCubeDockerNativeBuildExecutor).GetMethod(
                "CreateStartInfo",
                BindingFlags.Static | BindingFlags.NonPublic);

            Assert.NotNull(createStartInfoMethod);

            ProcessStartInfo startInfo = (ProcessStartInfo)createStartInfoMethod.Invoke(null, [paths]);

            Assert.Contains(repositoryRootPath + ":/workspace", startInfo.ArgumentList);
            Assert.Contains(generatedCoreRootPath + ":/helengine-generated-core", startInfo.ArgumentList);
            Assert.Contains("HELENGINE_CORE_CPP_ROOT=/helengine-generated-core", startInfo.ArgumentList);
            Assert.False(startInfo.UseShellExecute);
            Assert.True(startInfo.RedirectStandardOutput);
            Assert.True(startInfo.RedirectStandardError);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }
}
