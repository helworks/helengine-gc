using helengine.baseplatform.Manifest;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the packaged GameCube runtime scene manifest emitted for native startup.
/// </summary>
public sealed class GameCubeRuntimeSceneManifestWriterTests {
    /// <summary>
    /// Ensures the writer emits a startup-scene function and a cooked scene catalog that the native bootstrap can consume.
    /// </summary>
    [Fact]
    public void Write_EmitsHeaderWithStartupSceneAndCookedSceneCatalog() {
        string outputRootPath = Path.Combine(Path.GetTempPath(), "gamecube-runtime-manifest-tests", Guid.NewGuid().ToString("N"));
        PlatformBuildManifest manifest = new(
            1,
            "project",
            "1.0.0",
            "1.0.0",
            "gamecube",
            "1.0.0",
            "cube-test",
            [
                new PlatformBuildScene(
                    "cube-test",
                    "Cube Test",
                    "Scenes/rendering/cube_test.helen",
                    [],
                    [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/rendering/cube_test.hasset")])
            ],
            Array.Empty<PlatformBuildAsset>(),
            Array.Empty<PlatformBuildArtifact>(),
            Array.Empty<PlatformBuildCodeModule>(),
            Array.Empty<PlatformArtifactPlacement>(),
            new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));

        try {
            GameCubeRuntimeSceneManifestWriter writer = new();
            writer.Write(outputRootPath, manifest);

            string header = File.ReadAllText(Path.Combine(outputRootPath, "runtime", "gamecube_runtime_scene_manifest.hpp"));
            Assert.Contains("he_get_runtime_gamecube_startup_scene_id", header, StringComparison.Ordinal);
            Assert.Contains("he_get_runtime_gamecube_scene_entries", header, StringComparison.Ordinal);

            string source = File.ReadAllText(Path.Combine(outputRootPath, "runtime", "gamecube_runtime_scene_manifest.inl"));
            Assert.Contains("\"cube-test\"", source, StringComparison.Ordinal);
            Assert.Contains("\"cooked/scenes/rendering/cube_test.hasset\"", source, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(outputRootPath)) {
                Directory.Delete(outputRootPath, recursive: true);
            }
        }
    }
}
