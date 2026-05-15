using System.Buffers.Binary;
using System.Text;

namespace helengine.gamecube.builder;

/// <summary>
/// Writes the required GameCube system-area files for one extracted-disc root before image packaging.
/// </summary>
public sealed class GameCubeDiscSystemAreaWriter {
    /// <summary>
    /// Size in bytes of one GameCube <c>boot.bin</c> file.
    /// </summary>
    public const int BootBinSize = 0x440;

    /// <summary>
    /// Size in bytes of one GameCube <c>bi2.bin</c> file.
    /// </summary>
    public const int Bi2BinSize = 0x2000;

    /// <summary>
    /// Disc offset where <c>bi2.bin</c> begins in a retail-style GameCube image.
    /// </summary>
    public const uint Bi2BinOffset = BootBinSize;

    /// <summary>
    /// Disc offset where <c>apploader.img</c> begins in a retail-style GameCube image.
    /// </summary>
    public const uint ApploaderOffset = Bi2BinOffset + Bi2BinSize;

    /// <summary>
    /// Disc offset where the reserved retail-style GameCube <c>region.bin</c> slot begins.
    /// </summary>
    public const uint RegionBinOffset = 0x4E000;

    /// <summary>
    /// Size in bytes of the reserved retail-style GameCube <c>region.bin</c> slot.
    /// </summary>
    public const uint RegionBinSize = 0x20;

    /// <summary>
    /// First legal disc offset for authored payload data after the reserved GameCube system area.
    /// </summary>
    public const uint FirstPayloadOffset = RegionBinOffset + RegionBinSize;

    /// <summary>
    /// Boot header offset that stores the final main DOL offset in raw bytes for extracted-directory composition.
    /// </summary>
    public const int BootBinDolOffsetFieldOffset = 0x420;

    /// <summary>
    /// Boot header offset that stores the final FST offset in raw bytes for extracted-directory composition.
    /// </summary>
    public const int BootBinFstOffsetFieldOffset = 0x424;

    /// <summary>
    /// Boot header offset that stores the final FST size in raw bytes for extracted-directory composition.
    /// </summary>
    public const int BootBinFstSizeFieldOffset = 0x428;

    /// <summary>
    /// Boot header offset that stores the maximum staged FST size in raw bytes for extracted-directory composition.
    /// </summary>
    public const int BootBinFstMaxSizeFieldOffset = 0x42C;

    /// <summary>
    /// Magic value written into the GameCube disc header.
    /// </summary>
    const uint GameCubeDiscMagic = 0xC2339F3D;

    /// <summary>
    /// Writes the GameCube system-area files into the supplied extracted-disc root.
    /// </summary>
    /// <param name="discRootPath">Destination extracted-disc root.</param>
    /// <param name="nativeExecutablePath">Built GameCube executable to stage as <c>sys/main.dol</c>.</param>
    /// <param name="options">Explicit system-area inputs used to stage boot metadata and the apploader image.</param>
    public void Write(
        string discRootPath,
        string nativeExecutablePath,
        GameCubeDiscSystemAreaOptions options) {
        if (string.IsNullOrWhiteSpace(discRootPath)) {
            throw new ArgumentException("Disc root path is required.", nameof(discRootPath));
        } else if (!File.Exists(nativeExecutablePath)) {
            throw new FileNotFoundException("Native GameCube executable is required before disc staging.", nativeExecutablePath);
        } else if (options == null) {
            throw new ArgumentNullException(nameof(options));
        } else if (!File.Exists(options.ApploaderImagePath)) {
            throw new FileNotFoundException("Configured GameCube apploader image was not found.", options.ApploaderImagePath);
        }

        string sysRootPath = Path.Combine(discRootPath, "sys");
        Directory.CreateDirectory(sysRootPath);

        string mainDolPath = Path.Combine(sysRootPath, "main.dol");
        string apploaderOutputPath = Path.Combine(sysRootPath, "apploader.img");
        File.Copy(nativeExecutablePath, mainDolPath, true);
        File.Copy(options.ApploaderImagePath, apploaderOutputPath, true);
        File.WriteAllBytes(Path.Combine(sysRootPath, "bi2.bin"), BuildBi2Bin());
        File.WriteAllBytes(Path.Combine(sysRootPath, "boot.bin"), BuildBootBin(discRootPath, mainDolPath, apploaderOutputPath, options));
        File.WriteAllText(Path.Combine(discRootPath, "setup.txt"), "disc-type = GameCube\n");
    }

    /// <summary>
    /// Builds one synthetic GameCube <c>boot.bin</c> header suitable for extracted-disc packaging.
    /// </summary>
    /// <param name="options">Disc metadata to encode into the header.</param>
    /// <returns>Binary <c>boot.bin</c> payload.</returns>
    static byte[] BuildBootBin(
        string discRootPath,
        string mainDolPath,
        string apploaderImagePath,
        GameCubeDiscSystemAreaOptions options) {
        byte[] buffer = new byte[BootBinSize];
        string normalizedDiscId = NormalizeDiscId(options.DiscId);
        Encoding ascii = Encoding.ASCII;
        ascii.GetBytes(normalizedDiscId, 0, normalizedDiscId.Length, buffer, 0);
        BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(0x1C, sizeof(uint)), GameCubeDiscMagic);

        string normalizedTitle = NormalizeDiscTitle(options.DiscTitle);
        int titleByteCount = ascii.GetByteCount(normalizedTitle);
        ascii.GetBytes(normalizedTitle, 0, normalizedTitle.Length, buffer, 0x20);
        Array.Clear(buffer, 0x20 + titleByteCount, 0x3E0 - titleByteCount);

