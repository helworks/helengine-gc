using System.Reflection;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies GameCube builder path resolution uses the GameCube builder assembly location as the repository-root anchor.
/// </summary>
public sealed class GameCubeBuilderPathsTests {
    /// <summary>
    /// Ensures the repository-root resolver finds the GameCube repository root from the builder assembly location.
    /// </summary>
    [Fact]
    public void ResolveRepositoryRootPath_WhenInvokedFromBuilderAssemblyLocation_ReturnsGameCubeRepositoryRoot() {
        MethodInfo resolveRepositoryRootPathMethod = typeof(GameCubeBuilderPaths).GetMethod(
            "ResolveRepositoryRootPath",
            BindingFlags.Static | BindingFlags.NonPublic);

        Assert.NotNull(resolveRepositoryRootPathMethod);

        string repositoryRootPath = (string)resolveRepositoryRootPathMethod.Invoke(null, null);
        string expectedRepositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));

        Assert.Equal(expectedRepositoryRootPath, repositoryRootPath);
        Assert.True(File.Exists(Path.Combine(repositoryRootPath, "Makefile")));
        Assert.True(File.Exists(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp")));
    }

    /// <summary>
    /// Ensures the repository-root resolver prefers the explicit editor-owned environment override when present.
    /// </summary>
    [Fact]
    public void ResolveRepositoryRootPath_WhenRepositoryRootEnvironmentVariableIsSet_ReturnsConfiguredRepositoryRoot() {
        string expectedRepositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string previousRepositoryRootPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT");

        try {
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT", expectedRepositoryRootPath);

            MethodInfo resolveRepositoryRootPathMethod = typeof(GameCubeBuilderPaths).GetMethod(
                "ResolveRepositoryRootPath",
                BindingFlags.Static | BindingFlags.NonPublic);

            Assert.NotNull(resolveRepositoryRootPathMethod);

            string repositoryRootPath = (string)resolveRepositoryRootPathMethod.Invoke(null, null);

            Assert.Equal(expectedRepositoryRootPath, repositoryRootPath);
        } finally {
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT", previousRepositoryRootPath);
        }
    }

    /// <summary>
    /// Ensures packaged builder paths use the resolved GameCube repository root when constructing one workspace.
    /// </summary>
    [Fact]
    public void Create_WhenBuildingWorkspace_UsesResolvedRepositoryRootPath() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-builder-path-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "output");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");

        try {
            Directory.CreateDirectory(outputRootPath);
            Directory.CreateDirectory(generatedCoreRootPath);

            PlatformBuildRequest request = new(
                new PlatformBuildManifest(
                    1,
                    "project",
                    "1.0.0",
                    "1.0.0",
                    "gamecube",
                    "1.0.0",
                    "startup",
                    [],
                    [],
                    [],
                    [],
                    [],
                    new PlatformContainerWritePlan("disc-layout", [])),
                [new PlatformBuildTargetVariant("gamecube-default", "gamecube", "gamecube", "gamecube-default")],
                [new PlatformCookProfile(
                    "gamecube-default",
                    "GameCube Default",
                    new PlatformCookProfileCapabilities(
                        "gamecube",
                        "gx",
                        "raw",
                        "gamecube-scene-v1",
                        PlatformSerializationEndianness.BigEndian))],
                outputRootPath,
                workingRootPath,
                string.Empty,
                string.Empty,
                string.Empty,
                null,
                null,
                null,
                generatedCoreRootPath);

            GameCubeBuilderPaths paths = GameCubeBuilderPaths.Create(request);
            string expectedRepositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));

            Assert.Equal(expectedRepositoryRootPath, paths.RepositoryRootPath);
        } finally {
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, true);
            }
        }
    }
}
