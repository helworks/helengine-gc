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
        const string StartupSceneId = "Scenes/DemoDiscMainMenu.helen";
        const string StartupSceneCookedRelativePath = "cooked/scenes/DemoDiscMainMenu.hasset";
        const string CanonicalStartupSceneCookedRelativePath = "cooked/scenes/demodiscmainmenu.hasset";
        PlatformBuildManifest manifest = new(
            1,
            "project",
            "1.0.0",
            "1.0.0",
            "gamecube",
            "1.0.0",
            StartupSceneId,
            [
                new PlatformBuildScene(
                    StartupSceneId,
                    "Demo Disc Main Menu",
                    "Scenes/DemoDiscMainMenu.helen",
                    [],
                    [new KeyValuePair<string, string>("cooked-relative-path", StartupSceneCookedRelativePath)])
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
            Assert.Contains("\"Scenes/DemoDiscMainMenu.helen\"", source, StringComparison.Ordinal);
            Assert.Contains("\"" + CanonicalStartupSceneCookedRelativePath + "\"", source, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(outputRootPath)) {
                Directory.Delete(outputRootPath, recursive: true);
            }
        }
    }
}
