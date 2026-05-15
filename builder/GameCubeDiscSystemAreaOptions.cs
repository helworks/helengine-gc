namespace helengine.gamecube.builder;

/// <summary>
/// Carries the explicit system-area inputs required to stage a valid extracted GameCube disc root for image packaging.
/// </summary>
public sealed class GameCubeDiscSystemAreaOptions {
    /// <summary>
    /// Initializes one GameCube disc system-area option set.
    /// </summary>
    /// <param name="apploaderImagePath">Path to the apploader image that should be staged as <c>sys/apploader.img</c>.</param>
    /// <param name="discId">Six-character GameCube disc identifier written into <c>boot.bin</c>.</param>
    /// <param name="discTitle">Disc title written into <c>boot.bin</c>.</param>
    public GameCubeDiscSystemAreaOptions(
        string apploaderImagePath,
        string discId,
        string discTitle) {
        if (string.IsNullOrWhiteSpace(apploaderImagePath)) {
            throw new ArgumentException("Apploader image path is required.", nameof(apploaderImagePath));
        } else if (string.IsNullOrWhiteSpace(discId)) {
            throw new ArgumentException("Disc id is required.", nameof(discId));
        } else if (string.IsNullOrWhiteSpace(discTitle)) {
            throw new ArgumentException("Disc title is required.", nameof(discTitle));
        }

        ApploaderImagePath = apploaderImagePath;
        DiscId = discId;
        DiscTitle = discTitle;
    }

    /// <summary>
    /// Gets the path to the apploader image that should be staged as <c>sys/apploader.img</c>.
    /// </summary>
    public string ApploaderImagePath { get; }

    /// <summary>
    /// Gets the six-character GameCube disc identifier written into <c>boot.bin</c>.
    /// </summary>
    public string DiscId { get; }

    /// <summary>
    /// Gets the disc title written into <c>boot.bin</c>.
    /// </summary>
    public string DiscTitle { get; }

}
