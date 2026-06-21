using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using helengine;
using helengine.baseplatform.Manifest;
using helengine.editor;
using FilesAssetSerializer = helengine.files.AssetSerializer;
using FilesEngineBinaryHeaderSerializer = helengine.files.EngineBinaryHeaderSerializer;

namespace helengine.gamecube.builder;

/// <summary>
/// Executes generic platform-owned cook work items for the GameCube builder.
/// </summary>
public sealed class GameCubePlatformCookWorkItemExecutor {
    /// <summary>
    /// Shared GameCube texture cooker used by all GameCube texture work items.
    /// </summary>
    readonly GameCubeTextureCooker TextureCooker;

    /// <summary>
    /// Initializes one GameCube platform cook work-item executor.
    /// </summary>
    public GameCubePlatformCookWorkItemExecutor() {
        TextureCooker = new GameCubeTextureCooker();
    }

    /// <summary>
    /// Executes the supplied GameCube platform cook work items into the staging root.
    /// </summary>
    /// <param name="workItems">Work items emitted by the editor build graph.</param>
    /// <param name="projectRootPath">Project root used to resolve source asset paths.</param>
    /// <param name="stagingRootPath">Staging root that receives cooked platform-owned outputs.</param>
    public void Execute(PlatformCookWorkItem[] workItems, string projectRootPath, string stagingRootPath) {
        if (workItems == null) {
            throw new ArgumentNullException(nameof(workItems));
        } else if (string.IsNullOrWhiteSpace(projectRootPath)) {
            throw new ArgumentException("Project root path must be provided.", nameof(projectRootPath));
        } else if (string.IsNullOrWhiteSpace(stagingRootPath)) {
            throw new ArgumentException("Staging root path must be provided.", nameof(stagingRootPath));
        }

        for (int index = 0; index < workItems.Length; index++) {
            PlatformCookWorkItem workItem = workItems[index] ?? throw new InvalidOperationException("Platform cook work items must not contain null entries.");
            if (string.Equals(workItem.SourceAssetKind, "texture", StringComparison.OrdinalIgnoreCase)) {
                ExecuteTextureWorkItem(workItem, projectRootPath, stagingRootPath);
            } else {
                throw new InvalidOperationException($"GameCube builder does not support platform cook work item kind '{workItem.SourceAssetKind}'.");
            }
        }
    }

    /// <summary>
    /// Executes one builder-owned texture work item into the staging root.
    /// </summary>
    /// <param name="workItem">Texture work item emitted by the editor build graph.</param>
    /// <param name="projectRootPath">Project root used to resolve the source texture asset.</param>
    /// <param name="stagingRootPath">Staging root that receives the cooked texture asset.</param>
    void ExecuteTextureWorkItem(PlatformCookWorkItem workItem, string projectRootPath, string stagingRootPath) {
        TextureAsset sourceTexture = LoadSourceTexture(workItem, projectRootPath);
        GameCubeTextureCookSettings settings = GameCubeTextureCookSettings.Parse(workItem.SerializedPlatformSettings);
        TextureAsset cookedTexture = TextureCooker.CookTexture(sourceTexture, settings);
        WriteTextureAsset(Path.Combine(stagingRootPath, NormalizePath(workItem.OutputRelativePath)), cookedTexture);
    }

