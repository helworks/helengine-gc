namespace helengine.gamecube.builder.tests;

/// <summary>
/// Carries one parsed file entry from a serialized GameCube <c>fst.bin</c> payload.
/// </summary>
public sealed class GameCubeRawImageFileEntry {
    /// <summary>
    /// Initializes one parsed GameCube raw-image file entry.
    /// </summary>
    /// <param name="dataOffset">Final disc byte offset for the file payload.</param>
    /// <param name="dataSize">Final file payload size in bytes.</param>
    public GameCubeRawImageFileEntry(uint dataOffset, uint dataSize) {
        DataOffset = dataOffset;
        DataSize = dataSize;
    }

    /// <summary>
    /// Gets the final disc byte offset for the file payload.
    /// </summary>
    public uint DataOffset { get; }

    /// <summary>
    /// Gets the final file payload size in bytes.
    /// </summary>
    public uint DataSize { get; }
}
