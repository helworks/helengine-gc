using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.gamecube.builder.tests.Builders;
using FilesAssetSerializer = helengine.files.AssetSerializer;

namespace helengine.gamecube.builder.tests;

/// <summary>Checks real texture and atlas cooking plus complete batch preflight before resetting a packaged staging tree.</summary>
public sealed class GameCubeNativeTextureWorkItemTests : IDisposable {
    /// <summary>Uses one fixed fixture tree beneath the explicit dotnet artifacts directory.</summary>
    readonly string Root = Path.Combine(AppContext.BaseDirectory, "gx-texture-work-items");
    /// <summary>Retains the caller's repository override while fixtures run from relocated artifacts.</summary>
    readonly string PreviousRepositoryRoot = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT");
    /// <summary>Gets the independent source serialized by the real files serializer.</summary>
    string Source => Path.Combine(Root, "source.hetex");
    /// <summary>Gets the existing packaged staging tree whose contents must survive preflight failure.</summary>
    string Stage => Path.Combine(Root, "working", "staged-content");

    /// <summary>Creates authored source data, generated-root metadata and an existing stage marker.</summary>
    public GameCubeNativeTextureWorkItemTests() {
        Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT", ResolveRepositoryRoot());
        Directory.CreateDirectory(Stage);
        Directory.CreateDirectory(Path.Combine(Root, "generated-core"));
        using TextureAsset source = GameCubeNativeTextureCookTests.Source(4, 2);
        File.WriteAllBytes(Source, FilesAssetSerializer.SerializeToBytes(source));
        File.WriteAllText(Path.Combine(Stage, "previous.txt"), "previous");
    }

    /// <summary>Cooks all native selections for both texture and atlas work-item kinds using real source, codec and serialization paths.</summary>
    [Fact]
    public void EveryNativeSelectionCooksTextureAndFontAtlasWorkItems() {
        byte[] original = File.ReadAllBytes(Source);
        List<PlatformCookWorkItem> items = [];
        for (int index = 0; index < GxNativeTextureFormatCatalog.Formats.Length; index++) {
            var format = GxNativeTextureFormatCatalog.Formats[index];
            items.Add(Item($"textures/{index}.hetex", format.Id));
            items.Add(Item($"fonts/{index}.hetex", format.Id, "font-atlas-texture"));
        }
        new GameCubePlatformCookWorkItemExecutor().Execute(items.ToArray(), Root, Stage);
        foreach (var item in items) {
            using TextureAsset cooked = Assert.IsType<TextureAsset>(FilesAssetSerializer.DeserializeFromBytes(File.ReadAllBytes(Path.Combine(Stage, item.OutputRelativePath))));
            Assert.Equal(TextureAssetColorFormat.GxNative, cooked.ColorFormat);
            Assert.Equal(item.OutputLogicalArtifactId, cooked.Id);
            Assert.NotEqual(0ul, cooked.RuntimeAssetId);
            Assert.Equal(4 * 2 * 4, TextureAssetPixelCodec.DecodeToRgba32(cooked).Length);
        }
        Assert.Equal(original, File.ReadAllBytes(Source));
    }

    /// <summary>Checks the unchanged atlas default produces the legacy tag4 payload without altering its source.</summary>
    [Fact]
    public void LegacyFontAtlasDefaultStillCooks() {
        var capability = Assert.Single(GameCubePlatformDefinitionFactory.Create().AssetCookCapabilities, item => item.SourceAssetKind == "font-atlas-texture");
        var item = new PlatformCookWorkItem("atlas", Source, "font-atlas-texture", "gamecube", "runtime-texture", "fonts/legacy.hetex", "fonts/legacy", "source", "settings", capability.DefaultSerializedPlatformSettings, []);
        new GameCubePlatformCookWorkItemExecutor().Execute([item], Root, Stage);
        using TextureAsset cooked = Assert.IsType<TextureAsset>(FilesAssetSerializer.DeserializeFromBytes(File.ReadAllBytes(Path.Combine(Stage, item.OutputRelativePath))));
        Assert.Equal(TextureAssetColorFormat.GxRgb5A3, cooked.ColorFormat);
        Assert.Equal(TextureAssetAlphaPrecision.A8, cooked.AlphaPrecision);
    }

