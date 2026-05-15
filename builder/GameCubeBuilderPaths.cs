using helengine.baseplatform.Requests;

namespace helengine.gamecube.builder;

/// <summary>
/// Centralizes the filesystem paths used by one packaged GameCube build invocation.
/// </summary>
public sealed class GameCubeBuilderPaths {
    /// <summary>
    /// Creates one packaged-build path set from a resolved GameCube build request.
    /// </summary>
    /// <param name="request">Resolved build request that owns the packaged build paths.</param>
    /// <returns>Packaged-build path set for the supplied request.</returns>
    public static GameCubeBuilderPaths Create(PlatformBuildRequest request) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        }

        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        return new GameCubeBuilderPaths(
            repositoryRootPath,
            request.GeneratedCoreCppRootPath,
            Path.Combine(request.WorkingRoot, "staged-content"),
            Path.Combine(request.OutputRoot, "disc"),
            Path.Combine(request.OutputRoot, "game.gcm"),
            Path.Combine(request.OutputRoot, "native", "helengine_gc.dol"),
            Path.Combine(request.OutputRoot, "native", "apploader.img"),
            Path.Combine(request.OutputRoot, "native", "gbi.hdr"));
    }

    /// <summary>
    /// Initializes one packaged-build path set.
    /// </summary>
    /// <param name="repositoryRootPath">Repository root that contains the native GameCube project files.</param>
    /// <param name="generatedCoreRootPath">Generated core root that receives packaged runtime manifest files.</param>
    /// <param name="stagingRootPath">Working staging root that receives cooked artifacts before disc layout.</param>
    /// <param name="discRootPath">Extracted disc root written for inspection and image packaging.</param>
    /// <param name="discImagePath">Final GameCube disc-image output path.</param>
    /// <param name="nativeExecutablePath">Packaged-mode native DOL output path staged by the builder.</param>
    /// <param name="nativeApploaderImagePath">Packaged-mode GameCube apploader image output path staged by the builder.</param>
    /// <param name="nativeGenericBootImagePath">Packaged-mode GameCube generic boot image header staged by the builder.</param>
    public GameCubeBuilderPaths(
        string repositoryRootPath,
        string generatedCoreRootPath,
        string stagingRootPath,
        string discRootPath,
        string discImagePath,
        string nativeExecutablePath,
        string nativeApploaderImagePath,
        string nativeGenericBootImagePath) {
        RepositoryRootPath = string.IsNullOrWhiteSpace(repositoryRootPath)
            ? throw new ArgumentException("Repository root path is required.", nameof(repositoryRootPath))
            : repositoryRootPath;
        GeneratedCoreRootPath = string.IsNullOrWhiteSpace(generatedCoreRootPath)
            ? throw new ArgumentException("Generated core root path is required.", nameof(generatedCoreRootPath))
            : generatedCoreRootPath;
        StagingRootPath = string.IsNullOrWhiteSpace(stagingRootPath)
            ? throw new ArgumentException("Staging root path is required.", nameof(stagingRootPath))
            : stagingRootPath;
        DiscRootPath = string.IsNullOrWhiteSpace(discRootPath)
            ? throw new ArgumentException("Disc root path is required.", nameof(discRootPath))
            : discRootPath;
        DiscImagePath = string.IsNullOrWhiteSpace(discImagePath)
            ? throw new ArgumentException("Disc image path is required.", nameof(discImagePath))
            : discImagePath;
        NativeExecutablePath = string.IsNullOrWhiteSpace(nativeExecutablePath)
            ? throw new ArgumentException("Native executable path is required.", nameof(nativeExecutablePath))
            : nativeExecutablePath;
        NativeApploaderImagePath = string.IsNullOrWhiteSpace(nativeApploaderImagePath)
            ? throw new ArgumentException("Native apploader image path is required.", nameof(nativeApploaderImagePath))
            : nativeApploaderImagePath;
        NativeGenericBootImagePath = string.IsNullOrWhiteSpace(nativeGenericBootImagePath)
            ? throw new ArgumentException("Native generic boot image path is required.", nameof(nativeGenericBootImagePath))
            : nativeGenericBootImagePath;
    }

    /// <summary>
    /// Gets the repository root that contains the native GameCube project files.
    /// </summary>
    public string RepositoryRootPath { get; }

    /// <summary>
    /// Gets the generated core root that receives packaged runtime manifest files.
    /// </summary>
    public string GeneratedCoreRootPath { get; }

    /// <summary>
    /// Gets the working staging root that receives cooked artifacts before disc layout.
    /// </summary>
    public string StagingRootPath { get; }

    /// <summary>
    /// Gets the extracted disc root written for inspection and image packaging.
    /// </summary>
    public string DiscRootPath { get; }

    /// <summary>
    /// Gets the final GameCube disc-image output path.
    /// </summary>
    public string DiscImagePath { get; }

    /// <summary>
    /// Gets the packaged-mode native DOL output path staged by the builder.
    /// </summary>
    public string NativeExecutablePath { get; }

    /// <summary>
    /// Gets the packaged-mode GameCube apploader image output path staged by the builder.
    /// </summary>
    public string NativeApploaderImagePath { get; }

    /// <summary>
    /// Gets the packaged-mode GameCube generic boot image header output path staged by the builder.
    /// </summary>
    public string NativeGenericBootImagePath { get; }

    /// <summary>
    /// Gets the generated-core root relative to the repository root for Docker path mapping.
    /// </summary>
    public string GeneratedCoreRelativePath => Path.GetRelativePath(RepositoryRootPath, GeneratedCoreRootPath).Replace('\\', '/');
}
