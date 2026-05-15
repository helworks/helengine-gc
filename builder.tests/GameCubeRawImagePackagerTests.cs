using System.Buffers.Binary;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the direct raw GameCube image packager that writes retail-style disc bytes without external ISO tooling.
/// </summary>
public sealed class GameCubeRawImagePackagerTests {
    /// <summary>
    /// Ensures the packager writes the staged system area, FST, and file payloads into the final raw GameCube image at the offsets authored in <c>boot.bin</c> and <c>fst.bin</c>.
    /// </summary>
    [Fact]
    public void Package_WritesRawGameCubeImageUsingStagedSystemAreaAndFstOffsets() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-raw-image-packager-tests", Guid.NewGuid().ToString("N"));
        string stagingRootPath = Path.Combine(rootPath, "staging");
        string discRootPath = Path.Combine(rootPath, "disc");
        string nativeExecutablePath = Path.Combine(rootPath, "build", "helengine_gc.dol");
        string apploaderImagePath = Path.Combine(rootPath, "tooling", "apploader.img");
        string outputImagePath = Path.Combine(rootPath, "game.gcm");
        string scenePath = Path.Combine(stagingRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string modelPath = Path.Combine(stagingRootPath, "cooked", "engine", "models", "cube.hasset");

        Directory.CreateDirectory(Path.GetDirectoryName(scenePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(modelPath) ?? throw new InvalidOperationException("Model directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(nativeExecutablePath) ?? throw new InvalidOperationException("Native executable directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
        File.WriteAllBytes(scenePath, [0x53, 0x43, 0x45, 0x4E, 0x45]);
        File.WriteAllBytes(modelPath, [0x4D, 0x4F, 0x44, 0x45, 0x4C, 0x21]);
        File.WriteAllBytes(nativeExecutablePath, [0x44, 0x4F, 0x4C]);
        File.WriteAllBytes(apploaderImagePath, [0x41, 0x50, 0x50, 0x4C]);

        try {
            GameCubeDiscLayoutResult layout = new GameCubeDiscLayoutWriter().Write(
                stagingRootPath,
                nativeExecutablePath,
                discRootPath,
                new GameCubeDiscSystemAreaOptions(apploaderImagePath, "PROJHE", "project"));

            new GameCubeRawImagePackager().Package(layout, outputImagePath, CancellationToken.None);

            Assert.True(File.Exists(outputImagePath));
            FileInfo outputImage = new(outputImagePath);
            Assert.Equal(GameCubeRawImagePackager.DiscSize, outputImage.Length);

            byte[] bootBinBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "boot.bin"));
            byte[] bi2BinBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "bi2.bin"));
            byte[] apploaderBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "apploader.img"));
            byte[] mainDolBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "main.dol"));
            byte[] fstBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "fst.bin"));

            Assert.Equal(bootBinBytes, ReadImageRange(outputImagePath, 0, bootBinBytes.Length));
            Assert.Equal(bi2BinBytes, ReadImageRange(outputImagePath, GameCubeDiscSystemAreaWriter.Bi2BinOffset, bi2BinBytes.Length));
            Assert.Equal(apploaderBytes, ReadImageRange(outputImagePath, GameCubeDiscSystemAreaWriter.ApploaderOffset, apploaderBytes.Length));

            uint mainDolOffset = ReadBootField(bootBinBytes, GameCubeDiscSystemAreaWriter.BootBinDolOffsetFieldOffset);
            uint fstOffset = ReadBootField(bootBinBytes, GameCubeDiscSystemAreaWriter.BootBinFstOffsetFieldOffset);
            Assert.Equal(mainDolBytes, ReadImageRange(outputImagePath, mainDolOffset, mainDolBytes.Length));
            Assert.Equal(fstBytes, ReadImageRange(outputImagePath, fstOffset, fstBytes.Length));

            Dictionary<string, byte[]> payloadsByPath = new(StringComparer.Ordinal) {
                ["cooked/engine/models/cube.hasset"] = File.ReadAllBytes(Path.Combine(discRootPath, "files", "cooked", "engine", "models", "cube.hasset")),
                ["cooked/scenes/rendering/cube_test.hasset"] = File.ReadAllBytes(Path.Combine(discRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset"))
            };
            IReadOnlyDictionary<string, GameCubeRawImageFileEntry> fileEntries = ReadFileEntries(fstBytes);
            Assert.Equal(payloadsByPath.Count, fileEntries.Count);

            foreach (KeyValuePair<string, byte[]> payload in payloadsByPath) {
                Assert.True(fileEntries.TryGetValue(payload.Key, out GameCubeRawImageFileEntry fileEntry));
                Assert.Equal(payload.Value, ReadImageRange(outputImagePath, fileEntry.DataOffset, checked((int)fileEntry.DataSize)));
            }
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Ensures the raw image packager fails clearly when the staged retail-style disc root is missing <c>sys/fst.bin</c>.
    /// </summary>
    [Fact]
    public void Package_WhenDiscSourceIsMissingFstBin_ThrowsInvalidOperationException() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-raw-image-packager-tests", Guid.NewGuid().ToString("N"));
        string discRootPath = Path.Combine(rootPath, "disc");
        string sysRootPath = Path.Combine(discRootPath, "sys");
        string filesRootPath = Path.Combine(discRootPath, "files");
        string mainDolPath = Path.Combine(sysRootPath, "main.dol");

        Directory.CreateDirectory(sysRootPath);
        Directory.CreateDirectory(filesRootPath);
        File.WriteAllBytes(Path.Combine(sysRootPath, "boot.bin"), new byte[GameCubeDiscSystemAreaWriter.BootBinSize]);
        File.WriteAllBytes(Path.Combine(sysRootPath, "bi2.bin"), new byte[GameCubeDiscSystemAreaWriter.Bi2BinSize]);
        File.WriteAllBytes(Path.Combine(sysRootPath, "apploader.img"), [0x41]);
        File.WriteAllBytes(mainDolPath, [0x44]);

        try {
            GameCubeDiscLayoutResult layout = new(
                discRootPath,
                mainDolPath,
                new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase));

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                new GameCubeRawImagePackager().Package(layout, Path.Combine(rootPath, "game.gcm"), CancellationToken.None));

            Assert.Contains("sys/fst.bin", exception.Message, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Reads one 4-byte-unit boot header field back into a byte offset or size.
    /// </summary>
    /// <param name="bootBinBytes">Serialized <c>boot.bin</c> bytes.</param>
    /// <param name="fieldOffset">Field byte offset inside <c>boot.bin</c>.</param>
    /// <returns>Decoded byte offset or size.</returns>
    static uint ReadBootField(byte[] bootBinBytes, int fieldOffset) {
        return BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(fieldOffset, sizeof(uint))) << 2;
    }

    /// <summary>
    /// Reads all file entries from one serialized GameCube <c>fst.bin</c> payload.
    /// </summary>
    /// <param name="fstBytes">Serialized <c>fst.bin</c> payload.</param>
    /// <returns>File entries keyed by slash-normalized relative path under <c>files/</c>.</returns>
    static IReadOnlyDictionary<string, GameCubeRawImageFileEntry> ReadFileEntries(byte[] fstBytes) {
        int rootEntryCount = checked((int)BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(8, sizeof(uint))));
        int stringTableOffset = rootEntryCount * GameCubeDiscFileSystemTableBuilder.EntrySize;
        Dictionary<string, GameCubeRawImageFileEntry> fileEntries = new(StringComparer.Ordinal);
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
            fileEntries[relativePath] = new GameCubeRawImageFileEntry(
                BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(entryOffset + 4, sizeof(uint))),
                BinaryPrimitives.ReadUInt32BigEndian(fstBytes.AsSpan(entryOffset + 8, sizeof(uint))));
        }

        return fileEntries;
    }

    /// <summary>
    /// Reads one null-terminated ASCII name from the FST string table.
    /// </summary>
    /// <param name="fstBytes">Serialized <c>fst.bin</c> payload.</param>
    /// <param name="offset">Starting offset inside the payload.</param>
    /// <returns>Decoded entry name.</returns>
    static string ReadName(byte[] fstBytes, int offset) {
        int endOffset = offset;
        while (endOffset < fstBytes.Length && fstBytes[endOffset] != 0) {
            endOffset++;
        }

        return System.Text.Encoding.ASCII.GetString(fstBytes, offset, endOffset - offset);
    }

    /// <summary>
    /// Resolves the nearest parent directory index for one file entry by scanning the directory entry ranges that contain it.
    /// </summary>
    /// <param name="fstBytes">Serialized <c>fst.bin</c> payload.</param>
    /// <param name="entryIndex">File entry index whose parent should be resolved.</param>
    /// <returns>Parent directory index.</returns>
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
    /// Combines two slash-normalized relative path segments.
    /// </summary>
    /// <param name="basePath">Current base path.</param>
    /// <param name="entryName">Child entry name.</param>
    /// <returns>Combined relative path.</returns>
    static string CombineRelativePath(string basePath, string entryName) {
        if (string.IsNullOrWhiteSpace(basePath)) {
            return entryName;
        }

        return basePath + "/" + entryName;
    }

    /// <summary>
    /// Reads one byte range from the packaged GameCube image without loading the entire sparse disc into memory.
    /// </summary>
    /// <param name="imagePath">Packaged image path to read.</param>
    /// <param name="offset">Starting byte offset.</param>
    /// <param name="length">Number of bytes to read.</param>
    /// <returns>Requested image byte range.</returns>
    static byte[] ReadImageRange(string imagePath, uint offset, int length) {
        using FileStream stream = File.OpenRead(imagePath);
        stream.Seek(offset, SeekOrigin.Begin);
        byte[] buffer = new byte[length];
        int totalBytesRead = 0;
        while (totalBytesRead < length) {
            int bytesRead = stream.Read(buffer, totalBytesRead, length - totalBytesRead);
            if (bytesRead == 0) {
                throw new EndOfStreamException($"Could not read {length} bytes from '{imagePath}' at offset {offset}.");
            }

            totalBytesRead += bytesRead;
        }

        return buffer;
    }
}
