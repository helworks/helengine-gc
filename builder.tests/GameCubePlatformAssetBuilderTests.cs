using helengine;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Results;
using helengine.baseplatform.Targets;
using helengine.gamecube.builder.tests.Builders;
using helengine.files;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the GameCube platform builder metadata, staged payload flow, and packaged-disc routing behavior.
/// </summary>
public sealed class GameCubePlatformAssetBuilderTests {
    /// <summary>
    /// Ensures the builder exposes the expected public GameCube metadata and platform definition.
    /// </summary>
    [Fact]
    public void DescriptorAndDefinition_ExposeExpectedGameCubeMetadata() {
        GameCubePlatformAssetBuilder builder = new();

        Assert.Equal("helengine.gamecube.builder", builder.Descriptor.BuilderId);
        Assert.Equal("gamecube", builder.Descriptor.TargetPlatformId);
        Assert.Contains("gamecube", builder.Descriptor.SupportedRuntimeBackendIds);
        Assert.Equal("gamecube", builder.Definition.PlatformId);
        Assert.Contains(builder.Definition.BuildProfiles, profile => profile.ProfileId == "gamecube-default");
        Assert.Contains(builder.Definition.GraphicsProfiles, profile => profile.ProfileId == "gx");
        Assert.Equal(RuntimeMaterialResolutionMode.CookedPlatformOwned, builder.Definition.RuntimeGenerationContract.MaterialResolutionMode);
        Assert.Contains(builder.Definition.MaterialSchemas, schema => schema.SchemaId == GameCubeMaterialSchemaIds.StandardTexturedSchemaId);
    }

    /// <summary>
    /// Ensures the GameCube builder cooks platform-owned material payloads without shader references.
    /// </summary>
    [Fact]
    public void CookMaterial_WhenUsingGameCubeStandardSchema_ReturnsPlatformMaterialAssetWithoutShaderReferences() {
        GameCubePlatformAssetBuilder builder = new();

        PlatformMaterialCookResult result = builder.CookMaterial(new PlatformMaterialCookRequest(
            "Materials/rendering/test/Cube00",
            "Materials/rendering/test/Cube00.helmat",
            "gamecube",
            "gamecube-default",
            "gx",
            GameCubeMaterialSchemaIds.StandardTexturedSchemaId,
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
                [GameCubeMaterialSchemaIds.TextureRelativePathFieldId] = "cooked/engine/textures/test.hasset",
                [GameCubeMaterialSchemaIds.DoubleSidedFieldId] = "true",
                [GameCubeMaterialSchemaIds.VertexColorModeFieldId] = "multiply",
                [GameCubeMaterialSchemaIds.BaseColorFieldId] = "#FF8040FF",
                [GameCubeMaterialSchemaIds.LightingModeFieldId] = "lit"
            }));

        Assert.Empty(result.ReferencedShaderAssetIds);

        PlatformMaterialAsset cookedAsset = Assert.IsType<PlatformMaterialAsset>(AssetSerializer.DeserializeFromBytes(result.CookedMaterialBytes));
        Assert.Equal("gx", cookedAsset.RendererFamilyId);
        Assert.Equal("cooked/engine/textures/test.hasset", cookedAsset.TextureRelativePath);
        Assert.True(cookedAsset.DoubleSided);
        Assert.True(cookedAsset.UseVertexColor);
        Assert.True(cookedAsset.Lit);
        Assert.Equal((byte)255, cookedAsset.BaseColorR);
        Assert.Equal((byte)128, cookedAsset.BaseColorG);
        Assert.Equal((byte)64, cookedAsset.BaseColorB);
        Assert.Equal((byte)255, cookedAsset.BaseColorA);
    }

    /// <summary>
    /// Ensures the default public builder entrypoint continues copying staged payloads into the output root for non-packaged requests.
    /// </summary>
    [Fact]
    public async Task BuildAsync_WhenManifestUsesDefaultFlow_CopiesPayloadsIntoOutputRoot() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string sceneSourcePath = Path.Combine(sourceRootPath, "scenes", "startup.helen");
        string textureSourcePath = Path.Combine(sourceRootPath, "assets", "textures", "checker.png");
        string previousDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene source directory path could not be resolved."));
            Directory.CreateDirectory(Path.GetDirectoryName(textureSourcePath) ?? throw new InvalidOperationException("Texture source directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene payload");
            await File.WriteAllTextAsync(textureSourcePath, "texture payload");
            Directory.SetCurrentDirectory(sourceRootPath);

            PlatformBuildManifest manifest = new(
                1,
                "project",
                "1.0.0",
                "1.0.0",
                "gamecube",
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
                outputRootPath,
                Path.Combine(workingRootPath, "tmp"));

            RecordingProgressReporter progressReporter = new();
            RecordingDiagnosticReporter diagnosticReporter = new();
            GameCubePlatformAssetBuilder builder = new();

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            string diagnosticSummary = string.Join(
                Environment.NewLine,
                diagnosticReporter.Diagnostics.Select(diagnostic => diagnostic.Code + ": " + diagnostic.Message + " [" + diagnostic.SourceIdentity + "]"));
            Assert.True(report.Succeeded, diagnosticSummary);
            Assert.Empty(diagnosticReporter.Diagnostics);
            Assert.Equal(2, progressReporter.Updates.Count);
            Assert.True(File.Exists(Path.Combine(outputRootPath, "scenes", "startup.helen")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "assets", "textures", "checker.png")));
            Assert.True(File.Exists(Path.Combine(workingRootPath, "tmp", "gamecube-build-manifest.json")));
        } finally {
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures the public builder entrypoint emits packaged-disc artifacts when the manifest selects the GameCube packaged-disc runtime specialization.
    /// </summary>
    [Fact]
    public async Task BuildAsync_WhenManifestSelectsPackagedDiscFlow_WritesDiscRootAndImageArtifact() {
        string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-platform-builder-tests", Guid.NewGuid().ToString("N"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sourceRootPath = Path.Combine(workingRootPath, "project");
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string apploaderImagePath = Path.Combine(workingRootPath, "tooling", "apploader.img");
        string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string previousDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
            Directory.CreateDirectory(generatedCoreRootPath);
            Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
            await File.WriteAllTextAsync(sceneSourcePath, "scene");
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
            GameCubePlatformAssetBuilder builder = new(
                new FakeGameCubeNativeBuildExecutor(),
                new FakeGameCubeImagePackager(),
                new GameCubeDiscSystemAreaOptions(apploaderImagePath, "PROJHE", "project"));

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            Assert.True(report.Succeeded);
            Assert.Empty(diagnosticReporter.Diagnostics);
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "main.dol")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "boot.bin")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "bi2.bin")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "sys", "apploader.img")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
            Assert.True(File.Exists(Path.Combine(outputRootPath, "game.gcm")));
        } finally {
            Directory.SetCurrentDirectory(previousDirectory);
            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }
        }
    }
}
