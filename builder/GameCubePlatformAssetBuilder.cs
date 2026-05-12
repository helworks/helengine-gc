using helengine.baseplatform.Builders;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Descriptors;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;

namespace helengine.gamecube.builder;

/// <summary>
/// Implements the GameCube platform asset builder contract.
/// </summary>
public sealed class GameCubePlatformAssetBuilder : IPlatformAssetBuilder {
    /// <summary>
    /// Initializes one GameCube builder instance with the current platform metadata.
    /// </summary>
    public GameCubePlatformAssetBuilder() {
        Descriptor = new PlatformBuilderDescriptor(
            "helengine.gamecube.builder",
            "1.0.0",
            "gamecube",
            new EngineCompatibilityRange("1.0.0", "999.0.0"),
            new ManifestCompatibilityRange(1, 1),
            ["gamecube"],
            ["gamecube-default"]);
        Definition = GameCubePlatformDefinitionFactory.Create();
    }

    /// <summary>
    /// Gets the explicit builder descriptor for the GameCube builder assembly.
    /// </summary>
    public PlatformBuilderDescriptor Descriptor { get; }

    /// <summary>
    /// Gets the typed GameCube platform definition exposed to the editor.
    /// </summary>
    public PlatformDefinition Definition { get; }

    /// <summary>
    /// Executes one GameCube build request through the staged payload workspace.
    /// </summary>
    /// <param name="request">The resolved build request.</param>
    /// <param name="progressReporter">The progress reporter.</param>
    /// <param name="diagnosticReporter">The diagnostic reporter.</param>
    /// <param name="cancellationToken">The cancellation token.</param>
    /// <returns>The final build report.</returns>
    public Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        return GameCubeBuildWorkspace.BuildAsync(request, progressReporter, diagnosticReporter, cancellationToken);
    }
}
