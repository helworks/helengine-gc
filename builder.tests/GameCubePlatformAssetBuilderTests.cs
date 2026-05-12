using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.gamecube.builder.tests.Builders;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the GameCube builder metadata and payload staging behavior.
/// </summary>
public class GameCubePlatformAssetBuilderTests {
    /// <summary>
    /// Verifies the builder exposes the expected GameCube metadata.
    /// </summary>
    [Fact]
    public void Descriptor_and_definition_expose_gamecube_metadata() {
        GameCubePlatformAssetBuilder builder = new();

        Assert.Equal("helengine.gamecube.builder", builder.Descriptor.BuilderId);
        Assert.Equal("gamecube", builder.Descriptor.TargetPlatformId);
        Assert.Contains("gamecube", builder.Descriptor.SupportedRuntimeBackendIds);
        Assert.Equal("gamecube", builder.Definition.PlatformId);
        Assert.Contains(builder.Definition.BuildProfiles, profile => profile.ProfileId == "gamecube-default");
        Assert.Contains(builder.Definition.GraphicsProfiles, profile => profile.ProfileId == "gx");
    }

    /// <summary>
    /// Verifies the builder copies staged payloads into the output root.
    /// </summary>
    [Fact]
    public async Task BuildAsync_copies_payloads_into_the_output_root() {
        string workingRoot = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string sourceRoot = Path.Combine(workingRoot, "project");
        string sceneSourcePath = Path.Combine(sourceRoot, "scenes", "startup.helen");
        string textureSourcePath = Path.Combine(sourceRoot, "assets", "textures", "checker.png");

        Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath)!);
        Directory.CreateDirectory(Path.GetDirectoryName(textureSourcePath)!);
        await File.WriteAllTextAsync(sceneSourcePath, "scene payload");
        await File.WriteAllTextAsync(textureSourcePath, "texture payload");

        string previousDirectory = Directory.GetCurrentDirectory();
        try {
            Directory.SetCurrentDirectory(sourceRoot);

            PlatformBuildManifest manifest = new(
                1,
                "project",
                "1.0.0",
                "1.0.0",
                [
                    new PlatformBuildScene(
                        "startup",
                        "Startup",
                        "scenes/startup.helen",
                        [],
                        [])
                ],
                [
                    new PlatformBuildAsset(
                        "checker",
                        "Checker Texture",
                        "assets/textures/checker.png",
                        new PlatformBuildPayloadReference("checker-payload", "assets/textures/checker.png"),
                        [])
                ]);

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("gamecube-default", "gamecube", "gamecube", "gamecube-default")],
                [new PlatformCookProfile(
                    "gamecube-default",
                    "GameCube Default",
                    new PlatformCookProfileCapabilities(
                        "gamecube",
                        "raw",
                        "rgba",
                        "gamecube-scene-v1",
                        PlatformSerializationEndianness.BigEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"));

            GameCubePlatformAssetBuilder builder = new();
            RecordingProgressReporter progressReporter = new();
            RecordingDiagnosticReporter diagnosticReporter = new();

            PlatformBuildReport report = await builder.BuildAsync(request, progressReporter, diagnosticReporter, CancellationToken.None);

            Assert.True(report.Succeeded);
            Assert.Empty(diagnosticReporter.Diagnostics);
            Assert.Equal(2, progressReporter.Updates.Count);
            Assert.True(File.Exists(Path.Combine(outputRoot, "scenes", "startup.helen")));
            Assert.True(File.Exists(Path.Combine(outputRoot, "assets", "textures", "checker.png")));
            Assert.True(File.Exists(Path.Combine(workingRoot, "tmp", "gamecube-build-manifest.json")));
        } finally {
            try {
                Directory.SetCurrentDirectory(previousDirectory);
            } catch {
            }

            try {
                if (Directory.Exists(workingRoot)) {
                    Directory.Delete(workingRoot, recursive: true);
                }
            } catch {
            }
        }
    }
}
