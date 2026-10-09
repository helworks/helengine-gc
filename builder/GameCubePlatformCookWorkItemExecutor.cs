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

    /// <summary>Prepares the entire texture batch before publishing any output in the staging root.</summary>
    public void Execute(PlatformCookWorkItem[] workItems, string projectRootPath, string stagingRootPath) {
        IReadOnlyList<GameCubeTextureCookWorkPlan> plans = Prepare(workItems, [], projectRootPath, stagingRootPath, CancellationToken.None);
        Publish(plans, CancellationToken.None);
    }

    /// <summary>Validates all contracts and disc paths, then decodes and cooks every source without changing staged outputs.</summary>
    internal IReadOnlyList<GameCubeTextureCookWorkPlan> Prepare(
        PlatformCookWorkItem[] workItems, PlatformBuildArtifact[] artifacts, string projectRootPath, string stagingRootPath, CancellationToken cancellationToken) {
        ArgumentNullException.ThrowIfNull(workItems);
        ArgumentNullException.ThrowIfNull(artifacts);
        if (string.IsNullOrWhiteSpace(projectRootPath)) throw new ArgumentException("Project root is required.", nameof(projectRootPath));
        if (string.IsNullOrWhiteSpace(stagingRootPath)) throw new ArgumentException("Staging root is required.", nameof(stagingRootPath));
        cancellationToken.ThrowIfCancellationRequested();
        GameCubeTextureCookPath.ValidateRoot(stagingRootPath);
        HashSet<string> reserved = new(StringComparer.OrdinalIgnoreCase);
        foreach (PlatformBuildArtifact artifact in artifacts) {
            GameCubeTextureCookPath.Reserve(reserved, artifact.RelativePath);
            GameCubeTextureCookPath.Resolve(stagingRootPath, artifact.RelativePath);
        }
        HashSet<string> sources = new(StringComparer.OrdinalIgnoreCase);
        foreach (PlatformCookWorkItem item in workItems) {
            if (item == null) throw new InvalidOperationException("GameCube cook work items cannot be null.");
            sources.Add(ResolveSourceAssetPath(projectRootPath, item.SourceAssetPath));
        }
        List<GameCubeTextureCookWorkPlan> plans = new(workItems.Length);
        foreach (PlatformCookWorkItem item in workItems) {
            cancellationToken.ThrowIfCancellationRequested();
            if (!string.Equals(item.TargetPlatformId, "gamecube", StringComparison.OrdinalIgnoreCase) ||
                !string.Equals(item.TargetArtifactKind, "runtime-texture", StringComparison.OrdinalIgnoreCase) ||
                (!string.Equals(item.SourceAssetKind, "texture", StringComparison.OrdinalIgnoreCase) && !string.Equals(item.SourceAssetKind, "font-atlas-texture", StringComparison.OrdinalIgnoreCase))) {
                throw new InvalidOperationException($"Unsupported GameCube cook contract '{item.WorkItemId}'.");
            }
            GameCubeTextureCookSettings settings = GameCubeTextureCookSettings.Parse(item.SerializedPlatformSettings);
            string relative = GameCubeTextureCookPath.Normalize(item.OutputRelativePath);
            GameCubeTextureCookPath.Reserve(reserved, relative);
            string destination = GameCubeTextureCookPath.Resolve(stagingRootPath, relative);
            string source = ResolveSourceAssetPath(projectRootPath, item.SourceAssetPath);
            if (source.StartsWith(Path.TrimEndingDirectorySeparator(Path.GetFullPath(stagingRootPath)) + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("GameCube source cannot reside in the staging tree being replaced.");
            if (sources.Contains(destination)) throw new InvalidOperationException("GameCube cook output would replace a source.");
            if (!File.Exists(source)) throw new FileNotFoundException("GameCube cook source was not found.", source);
            plans.Add(new GameCubeTextureCookWorkPlan { WorkItem = item, Settings = settings, SourcePath = source, DestinationPath = destination, StagingRootPath = stagingRootPath });
        }
        foreach (GameCubeTextureCookWorkPlan plan in plans) {
            cancellationToken.ThrowIfCancellationRequested();
            using TextureAsset source = LoadSourceTexture(plan.WorkItem, projectRootPath);
            using TextureAsset cooked = TextureCooker.CookTexture(source, plan.Settings);
            cooked.Id = plan.WorkItem.OutputLogicalArtifactId;
            cooked.RuntimeAssetId = RuntimeAssetIdGenerator.Generate(cooked.Id);
            plan.Payload = FilesAssetSerializer.SerializeToBytes(cooked);
        }
        cancellationToken.ThrowIfCancellationRequested();
        return plans;
    }

    /// <summary>Rechecks contained destinations and writes only a fully prepared texture batch.</summary>
    internal void Publish(IReadOnlyList<GameCubeTextureCookWorkPlan> plans, CancellationToken cancellationToken) {
        cancellationToken.ThrowIfCancellationRequested();
        foreach (GameCubeTextureCookWorkPlan plan in plans) GameCubeTextureCookPath.Resolve(plan.StagingRootPath, plan.WorkItem.OutputRelativePath);
        foreach (GameCubeTextureCookWorkPlan plan in plans) {
            Directory.CreateDirectory(Path.GetDirectoryName(plan.DestinationPath));
            File.WriteAllBytes(plan.DestinationPath, plan.Payload);
        }
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
        if (string.Equals(workItem.SourceAssetKind, "font-atlas-texture", StringComparison.OrdinalIgnoreCase) && Path.GetExtension(sourcePath).Equals(".hefont", StringComparison.OrdinalIgnoreCase)) {
            using FileStream fontStream = File.OpenRead(sourcePath);
            FontAsset font = helengine.files.FontAssetBinarySerializer.Deserialize(fontStream);
            if (font.SourceTextureAsset == null) throw new InvalidOperationException("GameCube font cook source has no embedded atlas.");
            return font.SourceTextureAsset;
        }
        if (TryReadTextureAsset(sourcePath, out TextureAsset textureAsset)) {
            return textureAsset;
        }

        string bitmapSourcePath = ResolveBitmapSourcePath(sourcePath);
        if (!OperatingSystem.IsWindowsVersionAtLeast(6, 1)) throw new PlatformNotSupportedException("GameCube authored image decoding requires Windows; serialized atlas sources are portable.");
        Bitmap sourceBitmap;
        try {
            sourceBitmap = new Bitmap(bitmapSourcePath);
        } catch (ArgumentException exception) {
            throw new InvalidOperationException($"Texture source '{workItem.SourceAssetPath}' could not be read from '{bitmapSourcePath}'.", exception);
        }

        using (sourceBitmap) {
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
    }

    /// <summary>
    /// Reads the logical RGBA32 colors from one 32-bit ARGB bitmap lock.
    /// </summary>
    /// <param name="bitmapData">Locked bitmap payload.</param>
    /// <param name="width">Bitmap width in pixels.</param>
    /// <param name="height">Bitmap height in pixels.</param>
    /// <returns>Tightly packed RGBA32 bytes.</returns>
    [System.Runtime.Versioning.SupportedOSPlatform("windows6.1")]
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
            && !string.Equals(extension, ".heasset", StringComparison.OrdinalIgnoreCase)
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
    /// Resolves the bitmap source path that should be imported when one work-item source path is not a serialized texture asset.
    /// </summary>
    /// <param name="sourcePath">Absolute authored source path emitted by the editor work item.</param>
    /// <returns>Absolute bitmap source path that System.Drawing can load.</returns>
    static string ResolveBitmapSourcePath(string sourcePath) {
        if (string.IsNullOrWhiteSpace(sourcePath)) {
            throw new ArgumentException("Source path must be provided.", nameof(sourcePath));
        }

        if (TryResolveImportSettingsSiblingSourcePath(sourcePath, out string siblingSourcePath)) {
            return siblingSourcePath;
        }

        return sourcePath;
    }

    /// <summary>
    /// Attempts to resolve the sibling authored image path for one import-settings sidecar.
    /// </summary>
    /// <param name="sourcePath">Absolute work-item source path that may point at one `.hasset` import-settings sidecar.</param>
    /// <param name="siblingSourcePath">Resolved authored image path when the sidecar has one sibling source image.</param>
    /// <returns>True when the supplied path was one import-settings sidecar with a sibling source image.</returns>
    static bool TryResolveImportSettingsSiblingSourcePath(string sourcePath, out string siblingSourcePath) {
        if (string.IsNullOrWhiteSpace(sourcePath)) {
            throw new ArgumentException("Source path must be provided.", nameof(sourcePath));
        }

        siblingSourcePath = string.Empty;
        if (!string.Equals(Path.GetExtension(sourcePath), ".hasset", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        string candidateSourcePath = Path.Combine(
            Path.GetDirectoryName(sourcePath) ?? string.Empty,
            Path.GetFileNameWithoutExtension(sourcePath));
        if (!File.Exists(candidateSourcePath)) {
            return false;
        }

        if (string.IsNullOrWhiteSpace(Path.GetExtension(candidateSourcePath))) {
            return false;
        }

        siblingSourcePath = candidateSourcePath;
        return true;
    }

    /// <summary>

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

        if (Path.IsPathRooted(sourceAssetPath)) {
            return Path.GetFullPath(sourceAssetPath);
        }
        return GameCubeTextureCookPath.Resolve(Path.Combine(projectRootPath, "assets"), sourceAssetPath);
    }

}