    /// <summary>Rejects invalid later output paths before an earlier valid item can publish.</summary>
    [Theory]
    [InlineData("../outside.hetex")]
    [InlineData("C:/outside.hetex")]
    [InlineData("bad.hetex:stream")]
    [InlineData("NUL.hetex")]
    [InlineData("bad?.hetex")]
    public void UnsafePathsPreservePreviousOutputs(string path) {
        Assert.Throws<InvalidOperationException>(() => new GameCubePlatformCookWorkItemExecutor().Execute([Item("first.hetex"), Item(path)], Root, Stage));
        Assert.False(File.Exists(Path.Combine(Stage, "first.hetex")));
        Assert.Equal("previous", File.ReadAllText(Path.Combine(Stage, "previous.txt")));
    }

    /// <summary>Rejects exact, parent-child, existing file and physical files-prefix aliases before publishing.</summary>
    [Theory]
    [InlineData("same.hetex", "SAME.hetex")]
    [InlineData("texture", "texture/child.hetex")]
    [InlineData("first.hetex", "previous.txt/child.hetex")]
    [InlineData("image.hetex", "files/image.hetex")]
    public void NamespaceConflictsPreservePreviousOutputs(string first, string second) {
        Assert.Throws<InvalidOperationException>(() => new GameCubePlatformCookWorkItemExecutor().Execute([Item(first), Item(second)], Root, Stage));
        Assert.False(File.Exists(Path.Combine(Stage, first)));
    }

    /// <summary>Checks a later source decode failure cannot publish an earlier prepared payload.</summary>
    [Fact]
    public void LaterDecodeFailurePublishesNothing() {
        string bad = Path.Combine(Root, "bad.hetex");
        File.WriteAllBytes(bad, [1, 2, 3]);
        Assert.ThrowsAny<Exception>(() => new GameCubePlatformCookWorkItemExecutor().Execute([Item("first.hetex"), Item("second.hetex", source: bad)], Root, Stage));
        Assert.False(File.Exists(Path.Combine(Stage, "first.hetex")));
        Assert.Equal("previous", File.ReadAllText(Path.Combine(Stage, "previous.txt")));
    }

    /// <summary>Rejects unknown cook contracts before any source or prior output is changed.</summary>
    [Theory]
    [InlineData("material", "gamecube", "runtime-texture")]
    [InlineData("texture", "ps2", "runtime-texture")]
    [InlineData("texture", "gamecube", "runtime-material")]
    public void UnsupportedContractPublishesNothing(string kind, string platform, string artifact) {
        var item = new PlatformCookWorkItem("bad", Source, kind, platform, artifact, "first.hetex", "first", "source", "settings",
            GameCubeNativeTextureCookTests.Settings("Gx.RGBA8", TextureAssetAlphaPrecision.A8).Serialize(), []);
        Assert.Throws<InvalidOperationException>(() => new GameCubePlatformCookWorkItemExecutor().Execute([item], Root, Stage));
        Assert.False(File.Exists(Path.Combine(Stage, "first.hetex")));
    }

    /// <summary>Checks manifest collisions fail before the packaged workspace resets its preceding staging tree.</summary>
    [Fact]
    public async Task ManifestCollisionPreservesStageBeforeNativeBuild() {
        var artifact = new PlatformBuildArtifact("files/image.hetex", "image", "hash", "texture", "gamecube-default");
        var exception = await Assert.ThrowsAsync<InvalidOperationException>(() => new GameCubePlatformAssetBuilder().BuildAsync(
            Request([Item("image.hetex")], [artifact]), new RecordingProgressReporter(), new RecordingDiagnosticReporter(), CancellationToken.None));
        Assert.Contains("conflicts", exception.Message, StringComparison.Ordinal);
        Assert.Equal("previous", File.ReadAllText(Path.Combine(Stage, "previous.txt")));
    }

