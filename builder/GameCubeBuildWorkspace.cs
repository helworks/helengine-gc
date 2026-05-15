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
        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);
        Directory.CreateDirectory(paths.GeneratedCoreRootPath);

        List<PlatformBuildDiagnostic> diagnostics = [];
        List<PlatformBuildItemOutcome> sceneOutcomes = BuildSuccessfulSceneOutcomes(request.Manifest.Scenes);
        List<PlatformBuildItemOutcome> looseAssetOutcomes = BuildSuccessfulLooseAssetOutcomes(request.Manifest.LooseAssets);

        ResetDirectory(paths.StagingRootPath);
        Directory.CreateDirectory(paths.StagingRootPath);
        StageCookedArtifacts(request, paths.StagingRootPath, progressReporter, diagnosticReporter, diagnostics, cancellationToken);

        if (diagnostics.Count > 0) {
            return Task.FromResult(new PlatformBuildReport(false, [.. diagnostics], [.. sceneOutcomes], [.. looseAssetOutcomes]));
        }

        new GameCubeRuntimeSceneManifestWriter().Write(paths.GeneratedCoreRootPath, request.Manifest);
        progressReporter.Report(new PlatformBuildProgressUpdate("Generate Runtime Manifest", "runtime-scene-manifest", 1, 4, "Generated GameCube packaged runtime scene manifest."));

        nativeBuildExecutor.Build(paths, cancellationToken);
        progressReporter.Report(new PlatformBuildProgressUpdate("Build Native Executable", "helengine_gc.dol", 2, 4, "Built packaged-mode GameCube native executable."));

        GameCubeDiscSystemAreaOptions effectiveDiscSystemAreaOptions = discSystemAreaOptions ?? CreateConfiguredDiscSystemAreaOptions(request.Manifest, paths);
        GameCubeDiscLayoutResult discLayout = new GameCubeDiscLayoutWriter().Write(paths.StagingRootPath, paths.NativeExecutablePath, paths.DiscRootPath, effectiveDiscSystemAreaOptions);
        progressReporter.Report(new PlatformBuildProgressUpdate("Write Disc Layout", "disc-root", 3, 4, "Wrote extracted GameCube disc layout."));

        IGameCubeImagePackager effectiveImagePackager = imagePackager ?? CreateConfiguredImagePackager(paths);
        effectiveImagePackager.Package(discLayout, paths.DiscImagePath, cancellationToken);
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

        string apploaderPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH");
        if (string.IsNullOrWhiteSpace(apploaderPath)) {
            throw new InvalidOperationException("GameCube packaged-disc builds require HELENGINE_GAMECUBE_APPLOADER_PATH to point at a real GameCube apploader.img.");
        }

        return new GameCubeDiscSystemAreaOptions(
            apploaderPath,
            CreateDiscId(manifest.ProjectId),
            manifest.ProjectId);
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
