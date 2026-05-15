using System.Diagnostics;

namespace helengine.gamecube.builder;

/// <summary>
/// Runs real external processes for the GameCube builder.
/// </summary>
public sealed class GameCubeProcessRunner : IGameCubeProcessRunner {
    /// <summary>
    /// Runs one configured process and captures exit code, stdout, and stderr.
    /// </summary>
    /// <param name="startInfo">Prepared process start info.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the process cooperatively.</param>
    /// <returns>Captured process result.</returns>
    public GameCubeProcessRunResult Run(ProcessStartInfo startInfo, CancellationToken cancellationToken) {
        if (startInfo == null) {
            throw new ArgumentNullException(nameof(startInfo));
        }

        startInfo.RedirectStandardOutput = true;
        startInfo.RedirectStandardError = true;

        using Process process = Process.Start(startInfo) ?? throw new InvalidOperationException("Could not start external GameCube builder process.");
        process.WaitForExit();
        cancellationToken.ThrowIfCancellationRequested();
        return new GameCubeProcessRunResult(process.ExitCode, process.StandardOutput.ReadToEnd(), process.StandardError.ReadToEnd());
    }
}
