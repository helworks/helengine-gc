using helengine.baseplatform.Requests;

namespace helengine.gamecube.builder;

/// <summary>
/// Centralizes the filesystem paths used by one packaged GameCube build invocation.
/// </summary>
public sealed class GameCubeBuilderPaths {
    /// <summary>
    /// Environment variable that overrides the GameCube repository root when the builder is hosted inside the editor process.
    /// </summary>
    const string RepositoryRootEnvironmentVariableName = "HELENGINE_GAMECUBE_REPOSITORY_ROOT";

    /// <summary>
    /// Creates one packaged-build path set from a resolved GameCube build request.
    /// </summary>
    /// <param name="request">Resolved build request that owns the packaged build paths.</param>
    /// <returns>Packaged-build path set for the supplied request.</returns>
    public static GameCubeBuilderPaths Create(PlatformBuildRequest request) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        }

        string repositoryRootPath = ResolveRepositoryRootPath();
        return new GameCubeBuilderPaths(
            repositoryRootPath,
            request.GeneratedCoreCppRootPath,
            Path.Combine(request.WorkingRoot, "staged-content"),
            Path.Combine(request.OutputRoot, "disc"),
            Path.Combine(request.OutputRoot, "game.gcm"),
            Path.Combine(request.OutputRoot, "native", "helengine_gc.dol"),
            Path.Combine(request.OutputRoot, "native", "apploader.img"),
            Path.Combine(request.OutputRoot, "native", "gbi.hdr"),
            ResolveMemoryCardDiagnosticJournalEnabled(request),
            ResolveNintendontHandoffDiagnosticEnabled(request),
            ResolveFirstFrameTraceDiagnosticEnabled(request),
            ResolveExceptionScreenDiagnosticEnabled(request),
            ResolveGeneratedRuntimeModuleRegistrationEnabled(request),
            ResolveSystemReportEnabled(request),
            ResolveDirectFrameDiagnosticEnabled(request),
            ResolveLogoAnimationDiagnosticEnabled(request),
            ResolveOptionalBooleanBuildOption(request, "enable-input-trace-diagnostic"));
    }

    /// <summary>
    /// Resolves whether the current build explicitly enables the optional memory-card diagnostic journal.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True when the journal build option is enabled; otherwise false.</returns>
    static bool ResolveMemoryCardDiagnosticJournalEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-memory-card-diagnostic-journal");
    }

    /// <summary>
    /// Resolves whether the current build explicitly enables the optional Nintendont handoff diagnostic.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True when the handoff diagnostic build option is enabled; otherwise false.</returns>
    static bool ResolveNintendontHandoffDiagnosticEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-nintendont-handoff-diagnostic");
    }

    /// <summary>
    /// Resolves whether the current build explicitly enables the optional first-frame runtime trace diagnostic.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True when the first-frame trace build option is enabled; otherwise false.</returns>
    static bool ResolveFirstFrameTraceDiagnosticEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-first-frame-trace-diagnostic");
    }

    /// <summary>
    /// Resolves whether the current build explicitly enables the native PowerPC exception-screen diagnostic.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True when the exception-screen diagnostic build option is enabled; otherwise false.</returns>
    static bool ResolveExceptionScreenDiagnosticEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-exception-screen-diagnostic");
    }

    /// <summary>
    /// Resolves whether generated runtime-module registration remains enabled for the native build.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True unless the build explicitly disables generated runtime-module registration.</returns>
    static bool ResolveGeneratedRuntimeModuleRegistrationEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-generated-runtime-module-registration", true);
    }

    /// <summary>
    /// Resolves whether libogc system reports remain available to the native runtime.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True unless the build explicitly disables system reports.</returns>
    static bool ResolveSystemReportEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-system-report", true);
    }

    /// <summary>
    /// Resolves whether the native direct-frame diagnostic should render fixed XFB breadcrumbs and monitor progress through VI retraces.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True when the build explicitly enables the direct-frame diagnostic; otherwise false.</returns>
    static bool ResolveDirectFrameDiagnosticEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-direct-frame-diagnostic");
    }

    /// <summary>
    /// Resolves whether the native logo-animation diagnostic should latch a visible code when sampled sprite rotation is discontinuous.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <returns>True when the build explicitly enables the logo-animation diagnostic; otherwise false.</returns>
    static bool ResolveLogoAnimationDiagnosticEnabled(PlatformBuildRequest request) {
        return ResolveOptionalBooleanBuildOption(request, "enable-logo-animation-diagnostic");
    }

    /// <summary>
    /// Resolves one optional Boolean GameCube build option while rejecting malformed selected values.
    /// </summary>
    /// <param name="request">Resolved platform build request containing selected build options.</param>
    /// <param name="settingId">Stable identifier of the optional Boolean build setting.</param>
    /// <param name="defaultValue">Value used when no option was selected.</param>
    /// <returns>The selected setting value, or <paramref name="defaultValue"/> when no value was selected.</returns>
    static bool ResolveOptionalBooleanBuildOption(PlatformBuildRequest request, string settingId, bool defaultValue = false) {
        if (!request.SelectedBuildOptionValues.TryGetValue(settingId, out string configuredValue)) {
            return defaultValue;
        }

        if (!bool.TryParse(configuredValue, out bool enabled)) {
            throw new InvalidOperationException($"GameCube build option '{settingId}' must be either true or false.");
        }

        return enabled;
    }

    /// <summary>
    /// Resolves the GameCube repository root from the GameCube builder assembly location instead of the hosting application directory.
    /// </summary>
    /// <returns>Absolute GameCube repository root path.</returns>
    /// <exception cref="InvalidOperationException">Thrown when the GameCube repository root cannot be resolved from the builder assembly location.</exception>
    static string ResolveRepositoryRootPath() {
        string configuredRepositoryRootPath = Environment.GetEnvironmentVariable(RepositoryRootEnvironmentVariableName) ?? string.Empty;
        if (IsRepositoryRootPath(configuredRepositoryRootPath)) {
            return Path.GetFullPath(configuredRepositoryRootPath);
        }

        string assemblyLocation = typeof(GameCubeBuilderPaths).Assembly.Location;
        if (string.IsNullOrWhiteSpace(assemblyLocation)) {
            throw new InvalidOperationException("The GameCube builder assembly location could not be resolved.");
        }

        string currentPath = Path.GetDirectoryName(assemblyLocation) ?? string.Empty;
        while (!string.IsNullOrWhiteSpace(currentPath)) {
            if (IsRepositoryRootPath(currentPath)) {
                return currentPath;
            }

            DirectoryInfo parentDirectory = Directory.GetParent(currentPath);
            if (parentDirectory == null) {
                break;
            }

            currentPath = parentDirectory.FullName;
        }

        throw new InvalidOperationException("Could not resolve the helengine-gc repository root from the builder assembly location.");
    }

    /// <summary>
    /// Returns true when one path contains the GameCube repository markers needed for native packaged builds.
    /// </summary>
    /// <param name="path">Candidate repository root path.</param>
    /// <returns>True when the candidate path is the GameCube repository root.</returns>
    static bool IsRepositoryRootPath(string path) {
        if (string.IsNullOrWhiteSpace(path)) {
            return false;
        }

        string makefilePath = Path.Combine(path, "Makefile");
        string bootHostPath = Path.Combine(path, "src", "platform", "gamecube", "GameCubeApplication.cpp");
        return File.Exists(makefilePath) && File.Exists(bootHostPath);
    }

    /// <summary>
    /// Initializes one packaged-build path set.
    /// </summary>
    /// <param name="repositoryRootPath">Repository root that contains the native GameCube project files.</param>
    /// <param name="generatedCoreRootPath">Generated core root that receives packaged runtime manifest files.</param>
    /// <param name="stagingRootPath">Working staging root that receives cooked artifacts before disc layout.</param>
    /// <param name="discRootPath">Extracted disc root written for inspection and image packaging.</param>
    /// <param name="discImagePath">Final GameCube disc-image output path.</param>
    /// <param name="nativeExecutablePath">Packaged-mode native DOL output path staged by the builder.</param>
    /// <param name="nativeApploaderImagePath">Packaged-mode GameCube apploader image output path staged by the builder.</param>
    /// <param name="nativeGenericBootImagePath">Packaged-mode GameCube generic boot image header staged by the builder.</param>
    /// <param name="memoryCardDiagnosticJournalEnabled">True when the optional memory-card diagnostic journal should be compiled into the native runtime.</param>
    /// <param name="nintendontHandoffDiagnosticEnabled">True when the optional Nintendont DOL-entry handoff diagnostic should be compiled into the native runtime.</param>
    /// <param name="firstFrameTraceDiagnosticEnabled">True when the optional first-frame runtime trace should be compiled into the native runtime.</param>
    /// <param name="exceptionScreenDiagnosticEnabled">True when the optional native PowerPC exception screen should be compiled into the native runtime.</param>
    /// <param name="generatedRuntimeModuleRegistrationEnabled">True when generated runtime modules should be registered during core initialization.</param>
    /// <param name="systemReportEnabled">True when libogc system reports should be sent to their configured output.</param>
    /// <param name="directFrameDiagnosticEnabled">True when fixed XFB breadcrumbs and the VI watchdog should be compiled into the native runtime.</param>
    /// <param name="logoAnimationDiagnosticEnabled">True when discontinuous rotating-logo samples should latch a fixed XFB code.</param>
    /// <param name="inputTraceDiagnosticEnabled">True when the first nonzero GameCube controller state should be reported.</param>
    public GameCubeBuilderPaths(
        string repositoryRootPath,
        string generatedCoreRootPath,
        string stagingRootPath,
        string discRootPath,
        string discImagePath,
        string nativeExecutablePath,
        string nativeApploaderImagePath,
        string nativeGenericBootImagePath,
        bool memoryCardDiagnosticJournalEnabled = false,
        bool nintendontHandoffDiagnosticEnabled = false,
        bool firstFrameTraceDiagnosticEnabled = false,
        bool exceptionScreenDiagnosticEnabled = false,
        bool generatedRuntimeModuleRegistrationEnabled = true,
        bool systemReportEnabled = true,
        bool directFrameDiagnosticEnabled = false,
        bool logoAnimationDiagnosticEnabled = false,
        bool inputTraceDiagnosticEnabled = false) {
        RepositoryRootPath = string.IsNullOrWhiteSpace(repositoryRootPath)
            ? throw new ArgumentException("Repository root path is required.", nameof(repositoryRootPath))
            : repositoryRootPath;
        GeneratedCoreRootPath = string.IsNullOrWhiteSpace(generatedCoreRootPath)
            ? throw new ArgumentException("Generated core root path is required.", nameof(generatedCoreRootPath))
            : generatedCoreRootPath;
        StagingRootPath = string.IsNullOrWhiteSpace(stagingRootPath)
            ? throw new ArgumentException("Staging root path is required.", nameof(stagingRootPath))
            : stagingRootPath;
        DiscRootPath = string.IsNullOrWhiteSpace(discRootPath)
            ? throw new ArgumentException("Disc root path is required.", nameof(discRootPath))
            : discRootPath;
        DiscImagePath = string.IsNullOrWhiteSpace(discImagePath)
            ? throw new ArgumentException("Disc image path is required.", nameof(discImagePath))
            : discImagePath;
        NativeExecutablePath = string.IsNullOrWhiteSpace(nativeExecutablePath)
            ? throw new ArgumentException("Native executable path is required.", nameof(nativeExecutablePath))
            : nativeExecutablePath;
        NativeApploaderImagePath = string.IsNullOrWhiteSpace(nativeApploaderImagePath)
            ? throw new ArgumentException("Native apploader image path is required.", nameof(nativeApploaderImagePath))
            : nativeApploaderImagePath;
        NativeGenericBootImagePath = string.IsNullOrWhiteSpace(nativeGenericBootImagePath)
            ? throw new ArgumentException("Native generic boot image path is required.", nameof(nativeGenericBootImagePath))
            : nativeGenericBootImagePath;
        MemoryCardDiagnosticJournalEnabled = memoryCardDiagnosticJournalEnabled;
        NintendontHandoffDiagnosticEnabled = nintendontHandoffDiagnosticEnabled;
        FirstFrameTraceDiagnosticEnabled = firstFrameTraceDiagnosticEnabled;
        ExceptionScreenDiagnosticEnabled = exceptionScreenDiagnosticEnabled;
        GeneratedRuntimeModuleRegistrationEnabled = generatedRuntimeModuleRegistrationEnabled;
        SystemReportEnabled = systemReportEnabled;
        DirectFrameDiagnosticEnabled = directFrameDiagnosticEnabled;
        LogoAnimationDiagnosticEnabled = logoAnimationDiagnosticEnabled;
        InputTraceDiagnosticEnabled = inputTraceDiagnosticEnabled;
    }

    /// <summary>
    /// Gets the repository root that contains the native GameCube project files.
    /// </summary>
    public string RepositoryRootPath { get; }

    /// <summary>
    /// Gets the generated core root that receives packaged runtime manifest files.
    /// </summary>
    public string GeneratedCoreRootPath { get; }

    /// <summary>
    /// Gets the working staging root that receives cooked artifacts before disc layout.
    /// </summary>
    public string StagingRootPath { get; }

    /// <summary>
    /// Gets the extracted disc root written for inspection and image packaging.
    /// </summary>
    public string DiscRootPath { get; }

    /// <summary>
    /// Gets the final GameCube disc-image output path.
    /// </summary>
    public string DiscImagePath { get; }

    /// <summary>
    /// Gets the packaged-mode native DOL output path staged by the builder.
    /// </summary>
    public string NativeExecutablePath { get; }

    /// <summary>
    /// Gets the packaged-mode GameCube apploader image output path staged by the builder.
    /// </summary>
    public string NativeApploaderImagePath { get; }

    /// <summary>
    /// Gets the packaged-mode GameCube generic boot image header output path staged by the builder.
    /// </summary>
    public string NativeGenericBootImagePath { get; }

    /// <summary>
    /// Gets whether the optional memory-card diagnostic journal is enabled for this native build.
    /// </summary>
    public bool MemoryCardDiagnosticJournalEnabled { get; }

    /// <summary>
    /// Gets whether the optional Nintendont DOL-entry handoff diagnostic is enabled for this native build.
    /// </summary>
    public bool NintendontHandoffDiagnosticEnabled { get; }

    /// <summary>
    /// Gets whether the optional one-frame runtime boundary trace is enabled for this native build.
    /// </summary>
    public bool FirstFrameTraceDiagnosticEnabled { get; }

    /// <summary>
    /// Gets whether the optional native PowerPC exception screen is enabled for this build.
    /// </summary>
    public bool ExceptionScreenDiagnosticEnabled { get; }

    /// <summary>
    /// Gets whether generated runtime modules are registered after engine-core initialization.
    /// </summary>
    public bool GeneratedRuntimeModuleRegistrationEnabled { get; }

    /// <summary>
    /// Gets whether libogc system reports remain enabled in the native executable.
    /// </summary>
    public bool SystemReportEnabled { get; }

    /// <summary>
    /// Gets whether the native direct-frame diagnostic is enabled for this build.
    /// </summary>
    public bool DirectFrameDiagnosticEnabled { get; }

    /// <summary>
    /// Gets whether discontinuous rotating-logo samples latch a fixed XFB diagnostic code in the native executable.
    /// </summary>
    public bool LogoAnimationDiagnosticEnabled { get; }

    /// <summary>
    /// Gets whether the first nonzero GameCube controller state is reported to the configured system report sink.
    /// </summary>
    public bool InputTraceDiagnosticEnabled { get; }

    /// <summary>
    /// Gets the generated-core root relative to the repository root for Docker path mapping.
    /// </summary>
    public string GeneratedCoreRelativePath => Path.GetRelativePath(RepositoryRootPath, GeneratedCoreRootPath).Replace('\\', '/');
}
