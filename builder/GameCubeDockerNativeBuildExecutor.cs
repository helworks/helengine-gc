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

        ProcessStartInfo startInfo = CreateStartInfo(paths);

        using Process process = Process.Start(startInfo) ?? throw new InvalidOperationException("Could not start the GameCube Docker build process.");
        Task<string> standardOutputTask = process.StandardOutput.ReadToEndAsync();
        Task<string> standardErrorTask = process.StandardError.ReadToEndAsync();
        while (!process.HasExited) {
            cancellationToken.ThrowIfCancellationRequested();
            process.WaitForExit(100);
        }

        process.WaitForExit();
        Task.WaitAll(standardOutputTask, standardErrorTask);

        if (process.ExitCode != 0) {
            string standardOutput = standardOutputTask.Result;
            string standardError = standardErrorTask.Result;
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

    /// <summary>
    /// Creates the Docker process start info for one packaged native GameCube build.
    /// </summary>
    /// <param name="paths">Workspace paths that define the packaged build inputs and outputs.</param>
    /// <returns>Configured Docker process start info.</returns>
    static ProcessStartInfo CreateStartInfo(GameCubeBuilderPaths paths) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        ProcessStartInfo startInfo = new() {
            FileName = "rtk",
            WorkingDirectory = paths.RepositoryRootPath,
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };

        string generatedCoreContainerPath = "/workspace/" + paths.GeneratedCoreRelativePath;
        startInfo.ArgumentList.Add("docker");
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--rm");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(paths.RepositoryRootPath + ":/workspace");
        if (!IsPathUnderRoot(paths.GeneratedCoreRootPath, paths.RepositoryRootPath)) {
            generatedCoreContainerPath = "/helengine-generated-core";
            startInfo.ArgumentList.Add("-v");
            startInfo.ArgumentList.Add(paths.GeneratedCoreRootPath + ":" + generatedCoreContainerPath);
        }

        startInfo.ArgumentList.Add("-w");
        startInfo.ArgumentList.Add("/workspace");
        startInfo.ArgumentList.Add("-e");
        startInfo.ArgumentList.Add("HELENGINE_CORE_CPP_ROOT=" + generatedCoreContainerPath);
        startInfo.ArgumentList.Add("-e");
        startInfo.ArgumentList.Add("HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc");
        startInfo.ArgumentList.Add("helengine-gc");
        startInfo.ArgumentList.Add("make");
        startInfo.ArgumentList.Add("clean");
        startInfo.ArgumentList.Add("all");
        startInfo.ArgumentList.Add("packaged-disc-assets");
        return startInfo;
    }

    /// <summary>
    /// Returns true when one filesystem path is inside the supplied root path.
    /// </summary>
    /// <param name="candidatePath">Candidate filesystem path.</param>
    /// <param name="rootPath">Containing filesystem root path.</param>
    /// <returns>True when the candidate path is inside the supplied root path.</returns>
    static bool IsPathUnderRoot(string candidatePath, string rootPath) {
        if (string.IsNullOrWhiteSpace(candidatePath)) {
            throw new ArgumentException("Candidate path is required.", nameof(candidatePath));
        }
        if (string.IsNullOrWhiteSpace(rootPath)) {
            throw new ArgumentException("Root path is required.", nameof(rootPath));
        }

        string fullCandidatePath = Path.GetFullPath(candidatePath);
        string fullRootPath = Path.GetFullPath(rootPath);
        string relativePath = Path.GetRelativePath(fullRootPath, fullCandidatePath);
        return !relativePath.StartsWith("..", StringComparison.Ordinal) && !Path.IsPathRooted(relativePath);
    }
}
