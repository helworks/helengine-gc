namespace helengine.gamecube.builder;

/// <summary>
/// Represents one staged scene or loose-asset entry written into the GameCube build manifest.
/// </summary>
public sealed class GameCubeBuildManifestEntry {
    /// <summary>
    /// Initializes one manifest entry.
    /// </summary>
    /// <param name="itemId">The staged item identifier.</param>
    /// <param name="sourceIdentity">The source identity from the build request.</param>
    /// <param name="outputPath">The output path written by the builder.</param>
    public GameCubeBuildManifestEntry(string itemId, string sourceIdentity, string outputPath) {
        if (string.IsNullOrWhiteSpace(itemId)) {
            throw new ArgumentException("Item id is required.", nameof(itemId));
        } else if (string.IsNullOrWhiteSpace(sourceIdentity)) {
            throw new ArgumentException("Source identity is required.", nameof(sourceIdentity));
        } else if (string.IsNullOrWhiteSpace(outputPath)) {
            throw new ArgumentException("Output path is required.", nameof(outputPath));
        }

        ItemId = itemId;
        SourceIdentity = sourceIdentity;
        OutputPath = outputPath;
    }

    /// <summary>
    /// Gets the staged item identifier.
    /// </summary>
    public string ItemId { get; }

    /// <summary>
    /// Gets the source identity recorded in the build request.
    /// </summary>
    public string SourceIdentity { get; }

    /// <summary>
    /// Gets the output path written by the builder.
    /// </summary>
    public string OutputPath { get; }
}
