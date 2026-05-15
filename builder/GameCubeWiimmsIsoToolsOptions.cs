namespace helengine.gamecube.builder;

/// <summary>
/// Carries the explicit Wiimms ISO Tools executable path used by packaged GameCube image builds.
/// </summary>
public sealed class GameCubeWiimmsIsoToolsOptions {
    /// <summary>
    /// Initializes one explicit Wiimms ISO Tools option set.
    /// </summary>
    /// <param name="executablePath">Configured path to the <c>wit</c> executable.</param>
    public GameCubeWiimmsIsoToolsOptions(string executablePath) {
        ExecutablePath = executablePath ?? string.Empty;
    }

    /// <summary>
    /// Gets the configured path to the <c>wit</c> executable.
    /// </summary>
    public string ExecutablePath { get; }
}
