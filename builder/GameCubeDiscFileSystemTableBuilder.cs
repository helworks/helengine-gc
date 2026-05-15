using System.Buffers.Binary;
using System.Text;

namespace helengine.gamecube.builder;

/// <summary>
/// Builds the GameCube <c>sys/fst.bin</c> payload staged into extracted retail-style disc roots.
/// </summary>
public sealed class GameCubeDiscFileSystemTableBuilder {
    /// <summary>
    /// Size in bytes of one serialized GameCube FST entry.
    /// </summary>
    public const int EntrySize = 12;

    /// <summary>
    /// Alignment in bytes required by the boot header fields that reference the serialized FST payload.
    /// </summary>
    public const int SizeAlignment = 4;

    /// <summary>
    /// Alignment in bytes used for packaged GameCube file payload placement after the serialized FST.
    /// </summary>
    public const uint FileDataAlignment = 0x8000;

    /// <summary>
    /// Builds the serialized <c>sys/fst.bin</c> payload for the supplied extracted <c>files/</c> tree and assigns final file offsets.
    /// </summary>
    /// <param name="filesRootPath">Extracted <c>files/</c> root that should be represented by the FST.</param>
    /// <param name="fstOffset">Final disc offset where the FST bytes will be staged.</param>
    /// <returns>Serialized FST payload ready to write to <c>sys/fst.bin</c>.</returns>
    public byte[] Build(string filesRootPath, uint fstOffset) {
        if (!Directory.Exists(filesRootPath)) {
            throw new DirectoryNotFoundException($"Extracted files root '{filesRootPath}' was not found.");
        }

        List<GameCubeDiscFileSystemEntryRecord> entries = [
            new GameCubeDiscFileSystemEntryRecord(string.Empty, string.Empty, true, 0)
        ];
        CollectEntries(filesRootPath, string.Empty, 0, entries);
        entries[0].EndIndex = entries.Count;

        int stringTableSize = PopulateNameOffsets(entries);
        int fstSize = (entries.Count * EntrySize) + stringTableSize;
        uint nextDataOffset = AlignFileData(fstOffset + (uint)fstSize);
        AssignFileOffsets(filesRootPath, entries, nextDataOffset);

        return Serialize(entries, stringTableSize);
    }

    /// <summary>
    /// Aligns one disc offset to the next 32-byte boundary used by GameCube file payloads.
    /// </summary>
    /// <param name="value">Disc offset to align.</param>
    /// <returns>Aligned disc offset.</returns>
    static uint Align32(uint value) {
        return (value + 31U) & ~31U;
    }

    /// <summary>
    /// Aligns one file payload offset to the 32 KiB boundary used by GameCube disc data placement.
    /// </summary>
    /// <param name="value">Disc offset to align.</param>
    /// <returns>Aligned disc offset.</returns>
    static uint AlignFileData(uint value) {
        return (value + (FileDataAlignment - 1U)) & ~(FileDataAlignment - 1U);
    }

    /// <summary>
    /// Aligns one serialized FST byte count to the next 4-byte boundary expected by boot-header size fields.
    /// </summary>
    /// <param name="value">Serialized byte count to align.</param>
    /// <returns>Aligned byte count.</returns>
    static int Align4(int value) {
        return (value + (SizeAlignment - 1)) & ~(SizeAlignment - 1);
    }

    /// <summary>
    /// Recursively appends directory and file entries in the same depth-first order used by the GameCube FST.
    /// </summary>
    /// <param name="currentDirectoryPath">Current physical directory path.</param>
    /// <param name="currentRelativePath">Current path relative to the extracted <c>files/</c> root.</param>
    /// <param name="parentIndex">Parent directory index used for directory entries.</param>
    /// <param name="entries">Mutable entry list receiving appended directory and file records.</param>
    static void CollectEntries(
        string currentDirectoryPath,
        string currentRelativePath,
        int parentIndex,
        List<GameCubeDiscFileSystemEntryRecord> entries) {
        string[] directoryPaths = Directory.GetDirectories(currentDirectoryPath);
        Array.Sort(directoryPaths, StringComparer.Ordinal);
        for (int directoryIndex = 0; directoryIndex < directoryPaths.Length; directoryIndex++) {
            string directoryPath = directoryPaths[directoryIndex];
            string directoryName = Path.GetFileName(directoryPath);
            string relativePath = CombineRelativePath(currentRelativePath, directoryName);
            GameCubeDiscFileSystemEntryRecord directoryEntry = new(directoryName, relativePath, true, parentIndex);
            int entryIndex = entries.Count;
            entries.Add(directoryEntry);
            CollectEntries(directoryPath, relativePath, entryIndex, entries);
            directoryEntry.EndIndex = entries.Count;
        }

        string[] filePaths = Directory.GetFiles(currentDirectoryPath);
        Array.Sort(filePaths, StringComparer.Ordinal);
        for (int fileIndex = 0; fileIndex < filePaths.Length; fileIndex++) {
            string filePath = filePaths[fileIndex];
            string fileName = Path.GetFileName(filePath);
            string relativePath = CombineRelativePath(currentRelativePath, fileName);
            FileInfo fileInfo = new(filePath);
            GameCubeDiscFileSystemEntryRecord fileEntry = new(fileName, relativePath, false, parentIndex) {
                DataSize = checked((uint)fileInfo.Length)
            };
            entries.Add(fileEntry);
        }
    }

