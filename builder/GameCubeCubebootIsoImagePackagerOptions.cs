namespace helengine.gamecube.builder;

/// <summary>
/// Carries the explicit inputs required to package one GameCube ISO image through the cubeboot ISO authoring path.
/// </summary>
public sealed class GameCubeCubebootIsoImagePackagerOptions {
    /// <summary>
    /// Initializes one cubeboot ISO packager option set.
    /// </summary>
    /// <param name="repositoryRootPath">Repository root used as the Docker mount source.</param>
    /// <param name="genericBootImagePath">Path to the generated generic boot image header.</param>
    /// <param name="dockerImageName">Docker image name that contains the ISO authoring toolchain.</param>
    public GameCubeCubebootIsoImagePackagerOptions(
        string repositoryRootPath,
        string genericBootImagePath,
        string dockerImageName) {
        if (string.IsNullOrWhiteSpace(repositoryRootPath)) {
            throw new ArgumentException("Repository root path is required.", nameof(repositoryRootPath));
        } else if (string.IsNullOrWhiteSpace(genericBootImagePath)) {
            throw new ArgumentException("Generic boot image path is required.", nameof(genericBootImagePath));
        } else if (string.IsNullOrWhiteSpace(dockerImageName)) {
            throw new ArgumentException("Docker image name is required.", nameof(dockerImageName));
        }

        RepositoryRootPath = repositoryRootPath;
        GenericBootImagePath = genericBootImagePath;
        DockerImageName = dockerImageName;
    }

    /// <summary>
    /// Gets the repository root used as the Docker mount source.
    /// </summary>
    public string RepositoryRootPath { get; }

    /// <summary>
    /// Gets the path to the generated generic boot image header.
    /// </summary>
    public string GenericBootImagePath { get; }

    /// <summary>
    /// Gets the Docker image name that contains the ISO authoring toolchain.
    /// </summary>
    public string DockerImageName { get; }
}
