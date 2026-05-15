using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.gamecube.builder.tests.Builders;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the packaged GameCube builder workspace emits staged disc-layout outputs when supplied fake native-build collaborators.
/// </summary>
public sealed class GameCubePackagedBuildWorkspaceTests {
    /// <summary>
    /// Ensures the packaged builder stages cooked artifacts, writes the runtime manifest, builds the DOL, writes the extracted disc layout, and emits the packaged image artifact.
    /// </summary>
    [Fact]
    public async Task BuildPackagedAsync_WritesDiscRootAndImageArtifact() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-packaged-build-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string apploaderImagePath = Path.Combine(workingRootPath, "tooling", "apploader.img");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string modelSourcePath = Path.Combine(sourceRootPath, "cooked", "engine", "models", "cube.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(modelSourcePath) ?? throw new InvalidOperationException("Model directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllTextAsync(modelSourcePath, "model");
            await File.WriteAllTextAsync(apploaderImagePath, "apploader");
            Directory.SetCurrentDirectory(sourceRootPath);

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
                [
                    new PlatformBuildArtifact("cooked/scenes/rendering/cube_test.hasset", "scene-content-hash", "scene", "gamecube-default"),
                    new PlatformBuildArtifact("cooked/engine/models/cube.hasset", "model-content-hash", "model", "gamecube-default")
                ],
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));

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
                outputRootPath,
                Path.Combine(workingRootPath, "tmp"),
                string.Empty,
                string.Empty,
                string.Empty,
                null,
                null,
                null,
                generatedCoreRootPath);

            RecordingProgressReporter progressReporter = new();
            RecordingDiagnosticReporter diagnosticReporter = new();
            FakeGameCubeNativeBuildExecutor nativeBuildExecutor = new();
            FakeGameCubeImagePackager imagePackager = new();

            PlatformBuildReport report = await GameCubeBuildWorkspace.BuildPackagedAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None,
                nativeBuildExecutor,
                imagePackager,
                new GameCubeDiscSystemAreaOptions(apploaderImagePath, "PROJHE", "project"));

            Assert.True(report.Succeeded);
            Assert.Empty(diagnosticReporter.Diagnostics);
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "main.dol")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "boot.bin")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "bi2.bin")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "apploader.img")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "engine", "models", "cube.hasset")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "game.gcm")));
            Assert.True(File.Exists(Path.Combine(generatedCoreRootPath, "runtime", "gamecube_runtime_scene_manifest.hpp")));
        } finally {
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures packaged builds no longer require an explicit Wiimms ISO Tools path when the direct raw GameCube image writer is used.
    /// </summary>
    [Fact]
    public async Task BuildPackagedAsync_WhenWitPathEnvironmentVariableIsMissing_WritesDirectRawGameCubeImage() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-packaged-build-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string apploaderImagePath = Path.Combine(workingRootPath, "tooling", "apploader.img");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();
        string previousWitPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllTextAsync(apploaderImagePath, "apploader");
            Directory.SetCurrentDirectory(sourceRootPath);
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", null);

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
                [new PlatformBuildArtifact("cooked/scenes/rendering/cube_test.hasset", "scene-content-hash", "scene", "gamecube-default")],
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));

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
                outputRootPath,
                Path.Combine(workingRootPath, "tmp"),
                string.Empty,
                string.Empty,
                string.Empty,
                null,
                null,
                null,
                generatedCoreRootPath);

            RecordingProgressReporter progressReporter = new();
            RecordingDiagnosticReporter diagnosticReporter = new();
            FakeGameCubeNativeBuildExecutor nativeBuildExecutor = new();

            PlatformBuildReport report = await GameCubeBuildWorkspace.BuildPackagedAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None,
                nativeBuildExecutor,
                null,
                new GameCubeDiscSystemAreaOptions(apploaderImagePath, "PROJHE", "project"));

            Assert.True(report.Succeeded);
            Assert.True(File.Exists(Path.Combine(outputRootPath, "game.gcm")));
        } finally {
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", previousWitPath);
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures packaged builds fail clearly when no explicit GameCube apploader image path is configured.
    /// </summary>
    [Fact]
    public async Task BuildPackagedAsync_WhenApploaderPathEnvironmentVariableIsMissing_ThrowsInvalidOperationException() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-packaged-build-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();
        string previousWitPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
        string previousApploaderPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            Directory.SetCurrentDirectory(sourceRootPath);
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", Path.Combine(workingRootPath, "wit.exe"));
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH", null);
            await File.WriteAllTextAsync(Path.Combine(workingRootPath, "wit.exe"), "fake");

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
                [new PlatformBuildArtifact("cooked/scenes/rendering/cube_test.hasset", "scene-content-hash", "scene", "gamecube-default")],
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));

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
                outputRootPath,
                Path.Combine(workingRootPath, "tmp"),
                string.Empty,
                string.Empty,
                string.Empty,
                null,
                null,
                null,
                generatedCoreRootPath);

            RecordingProgressReporter progressReporter = new();
            RecordingDiagnosticReporter diagnosticReporter = new();
            FakeGameCubeNativeBuildExecutor nativeBuildExecutor = new();
            FakeGameCubeImagePackager imagePackager = new();

            InvalidOperationException exception = await Assert.ThrowsAsync<InvalidOperationException>(() =>
                GameCubeBuildWorkspace.BuildPackagedAsync(
                    request,
                    progressReporter,
                    diagnosticReporter,
                    CancellationToken.None,
                    nativeBuildExecutor,
                    imagePackager));

            Assert.Contains("HELENGINE_GAMECUBE_APPLOADER_PATH", exception.Message, StringComparison.Ordinal);
        } finally {
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", previousWitPath);
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH", previousApploaderPath);
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }
}
