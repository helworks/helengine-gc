using System.Diagnostics;
using helengine.baseplatform.Builders;

namespace helengine.gamecube.builder;

/// <summary>
/// Runs an external GameCube build process while forwarding and retaining both redirected output streams.
/// </summary>
public sealed class GameCubeProcessOutputStreamer {
    /// <summary>
    /// Runs one configured process and forwards output as it becomes available.
    /// </summary>
    /// <param name="startInfo">Prepared process start info.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the process cooperatively.</param>
    /// <returns>Captured process result.</returns>
    public GameCubeProcessRunResult Run(ProcessStartInfo startInfo, CancellationToken cancellationToken) {
        if (startInfo == null) {
            throw new ArgumentNullException(nameof(startInfo));
        }

        NativeProcessRunResult result = new NativeProcessRunner().Run(startInfo, cancellationToken);
        return new GameCubeProcessRunResult(result.ExitCode, result.StandardOutput, result.StandardError);
    }
}
