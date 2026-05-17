using System.Text.Json;
using helengine.baseplatform.Builders;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;

namespace helengine.gamecube.builder;

/// <summary>
/// Implements the GameCube builder execution flow for staged payloads.
/// </summary>
public static class GameCubeBuildWorkspace {
    /// <summary>
    /// Serializer options used when writing the GameCube build manifest.
    /// </summary>
    static readonly JsonSerializerOptions JsonOptions = new() {
        WriteIndented = true
    };

    /// <summary>
    /// Executes one GameCube build request by copying staged payloads into the output root and writing a manifest.
    /// </summary>
    /// <param name="request">Resolved build request to process.</param>
    /// <param name="progressReporter">Progress reporter that receives streaming updates.</param>
    /// <param name="diagnosticReporter">Diagnostic reporter that receives streaming diagnostics.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the build cooperatively.</param>
    /// <returns>The final build report.</returns>
    public static Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        } else if (progressReporter == null) {
            throw new ArgumentNullException(nameof(progressReporter));
        } else if (diagnosticReporter == null) {
            throw new ArgumentNullException(nameof(diagnosticReporter));
        }

        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);

        List<PlatformBuildDiagnostic> diagnostics = [];
        List<PlatformBuildItemOutcome> sceneOutcomes = [];
        List<PlatformBuildItemOutcome> looseAssetOutcomes = [];
        List<GameCubeBuildManifestEntry> sceneEntries = [];
        List<GameCubeBuildManifestEntry> looseAssetEntries = [];
        int totalItems = request.Manifest.Scenes.Length + request.Manifest.LooseAssets.Length;
        int completedItems = 0;

        CopyScenePayloads(
            request,
            progressReporter,
            diagnosticReporter,
            cancellationToken,
            diagnostics,
            sceneOutcomes,
            sceneEntries,
            totalItems,
            ref completedItems);
        CopyLooseAssetPayloads(
            request,
            progressReporter,
            diagnosticReporter,
            cancellationToken,
            diagnostics,
            looseAssetOutcomes,
            looseAssetEntries,
            totalItems,
            ref completedItems);
        WriteBuildManifest(request, sceneEntries, looseAssetEntries);

        bool succeeded = diagnostics.Count == 0
            && sceneOutcomes.TrueForAll(outcome => outcome.OutcomeKind == PlatformBuildItemOutcomeKind.Succeeded)
            && looseAssetOutcomes.TrueForAll(outcome => outcome.OutcomeKind == PlatformBuildItemOutcomeKind.Succeeded);

        return Task.FromResult(new PlatformBuildReport(
            succeeded,
            [.. diagnostics],
            [.. sceneOutcomes],
            [.. looseAssetOutcomes]));
    }

    /// <summary>
    /// Executes one packaged GameCube build request by staging cooked artifacts, emitting the packaged runtime manifest, invoking the native packaged build, writing the extracted disc layout, and invoking image packaging.
    /// </summary>
    /// <param name="request">Resolved packaged build request to process.</param>
    /// <param name="progressReporter">Progress reporter that receives streaming updates.</param>
    /// <param name="diagnosticReporter">Diagnostic reporter that receives streaming diagnostics.</param>
    /// <param name="cancellationToken">Cancellation token that can stop the build cooperatively.</param>
    /// <param name="nativeBuildExecutor">Native packaged-build executor used to produce the DOL.</param>
    /// <param name="imagePackager">Optional image packager override used to turn the extracted disc layout into a final image artifact.</param>
    /// <returns>The final packaged-build report.</returns>
    public static Task<PlatformBuildReport> BuildPackagedAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken,
        IGameCubeNativeBuildExecutor nativeBuildExecutor,
        IGameCubeImagePackager imagePackager = null,
        GameCubeDiscSystemAreaOptions discSystemAreaOptions = null) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        } else if (progressReporter == null) {
            throw new ArgumentNullException(nameof(progressReporter));
        } else if (diagnosticReporter == null) {
            throw new ArgumentNullException(nameof(diagnosticReporter));
        } else if (nativeBuildExecutor == null) {
            throw new ArgumentNullException(nameof(nativeBuildExecutor));
        }

        GameCubeBuilderPaths paths = GameCubeBuilderPaths.Create(request);
        string phaseMarkerPath = Path.Combine(request.OutputRoot, "gamecube-build-phase.txt");
        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);
        Directory.CreateDirectory(paths.GeneratedCoreRootPath);

        List<PlatformBuildDiagnostic> diagnostics = [];
        List<PlatformBuildItemOutcome> sceneOutcomes = BuildSuccessfulSceneOutcomes(request.Manifest.Scenes);
        List<PlatformBuildItemOutcome> looseAssetOutcomes = BuildSuccessfulLooseAssetOutcomes(request.Manifest.LooseAssets);

        ResetDirectory(paths.StagingRootPath);
        Directory.CreateDirectory(paths.StagingRootPath);
        WritePhaseMarker(phaseMarkerPath, "platform cook work items begin");
        ExecutePlatformCookWorkItems(request.Manifest.PlatformCookWorkItems ?? [], paths.StagingRootPath);
        WritePhaseMarker(phaseMarkerPath, "platform cook work items completed");
        WritePhaseMarker(phaseMarkerPath, "stage cooked artifacts begin");
        StageCookedArtifacts(request, paths.StagingRootPath, progressReporter, diagnosticReporter, diagnostics, cancellationToken);
        WritePhaseMarker(phaseMarkerPath, "stage cooked artifacts completed");

        if (diagnostics.Count > 0) {
            WritePhaseMarker(phaseMarkerPath, "diagnostics reported before packaged native build");
            return Task.FromResult(new PlatformBuildReport(false, [.. diagnostics], [.. sceneOutcomes], [.. looseAssetOutcomes]));
        }

        new GameCubeRuntimeSceneManifestWriter().Write(paths.GeneratedCoreRootPath, request.Manifest);
        WritePhaseMarker(phaseMarkerPath, "runtime scene manifest written");
        progressReporter.Report(new PlatformBuildProgressUpdate("Generate Runtime Manifest", "runtime-scene-manifest", 1, 4, "Generated GameCube packaged runtime scene manifest."));

        nativeBuildExecutor.Build(paths, cancellationToken);
        WritePhaseMarker(phaseMarkerPath, "native build completed");
        progressReporter.Report(new PlatformBuildProgressUpdate("Build Native Executable", "helengine_gc.dol", 2, 4, "Built packaged-mode GameCube native executable."));

        GameCubeDiscSystemAreaOptions effectiveDiscSystemAreaOptions = discSystemAreaOptions ?? CreateConfiguredDiscSystemAreaOptions(request.Manifest, paths);
        GameCubeDiscLayoutResult discLayout = new GameCubeDiscLayoutWriter().Write(paths.StagingRootPath, paths.NativeExecutablePath, paths.DiscRootPath, effectiveDiscSystemAreaOptions);
        WritePhaseMarker(phaseMarkerPath, "disc layout written");
        progressReporter.Report(new PlatformBuildProgressUpdate("Write Disc Layout", "disc-root", 3, 4, "Wrote extracted GameCube disc layout."));

        IGameCubeImagePackager effectiveImagePackager = imagePackager ?? CreateConfiguredImagePackager(paths);
        effectiveImagePackager.Package(discLayout, paths.DiscImagePath, cancellationToken);
        WritePhaseMarker(phaseMarkerPath, "disc image packaged");
        VerifyPackagedOutputs(paths);
        WritePhaseMarker(phaseMarkerPath, "packaged outputs verified");
        progressReporter.Report(new PlatformBuildProgressUpdate("Package Disc Image", "game.gcm", 4, 4, "Packaged GameCube disc image artifact."));

        return Task.FromResult(new PlatformBuildReport(true, [.. diagnostics], [.. sceneOutcomes], [.. looseAssetOutcomes]));
    }

    /// <summary>
    /// Creates the configured GameCube image packager for the staged retail-style raw disc-image path.
    /// </summary>
    /// <param name="paths">Packaged-build paths that expose the native packaged-disc artifacts.</param>
    /// <returns>Configured GameCube image packager.</returns>
    static IGameCubeImagePackager CreateConfiguredImagePackager(GameCubeBuilderPaths paths) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        string witExecutablePath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
        if (!string.IsNullOrWhiteSpace(witExecutablePath)) {
            return new GameCubeWiimmsIsoToolsImagePackager(
                new GameCubeWiimmsIsoToolsOptions(witExecutablePath),
                new GameCubeProcessRunner());
        }

        return new GameCubeRawImagePackager();
    }

    /// <summary>
    /// Creates the configured extracted-disc system-area options from explicit environment configuration and manifest metadata.
    /// </summary>
    /// <param name="manifest">Manifest supplying project metadata for the disc header.</param>
    /// <returns>Configured extracted-disc system-area options.</returns>
    static GameCubeDiscSystemAreaOptions CreateConfiguredDiscSystemAreaOptions(PlatformBuildManifest manifest, GameCubeBuilderPaths paths) {
        if (manifest == null) {
            throw new ArgumentNullException(nameof(manifest));
        } else if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        string apploaderPath = ResolveApploaderPath(paths);

        return new GameCubeDiscSystemAreaOptions(
            apploaderPath,
            CreateDiscId(manifest.ProjectId),
            manifest.ProjectId);
    }

    /// <summary>
    /// Resolves the apploader image path for one packaged GameCube build, preferring an explicit environment override and falling back to the staged native packaged-disc apploader.
    /// </summary>
    /// <param name="paths">Packaged-build paths that expose the staged native apploader artifact.</param>
    /// <returns>Absolute apploader image path for the packaged disc system area.</returns>
    /// <exception cref="InvalidOperationException">Thrown when neither the explicit override nor the staged native apploader image exists.</exception>
    static string ResolveApploaderPath(GameCubeBuilderPaths paths) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        string explicitApploaderPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH");
        if (!string.IsNullOrWhiteSpace(explicitApploaderPath)) {
            if (!File.Exists(explicitApploaderPath)) {
                throw new InvalidOperationException($"HELENGINE_GAMECUBE_APPLOADER_PATH does not point at an existing apploader.img: {explicitApploaderPath}");
            }

            return explicitApploaderPath;
        } else if (File.Exists(paths.NativeApploaderImagePath)) {
            return paths.NativeApploaderImagePath;
        }

        throw new InvalidOperationException(
            $"GameCube packaged-disc builds require either HELENGINE_GAMECUBE_APPLOADER_PATH or a staged native apploader image at '{paths.NativeApploaderImagePath}'.");
    }

    /// <summary>
    /// Creates one stable GameCube-shaped ID6 value from the authored project id.
    /// </summary>
    /// <param name="projectId">Authored project identifier.</param>
    /// <returns>Stable GameCube-shaped ID6 value.</returns>
    static string CreateDiscId(string projectId) {
        if (string.IsNullOrWhiteSpace(projectId)) {
            throw new ArgumentException("Project id is required.", nameof(projectId));
        }

        string normalizedProjectId = new string(projectId
            .ToUpperInvariant()
            .Where(char.IsAsciiLetterOrDigit)
            .Take(2)
            .ToArray());

        if (normalizedProjectId.Length == 0) {
            normalizedProjectId = "HX";
        } else if (normalizedProjectId.Length < 2) {
            normalizedProjectId = normalizedProjectId.PadRight(2, 'X');
        }

        return "G" + normalizedProjectId + "EHE";
    }

    /// <summary>
    /// Copies all staged scene payloads into the output root.
    /// </summary>
    /// <param name="request">The resolved build request.</param>
    /// <param name="progressReporter">The progress reporter receiving updates.</param>
    /// <param name="diagnosticReporter">The diagnostic reporter receiving failures.</param>
    /// <param name="cancellationToken">Cancellation token used to stop cooperative work.</param>
    /// <param name="diagnostics">Collected diagnostics.</param>
    /// <param name="sceneOutcomes">Collected scene outcomes.</param>
    /// <param name="sceneEntries">Collected scene manifest entries.</param>
    /// <param name="totalItems">The total number of staged items.</param>
    /// <param name="completedItems">The completed item count.</param>
    static void CopyScenePayloads(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken,
        List<PlatformBuildDiagnostic> diagnostics,
        List<PlatformBuildItemOutcome> sceneOutcomes,
        List<GameCubeBuildManifestEntry> sceneEntries,
        int totalItems,
        ref int completedItems) {
        for (int sceneIndex = 0; sceneIndex < request.Manifest.Scenes.Length; sceneIndex++) {
            cancellationToken.ThrowIfCancellationRequested();

            PlatformBuildScene scene = request.Manifest.Scenes[sceneIndex];
            CopyPayload(
                scene.SceneId,
                scene.SourceIdentity,
                request.OutputRoot,
                diagnostics,
                diagnosticReporter,
                "GCBUILD001",
                "GCBUILD002",
                out bool copied,
                out string outputPath);

            sceneOutcomes.Add(new PlatformBuildItemOutcome(
                scene.SceneId,
                copied ? PlatformBuildItemOutcomeKind.Succeeded : PlatformBuildItemOutcomeKind.Failed));

            completedItems++;
            progressReporter.Report(new PlatformBuildProgressUpdate(
                "Stage Payloads",
                scene.SceneId,
                completedItems,
                totalItems,
                copied ? $"Staged scene '{scene.SceneName}'." : $"Failed to stage scene '{scene.SceneName}'."));

            if (copied) {
                sceneEntries.Add(new GameCubeBuildManifestEntry(scene.SceneId, scene.SourceIdentity, outputPath));
            }
        }
    }

    /// <summary>
    /// Copies all staged loose-asset payloads into the output root.
    /// </summary>
    /// <param name="request">The resolved build request.</param>
    /// <param name="progressReporter">The progress reporter receiving updates.</param>
    /// <param name="diagnosticReporter">The diagnostic reporter receiving failures.</param>
    /// <param name="cancellationToken">Cancellation token used to stop cooperative work.</param>
    /// <param name="diagnostics">Collected diagnostics.</param>
    /// <param name="looseAssetOutcomes">Collected loose-asset outcomes.</param>
    /// <param name="looseAssetEntries">Collected loose-asset manifest entries.</param>
    /// <param name="totalItems">The total number of staged items.</param>
    /// <param name="completedItems">The completed item count.</param>
    static void CopyLooseAssetPayloads(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken,
        List<PlatformBuildDiagnostic> diagnostics,
        List<PlatformBuildItemOutcome> looseAssetOutcomes,
        List<GameCubeBuildManifestEntry> looseAssetEntries,
        int totalItems,
        ref int completedItems) {
        for (int assetIndex = 0; assetIndex < request.Manifest.LooseAssets.Length; assetIndex++) {
            cancellationToken.ThrowIfCancellationRequested();

            PlatformBuildAsset asset = request.Manifest.LooseAssets[assetIndex];
            CopyPayload(
                asset.AssetId,
                asset.SourceIdentity,
                request.OutputRoot,
                diagnostics,
                diagnosticReporter,
                "GCBUILD003",
                "GCBUILD004",
                out bool copied,
                out string outputPath);

            looseAssetOutcomes.Add(new PlatformBuildItemOutcome(
                asset.AssetId,
                copied ? PlatformBuildItemOutcomeKind.Succeeded : PlatformBuildItemOutcomeKind.Failed));

            completedItems++;
            progressReporter.Report(new PlatformBuildProgressUpdate(
                "Stage Payloads",
                asset.AssetId,
                completedItems,
                totalItems,
                copied ? $"Staged asset '{asset.AssetName}'." : $"Failed to stage asset '{asset.AssetName}'."));

            if (copied) {
                looseAssetEntries.Add(new GameCubeBuildManifestEntry(asset.AssetId, asset.SourceIdentity, outputPath));
            }
        }
    }

    /// <summary>
    /// Copies one staged payload into the output root and records any failure as a diagnostic.
    /// </summary>
    /// <param name="itemId">Resolved item identifier.</param>
    /// <param name="sourceIdentity">Source identity recorded in the build request.</param>
    /// <param name="outputRoot">Final output root for the build.</param>
    /// <param name="diagnostics">Diagnostic list collecting failures.</param>
    /// <param name="diagnosticReporter">Diagnostic reporter that mirrors collected failures.</param>
    /// <param name="missingIdentityCode">Diagnostic code for a missing source identity.</param>
    /// <param name="missingFileCode">Diagnostic code for a missing source file.</param>
    /// <param name="copied">Returns whether the payload was copied.</param>
    /// <param name="outputPath">Returns the destination path for the copied payload.</param>
    static void CopyPayload(
        string itemId,
        string sourceIdentity,
        string outputRoot,
        List<PlatformBuildDiagnostic> diagnostics,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        string missingIdentityCode,
        string missingFileCode,
        out bool copied,
        out string outputPath) {
        copied = false;
        outputPath = string.Empty;

        if (string.IsNullOrWhiteSpace(sourceIdentity)) {
            AddDiagnostic(
                diagnostics,
                diagnosticReporter,
                PlatformBuildDiagnosticSeverity.Error,
                missingIdentityCode,
                $"Item '{itemId}' is missing a source identity.",
                string.Empty,
                itemId,
                string.Empty);
            return;
        }

        string sourcePath = ResolveSourcePath(sourceIdentity);
        if (!File.Exists(sourcePath)) {
            AddDiagnostic(
                diagnostics,
                diagnosticReporter,
                PlatformBuildDiagnosticSeverity.Error,
                missingFileCode,
                $"Payload source '{sourceIdentity}' was not found.",
                string.Empty,
                itemId,
                sourceIdentity);
            return;
        }

        outputPath = ResolveOutputPath(outputRoot, sourceIdentity);
        string destinationDirectory = Path.GetDirectoryName(outputPath);
        if (!string.IsNullOrWhiteSpace(destinationDirectory)) {
            Directory.CreateDirectory(destinationDirectory);
        }

        File.Copy(sourcePath, outputPath, true);
        copied = true;
    }

    /// <summary>
    /// Adds one diagnostic to the shared list and mirrors it to the reporter.
    /// </summary>
    /// <param name="diagnostics">Collected diagnostics.</param>
    /// <param name="diagnosticReporter">Diagnostic reporter to mirror.</param>
    /// <param name="severity">Diagnostic severity.</param>
    /// <param name="code">Diagnostic code.</param>
    /// <param name="message">Diagnostic message.</param>
    /// <param name="sceneId">Scene identifier for the diagnostic.</param>
    /// <param name="assetId">Asset identifier for the diagnostic.</param>
    /// <param name="sourceIdentity">Source identity for the diagnostic.</param>
    static void AddDiagnostic(
        List<PlatformBuildDiagnostic> diagnostics,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        PlatformBuildDiagnosticSeverity severity,
        string code,
        string message,
        string sceneId,
        string assetId,
        string sourceIdentity) {
        PlatformBuildDiagnostic diagnostic = new(severity, code, message, sceneId, assetId, sourceIdentity);
        diagnostics.Add(diagnostic);
        diagnosticReporter.Report(diagnostic);
    }

    /// <summary>
    /// Writes the GameCube build manifest to the working root for traceability.
    /// </summary>
    /// <param name="request">Resolved build request.</param>
    /// <param name="sceneEntries">Resolved scene entries.</param>
    /// <param name="looseAssetEntries">Resolved loose-asset entries.</param>
    static void WriteBuildManifest(
        PlatformBuildRequest request,
        IReadOnlyList<GameCubeBuildManifestEntry> sceneEntries,
        IReadOnlyList<GameCubeBuildManifestEntry> looseAssetEntries) {
        string manifestPath = Path.Combine(request.WorkingRoot, "gamecube-build-manifest.json");
        object manifest = new {
            request.Manifest.ProjectId,
            request.Manifest.ProjectVersion,
            request.Manifest.RequiredEngineVersion,
            request.OutputRoot,
            Scenes = sceneEntries,
            LooseAssets = looseAssetEntries
        };

        File.WriteAllText(manifestPath, JsonSerializer.Serialize(manifest, JsonOptions));
    }

    /// <summary>
    /// Stages all cooked artifacts referenced by the packaged build manifest into the supplied staging root.
    /// </summary>
    /// <param name="request">Resolved packaged build request.</param>
    /// <param name="stagingRootPath">Working staging root that receives cooked artifacts.</param>
    /// <param name="progressReporter">Progress reporter that receives staging updates.</param>
    /// <param name="diagnosticReporter">Diagnostic reporter that receives staging failures.</param>
    /// <param name="diagnostics">Diagnostic list collecting staging failures.</param>
    /// <param name="cancellationToken">Cancellation token used to stop cooperative work.</param>
    static void StageCookedArtifacts(
        PlatformBuildRequest request,
        string stagingRootPath,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        List<PlatformBuildDiagnostic> diagnostics,
        CancellationToken cancellationToken) {
        PlatformBuildArtifact[] cookedArtifacts = request.Manifest.CookedArtifacts ?? [];
        for (int artifactIndex = 0; artifactIndex < cookedArtifacts.Length; artifactIndex++) {
            cancellationToken.ThrowIfCancellationRequested();

            PlatformBuildArtifact artifact = cookedArtifacts[artifactIndex];
            string sourcePath = ResolveSourcePath(artifact.RelativePath);
            if (!File.Exists(sourcePath)) {
                AddDiagnostic(
                    diagnostics,
                    diagnosticReporter,
                    PlatformBuildDiagnosticSeverity.Error,
                    "GCPACK001",
                    $"Cooked artifact '{artifact.RelativePath}' was not found in the staged package root.",
                    string.Empty,
                    artifact.LogicalArtifactId,
                    artifact.RelativePath);
                continue;
            }

            string destinationPath = Path.Combine(stagingRootPath, NormalizeRelativePath(artifact.RelativePath));
            string destinationDirectoryPath = Path.GetDirectoryName(destinationPath) ?? throw new InvalidOperationException($"Destination directory could not be resolved for '{destinationPath}'.");
            Directory.CreateDirectory(destinationDirectoryPath);
            File.Copy(sourcePath, destinationPath, true);
            progressReporter.Report(new PlatformBuildProgressUpdate(
                "Stage Cooked Artifacts",
                artifact.LogicalArtifactId,
                artifactIndex + 1,
                cookedArtifacts.Length,
                $"Staged cooked artifact '{artifact.RelativePath}'."));
        }
    }

    /// <summary>
    /// Executes any builder-owned platform cook work items emitted by the shared editor build graph.
    /// </summary>
    /// <param name="platformCookWorkItems">Platform cook work items emitted by the editor build graph.</param>
    /// <param name="stagingRootPath">Staging root that receives the cooked outputs.</param>
    static void ExecutePlatformCookWorkItems(PlatformCookWorkItem[] platformCookWorkItems, string stagingRootPath) {
        if (platformCookWorkItems == null) {
            throw new ArgumentNullException(nameof(platformCookWorkItems));
        } else if (string.IsNullOrWhiteSpace(stagingRootPath)) {
            throw new ArgumentException("Staging root path must be provided.", nameof(stagingRootPath));
        }

        if (platformCookWorkItems.Length == 0) {
            return;
        }

        new GameCubePlatformCookWorkItemExecutor().Execute(platformCookWorkItems, Directory.GetCurrentDirectory(), stagingRootPath);
    }

    /// <summary>
    /// Builds successful scene outcomes for the packaged-build report.
    /// </summary>
    /// <param name="scenes">Scenes included in the packaged build request.</param>
    /// <returns>Successful scene outcomes for the packaged build request.</returns>
    static List<PlatformBuildItemOutcome> BuildSuccessfulSceneOutcomes(PlatformBuildScene[] scenes) {
        List<PlatformBuildItemOutcome> outcomes = [];
        if (scenes == null) {
            return outcomes;
        }

        for (int index = 0; index < scenes.Length; index++) {
            outcomes.Add(new PlatformBuildItemOutcome(scenes[index].SceneId, PlatformBuildItemOutcomeKind.Succeeded));
        }

        return outcomes;
    }

    /// <summary>
    /// Builds successful loose-asset outcomes for the packaged-build report.
    /// </summary>
    /// <param name="looseAssets">Loose assets included in the packaged build request.</param>
    /// <returns>Successful loose-asset outcomes for the packaged build request.</returns>
    static List<PlatformBuildItemOutcome> BuildSuccessfulLooseAssetOutcomes(PlatformBuildAsset[] looseAssets) {
        List<PlatformBuildItemOutcome> outcomes = [];
        if (looseAssets == null) {
            return outcomes;
        }

        for (int index = 0; index < looseAssets.Length; index++) {
            outcomes.Add(new PlatformBuildItemOutcome(looseAssets[index].AssetId, PlatformBuildItemOutcomeKind.Succeeded));
        }

        return outcomes;
    }

    /// <summary>
    /// Removes one directory tree when it already exists so the packaged staging root can be rebuilt deterministically.
    /// </summary>
    /// <param name="path">Directory path to remove before rebuilding it.</param>
    static void ResetDirectory(string path) {
        if (Directory.Exists(path)) {
            Directory.Delete(path, recursive: true);
        }
    }

    /// <summary>
    /// Appends one build-phase marker into the current GameCube output root so editor-owned build failures can be recovered after the host process exits.
    /// </summary>
    /// <param name="phaseMarkerPath">Absolute phase-marker file path inside the active output root.</param>
    /// <param name="message">Human-readable phase message to append.</param>
    static void WritePhaseMarker(string phaseMarkerPath, string message) {
        if (string.IsNullOrWhiteSpace(phaseMarkerPath)) {
            throw new ArgumentException("Phase marker path must be provided.", nameof(phaseMarkerPath));
        } else if (string.IsNullOrWhiteSpace(message)) {
            throw new ArgumentException("Phase marker message must be provided.", nameof(message));
        }

        File.AppendAllText(phaseMarkerPath, message + Environment.NewLine);
        Console.WriteLine("[helengine-gc] " + message);
    }

    /// <summary>
    /// Verifies the packaged GameCube outputs exist after the builder finishes writing the disc layout and disc image.
    /// </summary>
    /// <param name="paths">Resolved GameCube builder paths that define the expected packaged outputs.</param>
    static void VerifyPackagedOutputs(GameCubeBuilderPaths paths) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        string discMainDolPath = Path.Combine(paths.DiscRootPath, "sys", "main.dol");
        string discBootImagePath = Path.Combine(paths.DiscRootPath, "sys", "boot.bin");
        string discBi2Path = Path.Combine(paths.DiscRootPath, "sys", "bi2.bin");
        string discApploaderPath = Path.Combine(paths.DiscRootPath, "sys", "apploader.img");
        if (!File.Exists(paths.NativeExecutablePath)) {
            throw new FileNotFoundException("The packaged native GameCube DOL was not staged into the build output.", paths.NativeExecutablePath);
        } else if (!File.Exists(paths.NativeApploaderImagePath)) {
            throw new FileNotFoundException("The packaged native GameCube apploader image was not staged into the build output.", paths.NativeApploaderImagePath);
        } else if (!File.Exists(discMainDolPath)) {
            throw new FileNotFoundException("The extracted GameCube disc main.dol was not produced.", discMainDolPath);
        } else if (!File.Exists(discBootImagePath)) {
            throw new FileNotFoundException("The extracted GameCube disc boot.bin was not produced.", discBootImagePath);
        } else if (!File.Exists(discBi2Path)) {
            throw new FileNotFoundException("The extracted GameCube disc bi2.bin was not produced.", discBi2Path);
        } else if (!File.Exists(discApploaderPath)) {
            throw new FileNotFoundException("The extracted GameCube disc apploader.img was not produced.", discApploaderPath);
        } else if (!File.Exists(paths.DiscImagePath)) {
            throw new FileNotFoundException("The packaged GameCube disc image was not produced.", paths.DiscImagePath);
        }
    }

    /// <summary>
    /// Resolves one relative source identity into a full path from the current working directory.
    /// </summary>
    /// <param name="sourceIdentity">The source identity recorded in the request.</param>
    /// <returns>The full source path.</returns>
    static string ResolveSourcePath(string sourceIdentity) {
        string normalizedSourceIdentity = NormalizeRelativePath(sourceIdentity);
        if (Path.IsPathRooted(normalizedSourceIdentity)) {
            return Path.GetFullPath(normalizedSourceIdentity);
        }

        return Path.GetFullPath(Path.Combine(Directory.GetCurrentDirectory(), normalizedSourceIdentity));
    }

    /// <summary>
    /// Resolves one output path under the supplied output root.
    /// </summary>
    /// <param name="outputRoot">The final output root.</param>
    /// <param name="sourceIdentity">The request source identity.</param>
    /// <returns>The full output path.</returns>
    static string ResolveOutputPath(string outputRoot, string sourceIdentity) {
        string normalizedSourceIdentity = NormalizeRelativePath(sourceIdentity);
        if (Path.IsPathRooted(normalizedSourceIdentity)) {
            normalizedSourceIdentity = Path.GetFileName(normalizedSourceIdentity);
        }

        return Path.GetFullPath(Path.Combine(outputRoot, normalizedSourceIdentity));
    }

    /// <summary>
    /// Normalizes one path for the current host operating system.
    /// </summary>
    /// <param name="path">The path to normalize.</param>
    /// <returns>The normalized path.</returns>
    static string NormalizeRelativePath(string path) {
        return path.Replace('\\', Path.DirectorySeparatorChar).Replace('/', Path.DirectorySeparatorChar);
    }
}
