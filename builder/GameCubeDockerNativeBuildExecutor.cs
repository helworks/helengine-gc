using System.Diagnostics;

namespace helengine.gamecube.builder;

/// <summary>
/// Invokes the Dockerized GameCube native build with packaged-disc boot mode enabled.
/// </summary>
public sealed class GameCubeDockerNativeBuildExecutor : IGameCubeNativeBuildExecutor {
    /// <summary>
    /// Normalizes regenerated GameCube core sources that still need packaged-runtime compatibility rewrites before compilation.
    /// </summary>
    readonly GameCubeGeneratedCoreCompatibilityNormalizer GeneratedCoreCompatibilityNormalizer = new();

    /// <summary>
    /// Builds the packaged-mode native DOL and stages it into the builder workspace.
    /// </summary>
    /// <param name="paths">Workspace paths that define the packaged build inputs and outputs.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the native build cooperatively.</param>
    public void Build(GameCubeBuilderPaths paths, CancellationToken cancellationToken) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        GeneratedCoreCompatibilityNormalizer.Normalize(paths.GeneratedCoreRootPath);

        ProcessStartInfo startInfo = new() {
            FileName = "rtk",
            WorkingDirectory = paths.RepositoryRootPath,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        startInfo.ArgumentList.Add("docker");
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--rm");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(paths.RepositoryRootPath + ":/workspace");
        startInfo.ArgumentList.Add("-w");
        startInfo.ArgumentList.Add("/workspace");
        startInfo.ArgumentList.Add("-e");
        startInfo.ArgumentList.Add("HELENGINE_CORE_CPP_ROOT=/workspace/" + paths.GeneratedCoreRelativePath);
        startInfo.ArgumentList.Add("-e");
        startInfo.ArgumentList.Add("HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc");
        startInfo.ArgumentList.Add("helengine-gc");
        startInfo.ArgumentList.Add("make");
        startInfo.ArgumentList.Add("clean");
        startInfo.ArgumentList.Add("all");
        startInfo.ArgumentList.Add("packaged-disc-assets");

        using Process process = Process.Start(startInfo) ?? throw new InvalidOperationException("Could not start the GameCube Docker build process.");
        process.WaitForExit();
        cancellationToken.ThrowIfCancellationRequested();

        if (process.ExitCode != 0) {
            string standardOutput = process.StandardOutput.ReadToEnd();
            string standardError = process.StandardError.ReadToEnd();
            throw new InvalidOperationException(
                "GameCube native packaged-disc build failed."
                + Environment.NewLine
                + standardOutput
                + Environment.NewLine
                + standardError);
        }

        string builtDolPath = Path.Combine(paths.RepositoryRootPath, "build", "helengine_gc.dol");
        if (!File.Exists(builtDolPath)) {
            throw new FileNotFoundException("The packaged native GameCube DOL was not produced by the Docker build.", builtDolPath);
        }

        string nativeExecutableDirectoryPath = Path.GetDirectoryName(paths.NativeExecutablePath) ?? throw new InvalidOperationException("Native executable directory path could not be resolved.");
        Directory.CreateDirectory(nativeExecutableDirectoryPath);
        File.Copy(builtDolPath, paths.NativeExecutablePath, true);

        string builtApploaderPath = Path.Combine(paths.RepositoryRootPath, "build", "packaged-disc", "apploader.img");
        if (!File.Exists(builtApploaderPath)) {
            throw new FileNotFoundException("The packaged native GameCube apploader image was not produced by the Docker build.", builtApploaderPath);
        }

        string nativeApploaderDirectoryPath = Path.GetDirectoryName(paths.NativeApploaderImagePath) ?? throw new InvalidOperationException("Native apploader directory path could not be resolved.");
        Directory.CreateDirectory(nativeApploaderDirectoryPath);
        File.Copy(builtApploaderPath, paths.NativeApploaderImagePath, true);

        string builtGenericBootImagePath = Path.Combine(paths.RepositoryRootPath, "build", "packaged-disc", "gbi.hdr");
        if (!File.Exists(builtGenericBootImagePath)) {
            throw new FileNotFoundException("The packaged native GameCube generic boot image header was not produced by the Docker build.", builtGenericBootImagePath);
        }

        string nativeGenericBootImageDirectoryPath = Path.GetDirectoryName(paths.NativeGenericBootImagePath) ?? throw new InvalidOperationException("Native generic boot image directory path could not be resolved.");
        Directory.CreateDirectory(nativeGenericBootImageDirectoryPath);
        File.Copy(builtGenericBootImagePath, paths.NativeGenericBootImagePath, true);
    }
}
