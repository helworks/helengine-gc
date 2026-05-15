namespace helengine.gamecube.builder;

/// <summary>
/// Carries one parsed file entry from the staged GameCube <c>sys/fst.bin</c> payload while the raw disc image is being materialized.
/// </summary>
internal sealed class GameCubeRawImagePackagerFileEntry {
    /// <summary>
    /// Initializes one parsed raw-image file entry.
    /// </summary>
    /// <param name="relativePath">Slash-normalized path relative to the staged <c>files/</c> root.</param>
    /// <param name="dataOffset">Final disc byte offset where the payload must be written.</param>
    /// <param name="dataSize">Final payload size in bytes.</param>
    public GameCubeRawImagePackagerFileEntry(string relativePath, uint dataOffset, uint dataSize) {
        RelativePath = relativePath ?? throw new ArgumentNullException(nameof(relativePath));
        DataOffset = dataOffset;
        DataSize = dataSize;
    }

    /// <summary>
    /// Gets the slash-normalized path relative to the staged <c>files/</c> root.
    /// </summary>
    public string RelativePath { get; }

    /// <summary>
    /// Gets the final disc byte offset where the payload must be written.
    /// </summary>
    public uint DataOffset { get; }

    /// <summary>
    /// Gets the final payload size in bytes.
    /// </summary>
    public uint DataSize { get; }
}