        uint minimumMainDolOffset = Align32(FirstPayloadOffset);
        uint computedMainDolOffset = Align32(ApploaderOffset + checked((uint)new FileInfo(apploaderImagePath).Length) + 0x20U);
        uint mainDolOffset = Math.Max(minimumMainDolOffset, computedMainDolOffset);
        byte[] fstBytes = BuildFileSystemTable(discRootPath, mainDolPath, mainDolOffset);
        string fstPath = Path.Combine(discRootPath, "sys", "fst.bin");
        File.WriteAllBytes(fstPath, fstBytes);
        uint fstOffset = Align32(mainDolOffset + checked((uint)new FileInfo(mainDolPath).Length) + 0x20U);
        uint fstSize = checked((uint)fstBytes.Length);
        WriteBootField(buffer, BootBinDolOffsetFieldOffset, mainDolOffset);
        WriteBootField(buffer, BootBinFstOffsetFieldOffset, fstOffset);
        WriteBootField(buffer, BootBinFstSizeFieldOffset, fstSize);
        WriteBootField(buffer, BootBinFstMaxSizeFieldOffset, fstSize);
        return buffer;
    }

    /// <summary>
    /// Aligns one disc offset to the next 32-byte boundary used by GameCube executable and file-system payloads.
    /// </summary>
    /// <param name="value">Disc offset to align.</param>
    /// <returns>Aligned disc offset.</returns>
    static uint Align32(uint value) {
        return (value + 31U) & ~31U;
    }

    /// <summary>
    /// Builds the serialized GameCube <c>sys/fst.bin</c> payload for the extracted <c>files/</c> tree.
    /// </summary>
    /// <param name="discRootPath">Extracted-disc root that already contains the staged <c>files/</c> payload tree.</param>
    /// <param name="mainDolPath">Staged <c>sys/main.dol</c> path.</param>
    /// <param name="mainDolOffset">Final disc offset where <c>sys/main.dol</c> will be staged.</param>
    /// <returns>Serialized <c>sys/fst.bin</c> payload.</returns>
    static byte[] BuildFileSystemTable(
        string discRootPath,
        string mainDolPath,
        uint mainDolOffset) {
        string filesRootPath = Path.Combine(discRootPath, "files");
        if (!Directory.Exists(filesRootPath)) {
            throw new DirectoryNotFoundException($"Extracted files root '{filesRootPath}' was not found.");
        }

        uint fstOffset = Align32(mainDolOffset + checked((uint)new FileInfo(mainDolPath).Length));
        GameCubeDiscFileSystemTableBuilder builder = new();
        return builder.Build(filesRootPath, fstOffset);
    }

    /// <summary>
    /// Writes one boot-header field that stores a disc offset or size in raw bytes for GameCube extracted-directory composition.
    /// </summary>
    /// <param name="buffer">Mutable <c>boot.bin</c> payload.</param>
    /// <param name="fieldOffset">Byte offset of the 32-bit field.</param>
    /// <param name="value">Disc offset or byte length to encode.</param>
    static void WriteBootField(byte[] buffer, int fieldOffset, uint value) {
        BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(fieldOffset, sizeof(uint)), value);
    }

    /// <summary>
    /// Builds one synthetic GameCube <c>bi2.bin</c> payload using only the documented public fields required for first-boot packaging.
    /// </summary>
    /// <returns>Binary <c>bi2.bin</c> payload.</returns>
    static byte[] BuildBi2Bin() {
        byte[] buffer = new byte[Bi2BinSize];
        BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(0x18, sizeof(uint)), 1U);
        return buffer;
    }

    /// <summary>
    /// Normalizes one authored disc identifier into one GameCube-shaped ID6 value with console, game, country, and maker fields.
    /// </summary>
    /// <param name="discId">Authored disc identifier.</param>
    /// <returns>Normalized six-character disc identifier.</returns>
    static string NormalizeDiscId(string discId) {
        StringBuilder normalizedSourceBuilder = new(6);
        for (int index = 0; index < discId.Length && normalizedSourceBuilder.Length < 6; index++) {
            char character = char.ToUpperInvariant(discId[index]);
            if (char.IsAsciiLetterOrDigit(character)) {
                normalizedSourceBuilder.Append(character);
            }
        }

        string normalizedSource = normalizedSourceBuilder.ToString();
        string gameCode = normalizedSource.Length >= 2
            ? normalizedSource.Substring(0, 2)
            : normalizedSource.PadRight(2, 'X');
        string makerCode = normalizedSource.Length >= 4
            ? normalizedSource.Substring(normalizedSource.Length - 2, 2)
            : "HE";

        StringBuilder builder = new(6);
        builder.Append('G');
        builder.Append(gameCode);
        builder.Append('E');
        builder.Append(makerCode);
        while (builder.Length < 6) {
            builder.Append('X');
        }

        return builder.ToString();
    }

    /// <summary>
    /// Normalizes one authored disc title into the printable ASCII subset stored in GameCube <c>boot.bin</c>.
    /// </summary>
    /// <param name="discTitle">Authored disc title.</param>
    /// <returns>Normalized disc title that fits the GameCube header payload.</returns>
    static string NormalizeDiscTitle(string discTitle) {
        StringBuilder builder = new(0x3E0);
        for (int index = 0; index < discTitle.Length && builder.Length < 0x3E0; index++) {
            char character = discTitle[index];
            if (character >= 0x20 && character <= 0x7E) {
                builder.Append(character);
            }
        }

        if (builder.Length == 0) {
            builder.Append("HELENGINE GAMECUBE");
        }

        return builder.ToString();
    }
}
