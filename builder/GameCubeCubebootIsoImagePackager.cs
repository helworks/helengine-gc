using System.Diagnostics;

namespace helengine.gamecube.builder;

/// <summary>
/// Packages one GameCube image by staging a cubeboot-compatible ISO filesystem and invoking ISO authoring inside the GameCube Docker toolchain.
/// </summary>
public sealed class GameCubeCubebootIsoImagePackager : IGameCubeImagePackager {
    readonly GameCubeCubebootIsoImagePackagerOptions Options;
    readonly IGameCubeProcessRunner ProcessRunner;
    readonly Action<string, string> SuccessImageWriter;

    /// <summary>
    /// Initializes one cubeboot ISO image packager.
    /// </summary>
    /// <param name="options">Explicit cubeboot ISO packager configuration.</param>
    /// <param name="processRunner">Process runner used to invoke Dockerized ISO authoring.</param>
    /// <param name="successImageWriter">Optional test hook that can materialize the expected image artifact after a successful process run.</param>
    public GameCubeCubebootIsoImagePackager(
        GameCubeCubebootIsoImagePackagerOptions options,
        IGameCubeProcessRunner processRunner,
        Action<string, string> successImageWriter = null) {
        Options = options ?? throw new ArgumentNullException(nameof(options));
        ProcessRunner = processRunner ?? throw new ArgumentNullException(nameof(processRunner));
        SuccessImageWriter = successImageWriter;
    }

