namespace helengine.gamecube.builder;

/// <summary>
/// Packages one extracted GameCube disc layout into a final disc-image artifact.
/// </summary>
public interface IGameCubeImagePackager {
    /// <summary>
    /// Packages one extracted GameCube disc layout into the final output image path.
    /// </summary>
    /// <param name="layout">Extracted disc layout that should be packaged.</param>
    /// <param name="outputImagePath">Final GameCube image path to write.</param>
    /// <param name="cancellationToken">Cancellation token that can stop packaging cooperatively.</param>
    void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken);
}
