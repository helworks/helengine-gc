namespace helengine.gamecube.builder.tests.Builders;

/// <summary>
/// Writes a fake packaged-mode DOL so builder tests can verify orchestration without Docker.
/// </summary>
public sealed class FakeGameCubeNativeBuildExecutor : IGameCubeNativeBuildExecutor {
    readonly bool WriteApploaderImage;
    readonly bool WriteGenericBootImage;

    /// <summary>
    /// Initializes the fake native build executor with optional packaged-disc artifact outputs.
    /// </summary>
    /// <param name="writeApploaderImage">Whether the fake native build should emit an apploader image artifact.</param>
    /// <param name="writeGenericBootImage">Whether the fake native build should emit a generic boot image header artifact.</param>
    public FakeGameCubeNativeBuildExecutor(
        bool writeApploaderImage = true,
        bool writeGenericBootImage = true) {
        WriteApploaderImage = writeApploaderImage;
        WriteGenericBootImage = writeGenericBootImage;
    }

    /// <summary>
    /// Writes a fake packaged-mode DOL into the requested workspace path.
    /// </summary>
    /// <param name="paths">Workspace paths that define the fake output destination.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the fake build cooperatively.</param>
    public void Build(GameCubeBuilderPaths paths, CancellationToken cancellationToken) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        cancellationToken.ThrowIfCancellationRequested();
        string nativeExecutableDirectoryPath = Path.GetDirectoryName(paths.NativeExecutablePath) ?? throw new InvalidOperationException("Native executable directory path could not be resolved.");
        string nativeApploaderDirectoryPath = Path.GetDirectoryName(paths.NativeApploaderImagePath) ?? throw new InvalidOperationException("Native apploader directory path could not be resolved.");
        string nativeGenericBootImageDirectoryPath = Path.GetDirectoryName(paths.NativeGenericBootImagePath) ?? throw new InvalidOperationException("Native generic boot image directory path could not be resolved.");
        Directory.CreateDirectory(nativeExecutableDirectoryPath);
        Directory.CreateDirectory(nativeApploaderDirectoryPath);
        Directory.CreateDirectory(nativeGenericBootImageDirectoryPath);
        File.WriteAllText(paths.NativeExecutablePath, "dol");
        if (WriteApploaderImage) {
            File.WriteAllText(paths.NativeApploaderImagePath, "apploader");
        }

        if (WriteGenericBootImage) {
            File.WriteAllText(paths.NativeGenericBootImagePath, "gbi");
        }
    }
}
