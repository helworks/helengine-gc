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
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "DemoDiscMainMenu.hasset");
        string fontSourcePath = Path.Combine(sourceRootPath, "cooked", "fonts", "DemoDiscBody.hefont");
        string defaultFontSourcePath = Path.Combine(sourceRootPath, "cooked", "fonts", "default.hefont");
        string previousDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(fontSourcePath) ?? throw new InvalidOperationException("Font directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(defaultFontSourcePath) ?? throw new InvalidOperationException("Default font directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllTextAsync(fontSourcePath, "font");
            await File.WriteAllTextAsync(defaultFontSourcePath, "default-font");
            await File.WriteAllTextAsync(apploaderImagePath, "apploader");
            Directory.SetCurrentDirectory(sourceRootPath);

            PlatformBuildManifest manifest = new(
                1,
                "project",
                "1.0.0",
                "1.0.0",
                "gamecube",
                "1.0.0",
                "Scenes/DemoDiscMainMenu.helen",
                [
                    new PlatformBuildScene(
                        "Scenes/DemoDiscMainMenu.helen",
                        "Demo Disc Main Menu",
                        "Scenes/DemoDiscMainMenu.helen",
                        [],
                        [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/DemoDiscMainMenu.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                [
                    new PlatformBuildArtifact("cooked/scenes/DemoDiscMainMenu.hasset", "scene-content-hash", "scene", "gamecube-default"),
                    new PlatformBuildArtifact("cooked/fonts/DemoDiscBody.hefont", "font-content-hash", "font", "gamecube-default"),
                    new PlatformBuildArtifact("cooked/fonts/default.hefont", "default-font-content-hash", "font", "gamecube-default")
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
            Assert.True(File.Exists(Path.Combine(outputRootPath, "gamecube-build-phase.txt")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "main.dol")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "boot.bin")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "bi2.bin")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "apploader.img")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "scenes", "DemoDiscMainMenu.hasset")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "fonts", "DemoDiscBody.hefont")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "fonts", "default.hefont")));
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
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "textured_cube_grid.hasset");
        string cubeMaterialPath = Path.Combine(sourceRootPath, "cooked", "materials", "rendering", "textured_cube_grid", "Cube00.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();
        string previousWitPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(cubeMaterialPath) ?? throw new InvalidOperationException("Material directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllTextAsync(cubeMaterialPath, "cube-material");
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
                "scenes/rendering/textured_cube_grid.helen",
                [
                    new PlatformBuildScene(
                        "scenes/rendering/textured_cube_grid.helen",
                        "Textured Cubes",
                        "Scenes/rendering/textured_cube_grid.helen",
                        [],
                        [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/rendering/textured_cube_grid.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                [
                    new PlatformBuildArtifact("cooked/scenes/rendering/textured_cube_grid.hasset", "scene-content-hash", "scene", "gamecube-default"),
                    new PlatformBuildArtifact("cooked/materials/rendering/textured_cube_grid/Cube00.hasset", "material-content-hash", "material", "gamecube-default")
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

            PlatformBuildReport report = await GameCubeBuildWorkspace.BuildPackagedAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None,
                nativeBuildExecutor,
                null,
                new GameCubeDiscSystemAreaOptions(apploaderImagePath, "PROJHE", "project"));

            Assert.True(report.Succeeded);
            Assert.True(File.Exists(Path.Combine(outputRootPath, "gamecube-build-phase.txt")));
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
    /// Ensures packaged builds fall back to the staged native apploader image when no explicit GameCube apploader environment override is configured.
    /// </summary>
    [Fact]
    public async Task BuildPackagedAsync_WhenApploaderPathEnvironmentVariableIsMissing_UsesStagedNativeApploaderImage() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-packaged-build-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "textured_cube_grid.hasset");
        string cubeMaterialPath = Path.Combine(sourceRootPath, "cooked", "materials", "rendering", "textured_cube_grid", "Cube00.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();
        string previousWitPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
        string previousApploaderPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(cubeMaterialPath) ?? throw new InvalidOperationException("Material directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllTextAsync(cubeMaterialPath, "cube-material");
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
                "scenes/rendering/textured_cube_grid.helen",
                [
                    new PlatformBuildScene(
                        "scenes/rendering/textured_cube_grid.helen",
                        "Textured Cubes",
                        "Scenes/rendering/textured_cube_grid.helen",
                        [],
                        [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/rendering/textured_cube_grid.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                [
                    new PlatformBuildArtifact("cooked/scenes/rendering/textured_cube_grid.hasset", "scene-content-hash", "scene", "gamecube-default"),
                    new PlatformBuildArtifact("cooked/materials/rendering/textured_cube_grid/Cube00.hasset", "material-content-hash", "material", "gamecube-default")
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
                imagePackager);

            Assert.True(report.Succeeded);
            Assert.True(File.Exists(Path.Combine(outputRootPath, "gamecube-build-phase.txt")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "apploader.img")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "native", "apploader.img")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "game.gcm")));
        } finally {
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", previousWitPath);
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH", previousApploaderPath);
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures packaged builds fail clearly when neither an explicit GameCube apploader override nor the staged native apploader artifact exists.
    /// </summary>
    [Fact]
    public async Task BuildPackagedAsync_WhenApploaderPathEnvironmentVariableIsMissingAndNativeBuildDoesNotEmitApploader_ThrowsInvalidOperationException() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-packaged-build-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "textured_cube_grid.hasset");
        string cubeMaterialPath = Path.Combine(sourceRootPath, "cooked", "materials", "rendering", "textured_cube_grid", "Cube00.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();
        string previousWitPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
        string previousApploaderPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(cubeMaterialPath) ?? throw new InvalidOperationException("Material directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllTextAsync(cubeMaterialPath, "cube-material");
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
                "scenes/rendering/textured_cube_grid.helen",
                [
                    new PlatformBuildScene(
                        "scenes/rendering/textured_cube_grid.helen",
                        "Textured Cubes",
                        "Scenes/rendering/textured_cube_grid.helen",
                        [],
                        [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/rendering/textured_cube_grid.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                [
                    new PlatformBuildArtifact("cooked/scenes/rendering/textured_cube_grid.hasset", "scene-content-hash", "scene", "gamecube-default"),
                    new PlatformBuildArtifact("cooked/materials/rendering/textured_cube_grid/Cube00.hasset", "material-content-hash", "material", "gamecube-default")
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
            FakeGameCubeNativeBuildExecutor nativeBuildExecutor = new(writeApploaderImage: false);
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
            Assert.Contains("native", exception.Message, StringComparison.OrdinalIgnoreCase);
        } finally {
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", previousWitPath);
            Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH", previousApploaderPath);
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures packaged builds execute builder-owned GameCube texture work items and stage their outputs into the extracted disc layout.
    /// </summary>
    [Fact]
    public async Task BuildPackagedAsync_WhenManifestContainsPlatformCookWorkItem_StagesCookedOutputIntoDiscFiles() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-packaged-build-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string apploaderImagePath = Path.Combine(workingRootPath, "tooling", "apploader.img");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "DemoDiscMainMenu.hasset");
        string sourceTexturePath = Path.Combine(sourceRootPath, "assets", "Images", "Menu", "logo.png");
        string previousDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(sourceTexturePath) ?? throw new InvalidOperationException("Texture source directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
            await File.WriteAllBytesAsync(sourceTexturePath, CreateSinglePixelPngBytes());
            await File.WriteAllTextAsync(apploaderImagePath, "apploader");
            Directory.SetCurrentDirectory(sourceRootPath);

            PlatformBuildManifest manifest = new(
                1,
                "project",
                "1.0.0",
                "1.0.0",
                "gamecube",
                "1.0.0",
                "Scenes/DemoDiscMainMenu.helen",
                [
                    new PlatformBuildScene(
                        "Scenes/DemoDiscMainMenu.helen",
                        "Demo Disc Main Menu",
                        "Scenes/DemoDiscMainMenu.helen",
                        [],
                        [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/DemoDiscMainMenu.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                [new PlatformBuildArtifact("cooked/scenes/DemoDiscMainMenu.hasset", "scene-content-hash", "scene", "gamecube-default")],
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()),
                [
                    new PlatformCookWorkItem(
                        "gamecube:texture:cooked/imported/logo-runtime.hasset",
                        "Images/Menu/logo.png",
                        "texture",
                        "gamecube",
                        "runtime-texture",
                        "cooked/imported/logo-runtime.hasset",
                        "runtime-texture:cooked/imported/logo-runtime.hasset",
                        "sha256:source",
                        "sha256:settings",
                        "{\"maxResolution\":0,\"colorFormat\":\"GxRgb5A3\",\"alphaPrecision\":\"A8\"}",
                        [new PlatformCookWorkItemMetadata("source-asset-id", "Images/Menu/logo.png")])
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
            string cookedTexturePath = Path.Combine(outputRootPath, "disc", "files", "cooked", "imported", "logo-runtime.hasset");
            Assert.True(File.Exists(cookedTexturePath));

            using FileStream stream = new FileStream(cookedTexturePath, FileMode.Open, FileAccess.Read, FileShare.Read);
            TextureAsset cookedTexture = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.Deserialize(stream));
            Assert.Equal(TextureAssetColorFormat.GxRgb5A3, cookedTexture.ColorFormat);
        } finally {
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Creates one tiny valid PNG payload for builder-owned texture staging tests.
    /// </summary>
    /// <returns>Serialized one-pixel PNG file bytes.</returns>
    static byte[] CreateSinglePixelPngBytes() {
        return Convert.FromBase64String("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO8B9WQAAAAASUVORK5CYII=");
    }
}