    /// <summary>Checks complete source cooking happens before the old packaged stage is reset.</summary>
    [Fact]
    public async Task InvalidCookPreservesPackagedStageBeforeNativeBuild() {
        var exception = await Assert.ThrowsAsync<InvalidOperationException>(() => new GameCubePlatformAssetBuilder().BuildAsync(
            Request([Item("../outside.hetex")], []), new RecordingProgressReporter(), new RecordingDiagnosticReporter(), CancellationToken.None));
        Assert.Contains("Unsafe GameCube cooked path", exception.Message, StringComparison.Ordinal);
        Assert.Equal("previous", File.ReadAllText(Path.Combine(Stage, "previous.txt")));
    }

    /// <summary>Checks an absent manifest source is diagnosed before resetting stage or publishing prepared textures.</summary>
    [Fact]
    public async Task MissingManifestSourcePreservesStageBeforeNativeBuild() {
        var artifact = new PlatformBuildArtifact("missing-native-format-source.hetex", "missing", "hash", "texture", "gamecube-default");
        var report = await new GameCubePlatformAssetBuilder().BuildAsync(Request([Item("first.hetex")], [artifact]),
            new RecordingProgressReporter(), new RecordingDiagnosticReporter(), CancellationToken.None);
        Assert.False(report.Succeeded);
        Assert.Equal("previous", File.ReadAllText(Path.Combine(Stage, "previous.txt")));
        Assert.False(File.Exists(Path.Combine(Stage, "first.hetex")));
    }

    /// <summary>Builds a cook item using the native catalog's exact preferred alpha policy and selected quantizer.</summary>
    PlatformCookWorkItem Item(string output, string formatId = "Gx.RGBA8", string kind = "texture", string source = null) {
        GxNativeTextureFormatCatalog.TryGetFormat(formatId, out var format);
        return new PlatformCookWorkItem(output, source ?? Source, kind, "gamecube", "runtime-texture", output, output, "source", "settings",
            GameCubeNativeTextureCookTests.Settings(formatId, format.DefaultAlphaPrecision).Serialize(), []);
    }

    /// <summary>Creates a packaged build request whose failures occur before the real native executor can run.</summary>
    PlatformBuildRequest Request(PlatformCookWorkItem[] items, PlatformBuildArtifact[] artifacts) {
        var manifest = new PlatformBuildManifest(3, "project", "1.0.0", "1.0.0", "gamecube", "1.0.0", "",
            [], [], artifacts, [], [], new PlatformContainerWritePlan("gamecube-disc-layout", []), items, PlatformBuildRuntimeFeatureManifest.Empty);
        return new PlatformBuildRequest(manifest,
            [new PlatformBuildTargetVariant("gamecube-default", "gamecube", "gamecube", "gamecube-default")],
            [new PlatformCookProfile("gamecube-default", "GameCube", new PlatformCookProfileCapabilities("gamecube", "raw", "pcm", "scene", PlatformSerializationEndianness.BigEndian))],
            Path.Combine(Root, "output"), Path.Combine(Root, "working"), "gamecube-default", "gx", "default", null, null, null, Path.Combine(Root, "generated-core"));
    }

    /// <summary>Removes only this test class's contained fixed fixture directory, preserving unrelated outputs.</summary>
    public void Dispose() {
        Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT", PreviousRepositoryRoot);
        string expected = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "gx-texture-work-items"));
        if (!Path.GetFullPath(Root).Equals(expected, StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("Fixture cleanup escaped the test artifacts root.");
        if (Directory.Exists(Root)) Directory.Delete(Root, true);
    }

    /// <summary>Finds the sibling checkout from relocated test artifacts without relying on the host's current directory.</summary>
    static string ResolveRepositoryRoot() {
        string current = AppContext.BaseDirectory;
        while (!string.IsNullOrEmpty(current)) {
            string candidate = Path.Combine(current, "helengine-gc");
            if (File.Exists(Path.Combine(candidate, "builder", "helengine.gamecube.builder.csproj"))) return candidate;
            current = Path.GetDirectoryName(Path.TrimEndingDirectorySeparator(current));
        }
        throw new DirectoryNotFoundException("The GameCube fixture could not locate its sibling checkout.");
    }
}
