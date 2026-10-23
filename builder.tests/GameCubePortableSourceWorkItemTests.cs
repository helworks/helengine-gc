using helengine.baseplatform.Manifest;
using helengine.editor;

namespace helengine.gamecube.builder.tests;

/// <summary>Exercises authored PNG and atlas work items through real cooking and the selected runtime reader.</summary>
public sealed class GameCubePortableSourceWorkItemTests {
    /// <summary>Preserves pixels and logical identity for direct PNG sources and their persisted import sidecars on every host.</summary>
    [Theory]
    [InlineData("texture", false)]
    [InlineData("texture", true)]
    [InlineData("font-atlas-texture", false)]
    [InlineData("font-atlas-texture", true)]
    public void Execute_PngSourcePreservesPixelsIdentityAndTarget32Envelope(string sourceKind, bool useSidecar) {
        string root = Path.Combine(Path.GetTempPath(), "gc-portable-source");
        string source = Path.Combine(root, "assets", "source.png");
        string staging = Path.Combine(root, "staging");
        Directory.CreateDirectory(Path.GetDirectoryName(source));
        File.WriteAllBytes(source, Convert.FromBase64String("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR4nGP4z8DwHwAFAAH/iZk9HQAAAABJRU5ErkJggg=="));
        string sourcePath = source;
        if (useSidecar) {
            sourcePath += ".hasset";
            File.WriteAllText(sourcePath, "{\"importerId\":\"gdi\"}");
        }
        string logicalId = sourceKind == "texture" ? "texture:portable" : "font:portable#atlas";
        PlatformCookWorkItem item = new PlatformCookWorkItem(
            "portable", sourcePath, sourceKind, "gamecube", "runtime-texture", "cooked/source.hetex", logicalId,
            "source", "settings", "{\"maxResolution\":0,\"colorFormat\":\"Gx.RGBA8\",\"alphaPrecision\":\"A8\"}",
            [new PlatformCookWorkItemMetadata("source-asset-id", "source.png")]);
        NumericPrecisionPolicy targetPolicy = new NumericPrecisionPolicy(32, 32);
        try {
            new GameCubePlatformCookWorkItemExecutor().Execute([item], root, staging, targetPolicy);
            using FileStream stream = File.OpenRead(Path.Combine(staging, "cooked", "source.hetex"));
            using TextureAsset cooked = Assert.IsType<TextureAsset>(PackagedAssetBinarySerializer.Deserialize(stream, targetPolicy));
            Assert.Equal(logicalId, cooked.Id);
            Assert.Equal(RuntimeAssetIdGenerator.Generate(logicalId), cooked.RuntimeAssetId);
            Assert.Equal((ushort)1, cooked.Width);
            Assert.Equal((ushort)1, cooked.Height);
            Assert.Equal(new byte[] { 255, 0, 0, 255 }, GxNativeTextureCodec.Decode(cooked));
        } finally {
            Directory.Delete(root, recursive: true);
        }
    }
}