    /// <summary>
    /// Packages one extracted GameCube disc layout into the final output image path.
    /// </summary>
    /// <param name="layout">Extracted disc layout that should be packaged.</param>
    /// <param name="outputImagePath">Final GameCube image path to write.</param>
    /// <param name="cancellationToken">Cancellation token that can stop packaging cooperatively.</param>
    public void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken) {
        if (layout == null) {
            throw new ArgumentNullException(nameof(layout));
        } else if (string.IsNullOrWhiteSpace(outputImagePath)) {
            throw new ArgumentException("Output image path is required.", nameof(outputImagePath));
        } else if (!File.Exists(Options.GenericBootImagePath)) {
            throw new FileNotFoundException("Generated GameCube generic boot image header was not found.", Options.GenericBootImagePath);
        } else if (!File.Exists(layout.DiscExecutablePath)) {
            throw new FileNotFoundException("The extracted GameCube layout is missing sys/main.dol.", layout.DiscExecutablePath);
        }

        string stagedFilesRootPath = Path.Combine(layout.DiscRootPath, "files");
        if (!Directory.Exists(stagedFilesRootPath)) {
            throw new InvalidOperationException("The extracted GameCube disc layout is missing the files/ payload directory.");
        }

        string outputDirectoryPath = Path.GetDirectoryName(outputImagePath) ?? throw new InvalidOperationException("Output image directory path could not be resolved.");
        Directory.CreateDirectory(outputDirectoryPath);

        string isoAuthoringRootPath = Path.Combine(outputDirectoryPath, ".cubeboot-iso-root");
        StageCubebootIsoFilesystem(isoAuthoringRootPath, stagedFilesRootPath, layout.DiscExecutablePath, Options.GenericBootImagePath);

        ProcessStartInfo startInfo = CreateProcessStartInfo(isoAuthoringRootPath, outputImagePath);
        GameCubeProcessRunResult result = ProcessRunner.Run(startInfo, cancellationToken);
        if (result.ExitCode != 0) {
            throw new InvalidOperationException(
                $"GameCube cubeboot ISO authoring failed with exit code {result.ExitCode}.{Environment.NewLine}{result.StandardOutput}{Environment.NewLine}{result.StandardError}");
        }

        if (SuccessImageWriter != null) {
            SuccessImageWriter(isoAuthoringRootPath, outputImagePath);
        }

        if (!File.Exists(outputImagePath)) {
            throw new InvalidOperationException("GameCube cubeboot ISO authoring reported success but the image artifact was not created.");
        }
    }

    /// <summary>
    /// Stages the ISO filesystem expected by the cubeboot generic boot image path.
    /// </summary>
    /// <param name="isoAuthoringRootPath">Destination ISO authoring root.</param>
    /// <param name="stagedFilesRootPath">Source files directory from the extracted layout.</param>
    /// <param name="bootExecutablePath">Boot DOL that should be visible as <c>bootldr.dol</c> in the ISO root.</param>
    /// <param name="genericBootImagePath">Generated generic boot image header staged beside the filesystem root.</param>
    static void StageCubebootIsoFilesystem(
        string isoAuthoringRootPath,
        string stagedFilesRootPath,
        string bootExecutablePath,
        string genericBootImagePath) {
        if (Directory.Exists(isoAuthoringRootPath)) {
            Directory.Delete(isoAuthoringRootPath, recursive: true);
        }

        Directory.CreateDirectory(isoAuthoringRootPath);
        CopyDirectory(stagedFilesRootPath, Path.Combine(isoAuthoringRootPath, "files"));
        File.Copy(bootExecutablePath, Path.Combine(isoAuthoringRootPath, "bootldr.dol"), true);
        File.Copy(genericBootImagePath, Path.Combine(isoAuthoringRootPath, "gbi.hdr"), true);
    }

    /// <summary>
    /// Creates the Dockerized ISO authoring process start info for one staged cubeboot filesystem root.
    /// </summary>
    /// <param name="isoAuthoringRootPath">Staged ISO authoring root that contains <c>bootldr.dol</c>, <c>files/</c>, and <c>gbi.hdr</c>.</param>
    /// <param name="outputImagePath">Final image path that should be written.</param>
    /// <returns>Prepared process start info.</returns>
    ProcessStartInfo CreateProcessStartInfo(string isoAuthoringRootPath, string outputImagePath) {
        string outputDirectoryPath = Path.GetDirectoryName(outputImagePath) ?? throw new InvalidOperationException("Output image directory path could not be resolved.");
        string outputFileName = Path.GetFileName(outputImagePath);

        ProcessStartInfo startInfo = new() {
            FileName = "rtk",
            WorkingDirectory = Options.RepositoryRootPath
        };
        startInfo.ArgumentList.Add("docker");
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--rm");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(isoAuthoringRootPath + ":/input");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(outputDirectoryPath + ":/output");
        startInfo.ArgumentList.Add("-w");
        startInfo.ArgumentList.Add("/input");
        startInfo.ArgumentList.Add(Options.DockerImageName);
        startInfo.ArgumentList.Add("xorriso");
        startInfo.ArgumentList.Add("-as");
        startInfo.ArgumentList.Add("mkisofs");
        startInfo.ArgumentList.Add("-R");
        startInfo.ArgumentList.Add("-J");
        startInfo.ArgumentList.Add("-G");
        startInfo.ArgumentList.Add("/input/gbi.hdr");
        startInfo.ArgumentList.Add("-no-emul-boot");
        startInfo.ArgumentList.Add("-boot-load-size");
        startInfo.ArgumentList.Add("4");
        startInfo.ArgumentList.Add("-b");
        startInfo.ArgumentList.Add("bootldr.dol");
        startInfo.ArgumentList.Add("-o");
        startInfo.ArgumentList.Add("/output/" + outputFileName);
        startInfo.ArgumentList.Add("/input");
        return startInfo;
    }

    /// <summary>
    /// Recursively copies one directory tree into the destination path.
    /// </summary>
    /// <param name="sourceRootPath">Source directory tree to copy.</param>
    /// <param name="destinationRootPath">Destination directory tree.</param>
    static void CopyDirectory(string sourceRootPath, string destinationRootPath) {
        Directory.CreateDirectory(destinationRootPath);

        string[] directoryPaths = Directory.GetDirectories(sourceRootPath, "*", SearchOption.AllDirectories);
        for (int directoryIndex = 0; directoryIndex < directoryPaths.Length; directoryIndex++) {
            string sourceDirectoryPath = directoryPaths[directoryIndex];
            string relativePath = Path.GetRelativePath(sourceRootPath, sourceDirectoryPath);
            Directory.CreateDirectory(Path.Combine(destinationRootPath, relativePath));
        }

        string[] filePaths = Directory.GetFiles(sourceRootPath, "*", SearchOption.AllDirectories);
        for (int fileIndex = 0; fileIndex < filePaths.Length; fileIndex++) {
            string sourceFilePath = filePaths[fileIndex];
            string relativePath = Path.GetRelativePath(sourceRootPath, sourceFilePath);
            string destinationFilePath = Path.Combine(destinationRootPath, relativePath);
            string destinationDirectoryPath = Path.GetDirectoryName(destinationFilePath) ?? throw new InvalidOperationException("Destination file directory path could not be resolved.");
            Directory.CreateDirectory(destinationDirectoryPath);
            File.Copy(sourceFilePath, destinationFilePath, true);
        }
    }
}
