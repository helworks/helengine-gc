using System.Buffers.Binary;
using System.Text;

namespace helengine.gamecube.builder;

/// <summary>
/// Packages one staged GameCube extracted-disc layout into a raw retail-style disc image without external ISO tooling.
/// </summary>
public sealed class GameCubeRawImagePackager : IGameCubeImagePackager {
    /// <summary>
    /// Size of one GameCube DVD sector.
    /// </summary>
    const long DiscSectorSize = 0x800;

    /// <summary>
    /// Packages one staged GameCube extracted-disc layout into the final raw disc image path.
    /// </summary>
    /// <param name="layout">Extracted-disc layout that should be packaged.</param>
    /// <param name="outputImagePath">Final GameCube image path to write.</param>
    /// <param name="cancellationToken">Cancellation token that can stop packaging cooperatively.</param>
    public void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken) {
        if (layout == null) {
            throw new ArgumentNullException(nameof(layout));
        } else if (string.IsNullOrWhiteSpace(outputImagePath)) {
            throw new ArgumentException("Output image path is required.", nameof(outputImagePath));
        }

        ValidateDiscSource(layout);
        string outputDirectoryPath = Path.GetDirectoryName(outputImagePath) ?? throw new InvalidOperationException("Output image directory path could not be resolved.");
        Directory.CreateDirectory(outputDirectoryPath);

        string discRootPath = layout.DiscRootPath;
        string sysRootPath = Path.Combine(discRootPath, "sys");
        string filesRootPath = Path.Combine(discRootPath, "files");
        string bootBinPath = Path.Combine(sysRootPath, "boot.bin");
        string bi2BinPath = Path.Combine(sysRootPath, "bi2.bin");
        string apploaderImagePath = Path.Combine(sysRootPath, "apploader.img");
        string fstPath = Path.Combine(sysRootPath, "fst.bin");

        byte[] bootBinBytes = File.ReadAllBytes(bootBinPath);
        byte[] bi2BinBytes = File.ReadAllBytes(bi2BinPath);
        byte[] apploaderBytes = File.ReadAllBytes(apploaderImagePath);
        byte[] mainDolBytes = File.ReadAllBytes(layout.DiscExecutablePath);
        byte[] fstBytes = File.ReadAllBytes(fstPath);
        uint mainDolOffset = ReadBootField(bootBinBytes, GameCubeDiscSystemAreaWriter.BootBinDolOffsetFieldOffset);
        uint fstOffset = ReadBootField(bootBinBytes, GameCubeDiscSystemAreaWriter.BootBinFstOffsetFieldOffset);

        ValidateSystemArea(bootBinBytes, bi2BinBytes, mainDolBytes, fstBytes, mainDolOffset, fstOffset);
        IReadOnlyList<GameCubeRawImagePackagerFileEntry> fileEntries = ReadFileEntries(fstBytes);
        ValidatePayloadFiles(filesRootPath, fileEntries);
        long imageLength = CalculateImageLength(fileEntries, mainDolOffset, mainDolBytes.Length, fstOffset, fstBytes.Length);

        using FileStream outputStream = new(
            outputImagePath,
            FileMode.Create,
            FileAccess.ReadWrite,
            FileShare.None);
        outputStream.SetLength(imageLength);
        WriteBytes(outputStream, 0, bootBinBytes, cancellationToken);
        WriteBytes(outputStream, GameCubeDiscSystemAreaWriter.Bi2BinOffset, bi2BinBytes, cancellationToken);
        WriteBytes(outputStream, GameCubeDiscSystemAreaWriter.ApploaderOffset, apploaderBytes, cancellationToken);
        WriteBytes(outputStream, mainDolOffset, mainDolBytes, cancellationToken);
        WriteBytes(outputStream, fstOffset, fstBytes, cancellationToken);
        WriteFilePayloads(outputStream, filesRootPath, fileEntries, cancellationToken);
    }

    /// <summary>
    /// Validates that the staged extracted-disc root contains the required retail-style system files.
    /// </summary>
    /// <param name="layout">Extracted-disc layout to validate.</param>
    static void ValidateDiscSource(GameCubeDiscLayoutResult layout) {
        string bootBinPath = Path.Combine(layout.DiscRootPath, "sys", "boot.bin");
        string bi2BinPath = Path.Combine(layout.DiscRootPath, "sys", "bi2.bin");
        string apploaderImagePath = Path.Combine(layout.DiscRootPath, "sys", "apploader.img");
        string fstPath = Path.Combine(layout.DiscRootPath, "sys", "fst.bin");
        string filesRootPath = Path.Combine(layout.DiscRootPath, "files");

        if (!File.Exists(bootBinPath)) {
            throw new InvalidOperationException("The extracted GameCube disc root is missing sys/boot.bin.");
        } else if (!File.Exists(bi2BinPath)) {
            throw new InvalidOperationException("The extracted GameCube disc root is missing sys/bi2.bin.");
        } else if (!File.Exists(apploaderImagePath)) {
            throw new InvalidOperationException("The extracted GameCube disc root is missing sys/apploader.img.");
        } else if (!File.Exists(layout.DiscExecutablePath)) {
            throw new InvalidOperationException("The extracted GameCube disc root is missing sys/main.dol.");
        } else if (!File.Exists(fstPath)) {
            throw new InvalidOperationException("The extracted GameCube disc root is missing sys/fst.bin.");
        } else if (!Directory.Exists(filesRootPath)) {
            throw new InvalidOperationException("The extracted GameCube disc root is missing the files/ payload directory.");
        }
    }

    /// <summary>
    /// Validates that the staged retail-style system area is internally consistent before the raw image is written.
    /// </summary>
    /// <param name="bootBinBytes">Serialized <c>boot.bin</c> bytes.</param>
    /// <param name="bi2BinBytes">Serialized <c>bi2.bin</c> bytes.</param>
    /// <param name="mainDolBytes">Serialized <c>main.dol</c> bytes.</param>
    /// <param name="fstBytes">Serialized <c>fst.bin</c> bytes.</param>
    /// <param name="mainDolOffset">Decoded main DOL offset from <c>boot.bin</c>.</param>
    /// <param name="fstOffset">Decoded FST offset from <c>boot.bin</c>.</param>
    static void ValidateSystemArea(
        byte[] bootBinBytes,
        byte[] bi2BinBytes,
        byte[] mainDolBytes,
        byte[] fstBytes,
        uint mainDolOffset,
        uint fstOffset) {
        if (bootBinBytes.Length != GameCubeDiscSystemAreaWriter.BootBinSize) {
            throw new InvalidOperationException("The staged GameCube sys/boot.bin size was invalid.");
        } else if (bi2BinBytes.Length != GameCubeDiscSystemAreaWriter.Bi2BinSize) {
            throw new InvalidOperationException("The staged GameCube sys/bi2.bin size was invalid.");
        } else if (BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(0x1C, sizeof(uint))) != 0xC2339F3DU) {
            throw new InvalidOperationException("The staged GameCube sys/boot.bin magic was invalid.");
        } else if (mainDolOffset < GameCubeDiscSystemAreaWriter.ApploaderOffset) {
            throw new InvalidOperationException("The staged GameCube sys/boot.bin main DOL offset was invalid.");
        } else if (mainDolOffset < GameCubeDiscSystemAreaWriter.FirstPayloadOffset) {
            throw new InvalidOperationException("The staged GameCube sys/boot.bin main DOL offset overlapped the reserved retail system area.");
        } else if (fstOffset < mainDolOffset + mainDolBytes.Length) {
            throw new InvalidOperationException("The staged GameCube sys/boot.bin FST offset was invalid.");
        }
    }

    /// <summary>
    /// Reads all file payload entries from the staged GameCube <c>sys/fst.bin</c> bytes.
    /// </summary>
    /// <param name="fstBytes">Serialized <c>sys/fst.bin</c> bytes.</param>
    /// <returns>Parsed file entries in staged FST order.</returns>
    static IReadOnlyList<GameCubeRawImagePackagerFileEntry> ReadFileEntries(byte[] fstBytes) {
        int rootEntryCount = checked((int)BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(8, sizeof(uint))));
        int stringTableOffset = rootEntryCount * GameCubeDiscFileSystemTableBuilder.EntrySize;
        List<GameCubeRawImagePackagerFileEntry> fileEntries = [];
        List<string> directoryPaths = [string.Empty];

        for (int entryIndex = 1; entryIndex < rootEntryCount; entryIndex++) {
            int entryOffset = entryIndex * GameCubeDiscFileSystemTableBuilder.EntrySize;
            bool isDirectory = fstBytes[entryOffset] != 0;
            int nameOffset = (fstBytes[entryOffset + 1] << 16) | (fstBytes[entryOffset + 2] << 8) | fstBytes[entryOffset + 3];
            string entryName = ReadName(fstBytes, stringTableOffset + nameOffset);
            if (isDirectory) {
                int parentIndex = checked((int)BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(entryOffset + 4, sizeof(uint))));
                while (directoryPaths.Count <= entryIndex) {
                    directoryPaths.Add(string.Empty);
                }

                directoryPaths[entryIndex] = CombineRelativePath(directoryPaths[parentIndex], entryName);
                continue;
            }

            int parentDirectoryIndex = ResolveParentDirectoryIndex(fstBytes, entryIndex);
            string relativePath = CombineRelativePath(directoryPaths[parentDirectoryIndex], entryName);
            uint dataOffset = BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(entryOffset + 4, sizeof(uint)));
            uint dataSize = BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(entryOffset + 8, sizeof(uint)));
            fileEntries.Add(new GameCubeRawImagePackagerFileEntry(relativePath, dataOffset, dataSize));
        }

        return fileEntries;
    }

    /// <summary>
    /// Validates that every staged file referenced by the serialized FST exists and fits within the fixed GameCube disc size.
    /// </summary>
    /// <param name="filesRootPath">Staged <c>files/</c> root.</param>
    /// <param name="fileEntries">Parsed file entries from <c>sys/fst.bin</c>.</param>
    static void ValidatePayloadFiles(string filesRootPath, IReadOnlyList<GameCubeRawImagePackagerFileEntry> fileEntries) {
        for (int entryIndex = 0; entryIndex < fileEntries.Count; entryIndex++) {
            GameCubeRawImagePackagerFileEntry fileEntry = fileEntries[entryIndex];
            string sourcePath = Path.Combine(filesRootPath, fileEntry.RelativePath.Replace('/', Path.DirectorySeparatorChar));
            if (!File.Exists(sourcePath)) {
                throw new InvalidOperationException($"The extracted GameCube disc root is missing staged file '{fileEntry.RelativePath}' referenced by sys/fst.bin.");
            }

            FileInfo fileInfo = new(sourcePath);
            if (fileInfo.Length != fileEntry.DataSize) {
                throw new InvalidOperationException($"The staged GameCube file '{fileEntry.RelativePath}' size did not match the size encoded in sys/fst.bin.");
            }
        }
    }

    /// <summary>
    /// Calculates the truncated raw image length needed to cover the retail system area, DOL, FST, and all staged payload files.
    /// </summary>
    /// <param name="fileEntries">Parsed payload entries from <c>sys/fst.bin</c>.</param>
    /// <param name="mainDolOffset">Decoded main DOL byte offset from <c>boot.bin</c>.</param>
    /// <param name="mainDolLength">Serialized <c>main.dol</c> byte length.</param>
    /// <param name="fstOffset">Decoded FST byte offset from <c>boot.bin</c>.</param>
    /// <param name="fstLength">Serialized <c>fst.bin</c> byte length.</param>
    /// <returns>Aligned truncated image length.</returns>
    static long CalculateImageLength(
        IReadOnlyList<GameCubeRawImagePackagerFileEntry> fileEntries,
        uint mainDolOffset,
        int mainDolLength,
        uint fstOffset,
        int fstLength) {
        long maximumOffset = Math.Max(
            GameCubeDiscSystemAreaWriter.FirstPayloadOffset,
            Math.Max(
                mainDolOffset + (long)mainDolLength,
                fstOffset + (long)fstLength));

        for (int entryIndex = 0; entryIndex < fileEntries.Count; entryIndex++) {
            GameCubeRawImagePackagerFileEntry fileEntry = fileEntries[entryIndex];
            maximumOffset = Math.Max(maximumOffset, fileEntry.DataOffset + (long)fileEntry.DataSize);
        }

        return Align32(maximumOffset);
    }

    /// <summary>
    /// Writes one serialized byte block at the supplied disc offset.
    /// </summary>
    /// <param name="outputStream">Open destination raw image stream.</param>
    /// <param name="offset">Disc byte offset.</param>
    /// <param name="bytes">Bytes to write.</param>
    /// <param name="cancellationToken">Cancellation token used to stop the write cooperatively.</param>
    static void WriteBytes(FileStream outputStream, long offset, byte[] bytes, CancellationToken cancellationToken) {
        cancellationToken.ThrowIfCancellationRequested();
        outputStream.Seek(offset, SeekOrigin.Begin);
        outputStream.Write(bytes, 0, bytes.Length);
    }

    /// <summary>
    /// Aligns one raw image length to the next GameCube DVD sector boundary so the final sector remains readable.
    /// </summary>
    /// <param name="value">Raw image length or offset to align.</param>
    /// <returns>Aligned value.</returns>
    static long Align32(long value) {
        return (value + DiscSectorSize - 1L) & ~(DiscSectorSize - 1L);
    }

    /// <summary>
    /// Copies all staged payload files into the raw disc image at the exact offsets authored in <c>sys/fst.bin</c>.
    /// </summary>
    /// <param name="outputStream">Open destination raw image stream.</param>
    /// <param name="filesRootPath">Staged <c>files/</c> root.</param>
    /// <param name="fileEntries">Parsed file entries from <c>sys/fst.bin</c>.</param>
    /// <param name="cancellationToken">Cancellation token used to stop the copy cooperatively.</param>
    static void WriteFilePayloads(
        FileStream outputStream,
        string filesRootPath,
        IReadOnlyList<GameCubeRawImagePackagerFileEntry> fileEntries,
        CancellationToken cancellationToken) {
        for (int entryIndex = 0; entryIndex < fileEntries.Count; entryIndex++) {
            cancellationToken.ThrowIfCancellationRequested();
            GameCubeRawImagePackagerFileEntry fileEntry = fileEntries[entryIndex];
            string sourcePath = Path.Combine(filesRootPath, fileEntry.RelativePath.Replace('/', Path.DirectorySeparatorChar));
            outputStream.Seek(fileEntry.DataOffset, SeekOrigin.Begin);
            using FileStream inputStream = File.OpenRead(sourcePath);
            inputStream.CopyTo(outputStream);
        }
    }

    /// <summary>
    /// Decodes one boot-header field that stores an offset or size in raw bytes.
    /// </summary>
    /// <param name="bootBinBytes">Serialized <c>boot.bin</c> bytes.</param>
    /// <param name="fieldOffset">Field byte offset inside <c>boot.bin</c>.</param>
    /// <returns>Decoded byte offset or size.</returns>
    static uint ReadBootField(byte[] bootBinBytes, int fieldOffset) {
        return BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(fieldOffset, sizeof(uint)));
    }

    /// <summary>
    /// Resolves the nearest parent directory index for one file entry by scanning the directory ranges that contain it.
    /// </summary>
    /// <param name="fstBytes">Serialized <c>sys/fst.bin</c> bytes.</param>
    /// <param name="entryIndex">File entry index whose parent directory should be resolved.</param>
    /// <returns>Nearest containing directory index.</returns>
    static int ResolveParentDirectoryIndex(byte[] fstBytes, int entryIndex) {
        int rootEntryCount = checked((int)BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(8, sizeof(uint))));
        int parentIndex = 0;
        int smallestContainingRange = int.MaxValue;
        for (int candidateIndex = 1; candidateIndex < rootEntryCount; candidateIndex++) {
            int candidateOffset = candidateIndex * GameCubeDiscFileSystemTableBuilder.EntrySize;
            if (fstBytes[candidateOffset] == 0) {
                continue;
            }

            int directoryEndIndex = checked((int)BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(candidateOffset + 8, sizeof(uint))));
            if (candidateIndex < entryIndex && directoryEndIndex > entryIndex) {
                int rangeSize = directoryEndIndex - candidateIndex;
                if (rangeSize < smallestContainingRange) {
                    smallestContainingRange = rangeSize;
                    parentIndex = candidateIndex;
                }
            }
        }

        return parentIndex;
    }

    /// <summary>
    /// Reads one null-terminated ASCII entry name from the staged FST string table.
    /// </summary>
    /// <param name="fstBytes">Serialized <c>sys/fst.bin</c> bytes.</param>
    /// <param name="offset">String-table byte offset.</param>
    /// <returns>Decoded entry name.</returns>
    static string ReadName(byte[] fstBytes, int offset) {
        int endOffset = offset;
        while (endOffset < fstBytes.Length && fstBytes[endOffset] != 0) {
            endOffset++;
        }

        return Encoding.ASCII.GetString(fstBytes, offset, endOffset - offset);
    }

    /// <summary>
    /// Combines two slash-normalized relative path segments.
    /// </summary>
    /// <param name="basePath">Base relative path.</param>
    /// <param name="entryName">Child entry name.</param>
    /// <returns>Combined relative path.</returns>
    static string CombineRelativePath(string basePath, string entryName) {
        if (string.IsNullOrWhiteSpace(basePath)) {
            return entryName;
        }

        return basePath + "/" + entryName;
    }
}
