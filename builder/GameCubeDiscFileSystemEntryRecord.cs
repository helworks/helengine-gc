namespace helengine.gamecube.builder;

/// <summary>
/// Carries one serialized GameCube disc file-system table entry while the extracted <c>files/</c> tree is converted into <c>sys/fst.bin</c>.
/// </summary>
public sealed class GameCubeDiscFileSystemEntryRecord {
    /// <summary>
    /// Initializes one GameCube disc file-system table entry record.
    /// </summary>
    /// <param name="name">Entry name stored in the FST string table.</param>
    /// <param name="relativePath">Path relative to the extracted <c>files/</c> root.</param>
    /// <param name="isDirectory">Whether the entry represents a directory node.</param>
    /// <param name="parentIndex">Parent directory index used by directory entries.</param>
    public GameCubeDiscFileSystemEntryRecord(
        string name,
        string relativePath,
        bool isDirectory,
        int parentIndex) {
        Name = name ?? throw new ArgumentNullException(nameof(name));
        RelativePath = relativePath ?? throw new ArgumentNullException(nameof(relativePath));
        IsDirectory = isDirectory;
        ParentIndex = parentIndex;
    }

    /// <summary>
    /// Gets the entry name stored in the FST string table.
    /// </summary>
    public string Name { get; }

    /// <summary>
    /// Gets the path relative to the extracted <c>files/</c> root.
    /// </summary>
    public string RelativePath { get; }

    /// <summary>
    /// Gets whether the entry represents a directory node.
    /// </summary>
    public bool IsDirectory { get; }

    /// <summary>
    /// Gets the parent directory index used by directory entries.
    /// </summary>
    public int ParentIndex { get; }

    /// <summary>
    /// Gets or sets the directory end index written into directory entries.
    /// </summary>
    public int EndIndex { get; set; }

    /// <summary>
    /// Gets or sets the string-table byte offset for the entry name.
    /// </summary>
    public int NameOffset { get; set; }

    /// <summary>
    /// Gets or sets the staged file offset written into file entries.
    /// </summary>
    public uint DataOffset { get; set; }

    /// <summary>
    /// Gets or sets the staged file size written into file entries.
    /// </summary>
    public uint DataSize { get; set; }
}
