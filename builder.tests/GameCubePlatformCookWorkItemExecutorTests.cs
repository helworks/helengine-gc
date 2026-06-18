using System.Text;
using helengine.baseplatform.Manifest;
using FilesAssetSerializer = helengine.files.AssetSerializer;
using FilesFontAssetBinarySerializer = helengine.files.FontAssetBinarySerializer;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the GameCube builder executes editor-emitted platform cook work items into staged runtime artifacts.
/// </summary>
public sealed class GameCubePlatformCookWorkItemExecutorTests {
    /// <summary>
    /// Ensures a texture work item imports one source PNG and writes a prepacked GameCube texture asset.
    /// </summary>
    [Fact]
    public void Execute_WhenUsingTextureWorkItem_WritesCookedTextureAssetIntoStagingRoot() {
        string workspacePath = Path.Combine(Path.GetTempPath(), "gamecube-platform-cook-work-item-tests", Guid.NewGuid().ToString("N"));
        string projectRootPath = Path.Combine(workspacePath, "project");
        string stagingRootPath = Path.Combine(workspacePath, "staging");
        string sourceTexturePath = Path.Combine(projectRootPath, "assets", "Images", "Menu", "logo.png");
        string outputRelativePath = "cooked/imported/logo-runtime.hasset";

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sourceTexturePath) ?? throw new InvalidOperationException("Texture source directory path could not be resolved."));
            File.WriteAllBytes(sourceTexturePath, CreateSinglePixelPngBytes());

            GameCubePlatformCookWorkItemExecutor executor = new GameCubePlatformCookWorkItemExecutor();
            executor.Execute(
                [
                    new PlatformCookWorkItem(
                        "gamecube:texture:cooked/imported/logo-runtime.hasset",
                        "Images/Menu/logo.png",
                        "texture",
                        "gamecube",
                        "runtime-texture",
                        outputRelativePath,
                        "runtime-texture:cooked/imported/logo-runtime.hasset",
                        "sha256:source",
                        "sha256:settings",
                        "{\"maxResolution\":0,\"colorFormat\":\"GxRgb5A3\",\"alphaPrecision\":\"A8\"}",
                        [new PlatformCookWorkItemMetadata("source-asset-id", "Images/Menu/logo.png")])
                ],
                projectRootPath,
                stagingRootPath);

            string outputPath = Path.Combine(stagingRootPath, "cooked", "imported", "logo-runtime.hasset");
            Assert.True(File.Exists(outputPath));

            using FileStream stream = new FileStream(outputPath, FileMode.Open, FileAccess.Read, FileShare.Read);
            TextureAsset cookedTexture = Assert.IsType<TextureAsset>(FilesAssetSerializer.Deserialize(stream));
            Assert.Equal(TextureAssetColorFormat.GxRgb5A3, cookedTexture.ColorFormat);
            Assert.Equal(32, cookedTexture.Colors.Length);
        } finally {
            if (Directory.Exists(workspacePath)) {
                Directory.Delete(workspacePath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures a font-atlas work item rewrites the serialized font atlas texture into a prepacked GameCube payload.
    /// </summary>
    [Fact]
    public void Execute_WhenUsingFontAtlasTextureWorkItem_WritesCookedFontAssetIntoStagingRoot() {
        string workspacePath = Path.Combine(Path.GetTempPath(), "gamecube-platform-cook-work-item-tests", Guid.NewGuid().ToString("N"));
        string projectRootPath = Path.Combine(workspacePath, "project");
        string stagingRootPath = Path.Combine(workspacePath, "staging");
        string sourceFontPath = Path.Combine(projectRootPath, "assets", "Fonts", "Body.hefont");
        string outputRelativePath = "cooked/fonts/Body.hefont";

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sourceFontPath) ?? throw new InvalidOperationException("Font source directory path could not be resolved."));
            WriteSourceFontAsset(sourceFontPath);

            GameCubePlatformCookWorkItemExecutor executor = new GameCubePlatformCookWorkItemExecutor();
            executor.Execute(
                [
                    new PlatformCookWorkItem(
                        "gamecube:font-atlas-texture:cooked/fonts/Body.hefont",
                        "Fonts/Body.hefont",
                        "font-atlas-texture",
                        "gamecube",
                        "runtime-font-atlas-texture",
                        outputRelativePath,
                        "runtime-font-atlas-texture:cooked/fonts/Body.hefont",
                        "sha256:source",
                        "sha256:settings",
                        "{\"maxResolution\":0,\"colorFormat\":\"GxRgb5A3\",\"alphaPrecision\":\"A8\"}",
                        [new PlatformCookWorkItemMetadata("source-asset-id", "Fonts/Body.hefont")])
                ],
                projectRootPath,
                stagingRootPath);

            string outputPath = Path.Combine(stagingRootPath, "cooked", "fonts", "Body.hefont");
            Assert.True(File.Exists(outputPath));

            using FileStream stream = new FileStream(outputPath, FileMode.Open, FileAccess.Read, FileShare.Read);
            TextureAsset cookedTexture = Assert.IsType<TextureAsset>(FilesAssetSerializer.Deserialize(stream));
            Assert.Equal(TextureAssetColorFormat.GxRgb5A3, cookedTexture.ColorFormat);
            Assert.Equal(TextureAssetAlphaPrecision.A8, cookedTexture.AlphaPrecision);
            Assert.Empty(cookedTexture.PaletteColors);
            Assert.Equal(32, cookedTexture.Colors.Length);
        } finally {
            if (Directory.Exists(workspacePath)) {
                Directory.Delete(workspacePath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures a font-atlas work item can load one generated source texture asset emitted with a generic .hasset extension.
    /// </summary>
    [Fact]
    public void Execute_WhenUsingGeneratedTextureAssetHasset_WritesCookedFontAssetIntoStagingRoot() {
        string workspacePath = Path.Combine(Path.GetTempPath(), "gamecube-platform-cook-work-item-tests", Guid.NewGuid().ToString("N"));
        string projectRootPath = Path.Combine(workspacePath, "project");
        string stagingRootPath = Path.Combine(workspacePath, "staging");
        string sourceFontPath = Path.Combine(projectRootPath, "assets", "generated", "editor", "fonts", "default-font-atlas.hasset");
        string outputRelativePath = "cooked/fonts/default-font-atlas.hasset";

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(sourceFontPath) ?? throw new InvalidOperationException("Font source directory path could not be resolved."));
            WriteSourceTextureAsset(sourceFontPath, "generated/editor/fonts/default-font-atlas.hasset");

            GameCubePlatformCookWorkItemExecutor executor = new GameCubePlatformCookWorkItemExecutor();
            executor.Execute(
                [
                    new PlatformCookWorkItem(
                        "gamecube:font-atlas-texture:cooked/fonts/default-font-atlas.hasset",
                        "generated/editor/fonts/default-font-atlas.hasset",
                        "font-atlas-texture",
                        "gamecube",
                        "runtime-font-atlas-texture",
                        outputRelativePath,
                        "runtime-font-atlas-texture:cooked/fonts/default-font-atlas.hasset",
                        "sha256:source",
                        "sha256:settings",
                        "{\"maxResolution\":0,\"colorFormat\":\"GxRgb5A3\",\"alphaPrecision\":\"A8\"}",
                        [new PlatformCookWorkItemMetadata("source-asset-id", "generated/editor/fonts/default-font-atlas.hasset")])
                ],
                projectRootPath,
                stagingRootPath);

            string outputPath = Path.Combine(stagingRootPath, "cooked", "fonts", "default-font-atlas.hasset");
            Assert.True(File.Exists(outputPath));

            using FileStream stream = new FileStream(outputPath, FileMode.Open, FileAccess.Read, FileShare.Read);
            TextureAsset cookedTexture = Assert.IsType<TextureAsset>(FilesAssetSerializer.Deserialize(stream));
            Assert.Equal(TextureAssetColorFormat.GxRgb5A3, cookedTexture.ColorFormat);
            Assert.Equal(TextureAssetAlphaPrecision.A8, cookedTexture.AlphaPrecision);
        } finally {
            if (Directory.Exists(workspacePath)) {
                Directory.Delete(workspacePath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Writes one minimal serialized source font asset for font-atlas cook testing.
    /// </summary>
    /// <param name="outputPath">Absolute font-asset output path.</param>
    static void WriteSourceFontAsset(string outputPath) {
        FontAsset fontAsset = new FontAsset(
            new FontInfo("Body", 16, 8),
            null,
            new Dictionary<char, FontChar>(),
            16,
            1,
            1) {
            SourceTextureAsset = new TextureAsset {
                Id = "Fonts/Body.hefont",
                Width = 1,
                Height = 1,
                ColorFormat = TextureAssetColorFormat.Rgba32,
                AlphaPrecision = TextureAssetAlphaPrecision.A8,
                Colors = [0xFF, 0x00, 0x00, 0xFF],
                PaletteColors = Array.Empty<byte>()
            }
        };

        using FileStream stream = new FileStream(outputPath, FileMode.Create, FileAccess.Write, FileShare.None);
        FilesFontAssetBinarySerializer.Serialize(stream, fontAsset);
    }

    /// <summary>
    /// Writes one minimal serialized source texture asset for generated font-atlas cook testing.
    /// </summary>
    /// <param name="outputPath">Absolute asset output path.</param>
    /// <param name="assetId">Stable texture asset id to serialize.</param>
    static void WriteSourceTextureAsset(string outputPath, string assetId) {
        TextureAsset textureAsset = new TextureAsset {
            Id = assetId,
            Width = 1,
            Height = 1,
            ColorFormat = TextureAssetColorFormat.Rgba32,
            AlphaPrecision = TextureAssetAlphaPrecision.A8,
            Colors = [0xFF, 0x00, 0x00, 0xFF],
            PaletteColors = Array.Empty<byte>()
        };

        using FileStream stream = new FileStream(outputPath, FileMode.Create, FileAccess.Write, FileShare.None);
        FilesAssetSerializer.Serialize(stream, textureAsset);
    }

    /// <summary>
    /// Creates one tiny valid PNG payload for source-texture work-item execution tests.
    /// </summary>
    /// <returns>Serialized one-pixel PNG file bytes.</returns>
    static byte[] CreateSinglePixelPngBytes() {
        return Convert.FromBase64String("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO8B9WQAAAAASUVORK5CYII=");
    }
}