    /// <summary>
    /// Combines two relative path segments using the slash form expected by FST entries.
    /// </summary>
    /// <param name="basePath">Base relative path.</param>
    /// <param name="childName">Child directory or file name.</param>
    /// <returns>Combined slash-normalized relative path.</returns>
    static string CombineRelativePath(string basePath, string childName) {
        if (string.IsNullOrWhiteSpace(basePath)) {
            return childName;
        }

        return basePath + "/" + childName;
    }

    /// <summary>
    /// Assigns byte offsets for every file payload after the serialized FST region.
    /// </summary>
    /// <param name="filesRootPath">Extracted <c>files/</c> root that holds the source payloads.</param>
    /// <param name="entries">Mutable FST entry list receiving final file offsets.</param>
    /// <param name="nextDataOffset">First aligned offset available after the FST bytes.</param>
    static void AssignFileOffsets(
        string filesRootPath,
        List<GameCubeDiscFileSystemEntryRecord> entries,
        uint nextDataOffset) {
        for (int entryIndex = 0; entryIndex < entries.Count; entryIndex++) {
            GameCubeDiscFileSystemEntryRecord entry = entries[entryIndex];
            if (entry.IsDirectory) {
                continue;
            }

            string sourcePath = Path.Combine(filesRootPath, entry.RelativePath.Replace('/', Path.DirectorySeparatorChar));
            FileInfo fileInfo = new(sourcePath);
            entry.DataSize = checked((uint)fileInfo.Length);
            entry.DataOffset = nextDataOffset;
            nextDataOffset = AlignFileData(nextDataOffset + entry.DataSize);
        }
    }

    /// <summary>
    /// Calculates name offsets inside the FST string table.
    /// </summary>
    /// <param name="entries">Mutable FST entry list receiving string-table offsets.</param>
    /// <returns>Total string-table byte size.</returns>
    static int PopulateNameOffsets(List<GameCubeDiscFileSystemEntryRecord> entries) {
        Encoding ascii = Encoding.ASCII;
        int nextNameOffset = 0;
        for (int entryIndex = 1; entryIndex < entries.Count; entryIndex++) {
            GameCubeDiscFileSystemEntryRecord entry = entries[entryIndex];
            entry.NameOffset = nextNameOffset;
            nextNameOffset += ascii.GetByteCount(entry.Name) + 1;
        }

        return nextNameOffset;
    }

    /// <summary>
    /// Serializes the computed entries and string table into one GameCube <c>sys/fst.bin</c> payload.
    /// </summary>
    /// <param name="entries">Fully populated FST entry list.</param>
    /// <param name="stringTableSize">Total string-table byte size.</param>
    /// <returns>Serialized FST payload.</returns>
    static byte[] Serialize(
        List<GameCubeDiscFileSystemEntryRecord> entries,
        int stringTableSize) {
        Encoding ascii = Encoding.ASCII;
        int entryTableSize = entries.Count * EntrySize;
        byte[] buffer = new byte[Align4(entryTableSize + stringTableSize)];
        for (int entryIndex = 0; entryIndex < entries.Count; entryIndex++) {
            GameCubeDiscFileSystemEntryRecord entry = entries[entryIndex];
            int entryOffset = entryIndex * EntrySize;
            buffer[entryOffset] = entry.IsDirectory ? (byte)1 : (byte)0;
            WriteUInt24BigEndian(buffer, entryOffset + 1, entry.NameOffset);
            if (entry.IsDirectory) {
                BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(entryOffset + 4, sizeof(uint)), checked((uint)entry.ParentIndex));
                BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(entryOffset + 8, sizeof(uint)), checked((uint)entry.EndIndex));
            } else {
                BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(entryOffset + 4, sizeof(uint)), entry.DataOffset);
                BinaryPrimitives.WriteUInt32BigEndian(buffer.AsSpan(entryOffset + 8, sizeof(uint)), entry.DataSize);
            }
        }

        for (int entryIndex = 1; entryIndex < entries.Count; entryIndex++) {
            GameCubeDiscFileSystemEntryRecord entry = entries[entryIndex];
            int stringOffset = entryTableSize + entry.NameOffset;
            int byteCount = ascii.GetByteCount(entry.Name);
            ascii.GetBytes(entry.Name, 0, entry.Name.Length, buffer, stringOffset);
            buffer[stringOffset + byteCount] = 0;
        }

        return buffer;
    }

    /// <summary>
    /// Writes one 24-bit big-endian integer into the serialized FST entry header.
    /// </summary>
    /// <param name="buffer">Destination serialized FST buffer.</param>
    /// <param name="offset">Byte offset to write.</param>
    /// <param name="value">24-bit value to encode.</param>
    static void WriteUInt24BigEndian(byte[] buffer, int offset, int value) {
        if (buffer == null) {
            throw new ArgumentNullException(nameof(buffer));
        } else if (value < 0 || value > 0x00FFFFFF) {
            throw new ArgumentOutOfRangeException(nameof(value), "FST name offsets must fit in 24 bits.");
        }

        buffer[offset] = (byte)((value >> 16) & 0xFF);
        buffer[offset + 1] = (byte)((value >> 8) & 0xFF);
        buffer[offset + 2] = (byte)(value & 0xFF);
    }
}