    /// <summary>
    /// Loads one source texture from the authored project asset path emitted by the editor work item.
    /// </summary>
    /// <param name="workItem">Texture work item emitted by the editor build graph.</param>
    /// <param name="projectRootPath">Project root used to resolve the source texture asset path.</param>
    /// <returns>Imported RGBA32 texture asset ready for GameCube-native cooking.</returns>
    static TextureAsset LoadSourceTexture(PlatformCookWorkItem workItem, string projectRootPath) {
        if (workItem == null) {
            throw new ArgumentNullException(nameof(workItem));
        } else if (string.IsNullOrWhiteSpace(projectRootPath)) {
            throw new ArgumentException("Project root path must be provided.", nameof(projectRootPath));
        }

        string sourcePath = ResolveSourceAssetPath(projectRootPath, workItem.SourceAssetPath);
        if (TryReadTextureAsset(sourcePath, out TextureAsset textureAsset)) {
            return textureAsset;
        }

        using Bitmap sourceBitmap = new Bitmap(sourcePath);
        int width = sourceBitmap.Width;
        int height = sourceBitmap.Height;
        if (width < 1 || height < 1 || width > ushort.MaxValue || height > ushort.MaxValue) {
            throw new InvalidOperationException($"Texture source '{workItem.SourceAssetPath}' produced unsupported dimensions '{width}x{height}'.");
        }

        Rectangle bounds = new Rectangle(0, 0, width, height);
        using Bitmap bitmap = sourceBitmap.Clone(bounds, PixelFormat.Format32bppArgb);
        BitmapData bitmapData = bitmap.LockBits(bounds, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        try {
            byte[] colors = ReadBitmapColors(bitmapData, width, height);
            return new TextureAsset {
                Id = ResolveMetadataValue(workItem, "source-asset-id"),
                Width = (ushort)width,
                Height = (ushort)height,
                Colors = colors,
                PaletteColors = Array.Empty<byte>(),
                ColorFormat = TextureAssetColorFormat.Rgba32,
                AlphaPrecision = TextureAssetAlphaPrecision.A8
            };
        } finally {
            bitmap.UnlockBits(bitmapData);
        }
    }

    /// <summary>
    /// Reads the logical RGBA32 colors from one 32-bit ARGB bitmap lock.
    /// </summary>
    /// <param name="bitmapData">Locked bitmap payload.</param>
    /// <param name="width">Bitmap width in pixels.</param>
    /// <param name="height">Bitmap height in pixels.</param>
    /// <returns>Tightly packed RGBA32 bytes.</returns>
    static byte[] ReadBitmapColors(BitmapData bitmapData, int width, int height) {
        if (bitmapData == null) {
            throw new ArgumentNullException(nameof(bitmapData));
        } else if (width < 1) {
            throw new ArgumentOutOfRangeException(nameof(width), "Bitmap width must be positive.");
        } else if (height < 1) {
            throw new ArgumentOutOfRangeException(nameof(height), "Bitmap height must be positive.");
        }

        const int bytesPerPixel = 4;
        int rowByteCount = width * bytesPerPixel;
        byte[] colors = new byte[width * height * bytesPerPixel];
        byte[] sourceRow = new byte[rowByteCount];
        for (int rowIndex = 0; rowIndex < height; rowIndex++) {
            IntPtr rowPointer = IntPtr.Add(bitmapData.Scan0, rowIndex * bitmapData.Stride);
            Marshal.Copy(rowPointer, sourceRow, 0, rowByteCount);

            int destinationRowOffset = rowIndex * rowByteCount;
            for (int columnIndex = 0; columnIndex < width; columnIndex++) {
                int sourceOffset = columnIndex * bytesPerPixel;
                int destinationOffset = destinationRowOffset + sourceOffset;
                colors[destinationOffset + 0] = sourceRow[sourceOffset + 2];
                colors[destinationOffset + 1] = sourceRow[sourceOffset + 1];
                colors[destinationOffset + 2] = sourceRow[sourceOffset + 0];
                colors[destinationOffset + 3] = sourceRow[sourceOffset + 3];
            }
        }

        return colors;
    }

    /// <summary>
    /// Attempts to read one serialized texture asset from an authored HELE file path.
    /// </summary>
    /// <param name="sourcePath">Absolute source path to inspect.</param>
    /// <param name="textureAsset">Deserialized texture asset when the file stores one.</param>
    /// <returns>True when the source path stored a texture asset; otherwise false.</returns>
    static bool TryReadTextureAsset(string sourcePath, out TextureAsset textureAsset) {
        textureAsset = null;
        if (string.IsNullOrWhiteSpace(sourcePath)) {
            throw new ArgumentException("Source path must be provided.", nameof(sourcePath));
        }

        string extension = Path.GetExtension(sourcePath);
        if (!string.Equals(extension, ".hasset", StringComparison.OrdinalIgnoreCase)
            && !string.Equals(extension, ".hetex", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        using FileStream stream = new FileStream(sourcePath, FileMode.Open, FileAccess.Read, FileShare.Read);
        EngineBinaryHeader header;
        try {
            header = FilesEngineBinaryHeaderSerializer.Read(stream);
        } catch (InvalidOperationException) {
            return false;
        }

        if (header.RecordKind != (ushort)EditorBinaryRecordKind.Asset) {
            return false;
        }

        stream.Position = 0;
        textureAsset = FilesAssetSerializer.Deserialize(stream) as TextureAsset;
        return textureAsset != null;
    }

    /// <summary>
    /// Writes one cooked texture asset to the supplied staging path.
    /// </summary>
    /// <param name="destinationPath">Absolute staging path that receives the cooked texture asset.</param>
    /// <param name="textureAsset">Cooked texture asset to serialize.</param>
    static void WriteTextureAsset(string destinationPath, TextureAsset textureAsset) {
        if (string.IsNullOrWhiteSpace(destinationPath)) {
            throw new ArgumentException("Destination path must be provided.", nameof(destinationPath));
        } else if (textureAsset == null) {
            throw new ArgumentNullException(nameof(textureAsset));
        }

        string directoryPath = Path.GetDirectoryName(destinationPath) ?? throw new InvalidOperationException("Destination directory path could not be resolved.");
        Directory.CreateDirectory(directoryPath);
        using FileStream stream = new FileStream(destinationPath, FileMode.Create, FileAccess.Write, FileShare.None);
        FilesAssetSerializer.Serialize(stream, textureAsset);
    }

    /// Resolves one work-item metadata value by key.
    /// </summary>
    /// <param name="workItem">Work item whose metadata should be searched.</param>
    /// <param name="key">Metadata key to resolve.</param>
    /// <returns>Matching metadata value when present; otherwise an empty string.</returns>
    static string ResolveMetadataValue(PlatformCookWorkItem workItem, string key) {
        if (workItem == null) {
            throw new ArgumentNullException(nameof(workItem));
        } else if (string.IsNullOrWhiteSpace(key)) {
            throw new ArgumentException("Metadata key must be provided.", nameof(key));
        }

        PlatformCookWorkItemMetadata[] metadata = workItem.Metadata ?? Array.Empty<PlatformCookWorkItemMetadata>();
        for (int index = 0; index < metadata.Length; index++) {
            PlatformCookWorkItemMetadata entry = metadata[index];
            if (entry != null && string.Equals(entry.Key, key, StringComparison.OrdinalIgnoreCase)) {
                return entry.Value ?? string.Empty;
            }
        }

        return string.Empty;
    }

    /// <summary>
    /// Resolves one source asset path beneath the project assets root.
    /// </summary>
    /// <param name="projectRootPath">Project root that owns the source asset.</param>
    /// <param name="sourceAssetPath">Relative or absolute source asset path emitted by the editor build graph.</param>
    /// <returns>Absolute source asset path.</returns>
    static string ResolveSourceAssetPath(string projectRootPath, string sourceAssetPath) {
        if (string.IsNullOrWhiteSpace(projectRootPath)) {
            throw new ArgumentException("Project root path must be provided.", nameof(projectRootPath));
        } else if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source asset path must be provided.", nameof(sourceAssetPath));
        }

        string normalizedSourcePath = NormalizePath(sourceAssetPath);
        if (Path.IsPathRooted(normalizedSourcePath)) {
            return Path.GetFullPath(normalizedSourcePath);
        }

        return Path.GetFullPath(Path.Combine(projectRootPath, "assets", normalizedSourcePath));
    }

    /// <summary>
    /// Normalizes one relative path for the current host filesystem.
    /// </summary>
    /// <param name="relativePath">Relative path emitted by the editor build graph.</param>
    /// <returns>Host-normalized relative path.</returns>
    static string NormalizePath(string relativePath) {
        if (string.IsNullOrWhiteSpace(relativePath)) {
            throw new ArgumentException("Relative path must be provided.", nameof(relativePath));
        }

        return relativePath.Replace('\\', Path.DirectorySeparatorChar).Replace('/', Path.DirectorySeparatorChar);
    }
}
