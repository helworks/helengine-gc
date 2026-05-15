using System.Diagnostics;

namespace helengine.gamecube.builder;

/// <summary>
/// Runs external processes used by the GameCube builder and captures their outputs.
/// </summary>
public interface IGameCubeProcessRunner {
    /// <summary>
    /// Runs one configured process and returns its captured output.
    /// </summary>
    /// <param name="startInfo">Prepared process start info.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the process cooperatively.</param>
    /// <returns>Captured process result.</returns>
    GameCubeProcessRunResult Run(ProcessStartInfo startInfo, CancellationToken cancellationToken);
}
