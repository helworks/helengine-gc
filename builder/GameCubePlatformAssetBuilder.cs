using helengine.baseplatform.Builders;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Descriptors;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Results;

namespace helengine.gamecube.builder;

/// <summary>
/// Implements the GameCube platform asset builder contract.
/// </summary>
public sealed class GameCubePlatformAssetBuilder : IPlatformAssetBuilder {
    /// <summary>
    /// Runtime specialization id that selects the packaged-disc GameCube build flow.
    /// </summary>
    const string PackagedDiscRuntimeSpecializationId = "gamecube-disc-layout";

    /// <summary>
    /// Native build executor used when the request selects the packaged-disc flow.
    /// </summary>
    readonly IGameCubeNativeBuildExecutor NativeBuildExecutor;

    /// <summary>
    /// Material cooker that translates authored GameCube material schemas into platform-owned cooked payloads.
    /// </summary>
    readonly GameCubeMaterialCooker MaterialCooker;

    /// <summary>
    /// Optional image packager override used by packaged-disc tests and custom tooling flows.
    /// </summary>
    readonly IGameCubeImagePackager ImagePackager;

    /// <summary>
    /// Optional disc system-area override used by packaged-disc tests and custom tooling flows.
    /// </summary>
    readonly GameCubeDiscSystemAreaOptions DiscSystemAreaOptions;

    /// <summary>
    /// Initializes one GameCube builder instance with the current platform metadata.
    /// </summary>
    public GameCubePlatformAssetBuilder()
        : this(new GameCubeDockerNativeBuildExecutor(), null, null) {
    }

    /// <summary>
    /// Initializes one GameCube builder instance with explicit packaged-build collaborators.
    /// </summary>
    /// <param name="nativeBuildExecutor">Native build executor used when the packaged-disc flow is selected.</param>
    /// <param name="imagePackager">Optional image packager override used when the packaged-disc flow is selected.</param>
    /// <param name="discSystemAreaOptions">Optional system-area override used when the packaged-disc flow is selected.</param>
    public GameCubePlatformAssetBuilder(
        IGameCubeNativeBuildExecutor nativeBuildExecutor,
        IGameCubeImagePackager imagePackager,
        GameCubeDiscSystemAreaOptions discSystemAreaOptions) {
        NativeBuildExecutor = nativeBuildExecutor ?? throw new ArgumentNullException(nameof(nativeBuildExecutor));
        MaterialCooker = new GameCubeMaterialCooker();
        ImagePackager = imagePackager;
        DiscSystemAreaOptions = discSystemAreaOptions;
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
    /// Translates one GameCube material schema request into the current minimal cooked payload contract.
    /// </summary>
    /// <param name="request">Material translation request for the GameCube builder.</param>
    /// <returns>Minimal cooked material payload plus referenced shader dependencies.</returns>
    public PlatformMaterialCookResult CookMaterial(PlatformMaterialCookRequest request) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        }

        return MaterialCooker.Cook(request);
    }

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
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        }

        if (string.Equals(
            request.Manifest.ContainerWritePlan.RuntimeSpecializationId,
            PackagedDiscRuntimeSpecializationId,
            StringComparison.Ordinal)) {
            return GameCubeBuildWorkspace.BuildPackagedAsync(
                request,
                progressReporter,
                diagnosticReporter,
                cancellationToken,
                NativeBuildExecutor,
                ImagePackager,
                DiscSystemAreaOptions);
        }

        return GameCubeBuildWorkspace.BuildAsync(request, progressReporter, diagnosticReporter, cancellationToken);
    }
}
