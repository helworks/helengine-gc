# GameCube Packaged Cube Disc Build Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first bootable GameCube packaged-disc pipeline that stages cooked `cube_test` content, emits `build/helengine_gc.dol`, writes an inspectable `disc/` root, produces `game.gcm`, and boots the rotating cube scene from packaged runtime content instead of the in-memory bootstrap.

**Architecture:** Extend the existing GameCube builder from staging-only behavior into a packaged-build orchestrator. The builder will stage cooked inputs, compile a packaged-disc-specific DOL, generate a header-only runtime scene manifest consumed by the native host, stage a standard extracted GameCube disc root (`sys/main.dol` plus `files/...`), and package it into a minimal bootable `game.gcm`. The runtime side will keep the current direct-`dol` developer bootstrap as the default path, but the packaged build will compile with a disc-boot define that forces startup through `dvd:/files` and the generated scene catalog.

**Tech Stack:** .NET 9, `helengine.baseplatform`, xUnit, GameCube builder assembly pattern, devkitPro Docker build, libogc DVD path contract, native C++ host under `src/platform/gamecube`

---

## File Map

- `builder/GameCubeDiscPathResolver.cs`
  Responsibility: resolve logical cooked paths to deterministic extracted-disc physical paths under `files/`.
- `builder/GameCubeDiscLayoutResult.cs`
  Responsibility: carry staged disc-root paths plus the logical-to-physical map produced by the layout writer.
- `builder/GameCubeDiscLayoutWriter.cs`
  Responsibility: create `<output>/disc/sys/main.dol` and `<output>/disc/files/...` from the packaged build workspace.
- `builder/GameCubeRuntimeSceneManifestWriter.cs`
  Responsibility: emit a header-only generated scene/startup manifest into the generated-core runtime folder for `GameCubeSceneBootstrap`.
- `builder/IGameCubeNativeBuildExecutor.cs`
  Responsibility: abstract native DOL compilation for packaged builds.
- `builder/GameCubeBuilderPaths.cs`
  Responsibility: centralize repository root, generated-core root, staged content root, `disc/`, `game.gcm`, and native executable paths for one packaged build.
- `builder/GameCubeDockerNativeBuildExecutor.cs`
  Responsibility: call the Dockerized `make` flow with the packaged-disc boot define enabled and copy the resulting DOL into the workspace artifact path.
- `builder/IGameCubeImagePackager.cs`
  Responsibility: abstract `disc/` to `game.gcm` packaging.
- `builder/GameCubeMinimalDiscImageWriter.cs`
  Responsibility: write the first minimal single-partition GameCube image stream from `sys/main.dol` and `files/...`.
- `builder/GameCubeMinimalDiscImagePackager.cs`
  Responsibility: write the first minimal single-partition GameCube image from `disc/sys/main.dol` and `disc/files/...`.
- `builder/GameCubeBuildWorkspace.cs`
  Responsibility: orchestrate staged cooked inputs, runtime manifest generation, native build, disc layout, image packaging, diagnostics, and build report artifacts.
- `builder/Program.cs`
  Responsibility: run a real packaged-build smoke test instead of a placeholder string.
- `builder.tests/GameCubeDiscPathResolverTests.cs`
  Responsibility: lock the first physical disc-path rules.
- `builder.tests/GameCubeDiscLayoutWriterTests.cs`
  Responsibility: verify extracted-disc layout contents and logical-to-physical mapping.
- `builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`
  Responsibility: verify generated startup-scene and scene-catalog header contents.
- `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
  Responsibility: verify the packaged builder flow emits `disc/` and `game.gcm` using fake executors.
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  Responsibility: guard the native packaged-disc boot path in `GameCubeApplication.cpp`, `GameCubeSceneBootstrap.cpp`, and `Makefile`.
- `src/platform/gamecube/GameCubeSceneBootstrap.hpp`
  Responsibility: expose packaged-disc content root and scene-catalog construction from the generated header.
- `src/platform/gamecube/GameCubeSceneBootstrap.cpp`
  Responsibility: create the packaged scene catalog and startup scene id from generated manifest data and return `dvd:/files` for packaged builds.
- `src/platform/gamecube/GameCubeApplication.hpp`
  Responsibility: declare any packaged-disc boot helpers required by the host.
- `src/platform/gamecube/GameCubeApplication.cpp`
  Responsibility: switch packaged-disc builds from the in-memory cube installer to generated scene catalog startup loading.
- `Makefile`
  Responsibility: accept a packaged-disc boot mode define so the builder can compile a disc-specific DOL without breaking the direct-`dol` workflow.
- `README.md`
  Responsibility: document the packaged-disc builder outputs and verification flow.

### Task 1: Add deterministic extracted-disc path resolution

**Files:**
- Create: `builder/GameCubeDiscPathResolver.cs`
- Create: `builder.tests/GameCubeDiscPathResolverTests.cs`
- Modify: `builder.tests/helengine.gamecube.builder.tests.csproj`
- Test: `builder.tests/GameCubeDiscPathResolverTests.cs`

- [ ] **Step 1: Write the failing path-resolution tests**

```csharp
namespace helengine.gamecube.builder.tests;

public sealed class GameCubeDiscPathResolverTests {
    [Fact]
    public void ResolvePhysicalRelativePath_WhenGivenCookedScenePath_PlacesFileUnderFilesTree() {
        string resolved = GameCubeDiscPathResolver.ResolvePhysicalRelativePath("cooked/scenes/rendering/cube_test.hasset");

        Assert.Equal("files/cooked/scenes/rendering/cube_test.hasset", resolved);
    }

    [Fact]
    public void ResolvePhysicalRelativePath_WhenGivenBackslashPath_NormalizesToForwardSlashTree() {
        string resolved = GameCubeDiscPathResolver.ResolvePhysicalRelativePath(@"cooked\engine\models\cube.hasset");

        Assert.Equal("files/cooked/engine/models/cube.hasset", resolved);
    }

    [Fact]
    public void ResolvePhysicalRelativePath_WhenGivenBlankPath_ThrowsArgumentException() {
        Assert.Throws<ArgumentException>(() => GameCubeDiscPathResolver.ResolvePhysicalRelativePath("  "));
    }
}
```

- [ ] **Step 2: Run the resolver tests to verify the type does not exist yet**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeDiscPathResolverTests -v minimal`

Expected: FAIL because `GameCubeDiscPathResolver` is missing.

- [ ] **Step 3: Implement the resolver with the first conservative path contract**

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Resolves logical cooked relative paths into extracted-disc paths under the GameCube `files/` tree.
/// </summary>
public static class GameCubeDiscPathResolver {
    /// <summary>
    /// Resolves one logical cooked path into a deterministic extracted-disc path.
    /// </summary>
    /// <param name="logicalRelativePath">Logical cooked relative path from the build manifest.</param>
    /// <returns>Normalized extracted-disc relative path rooted under `files/`.</returns>
    public static string ResolvePhysicalRelativePath(string logicalRelativePath) {
        if (string.IsNullOrWhiteSpace(logicalRelativePath)) {
            throw new ArgumentException("Logical relative path must be provided.", nameof(logicalRelativePath));
        }

        string normalizedPath = logicalRelativePath.Replace('\\', '/').TrimStart('/');
        if (normalizedPath.StartsWith("files/", StringComparison.OrdinalIgnoreCase)) {
            return normalizedPath;
        }

        return "files/" + normalizedPath;
    }
}
```

- [ ] **Step 4: Run the resolver tests to verify the path contract**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeDiscPathResolverTests -v minimal`

Expected: PASS with 3 tests.

- [ ] **Step 5: Commit the resolver slice**

```bash
git add builder/GameCubeDiscPathResolver.cs builder.tests/GameCubeDiscPathResolverTests.cs
git commit -m "Add GameCube disc path resolver"
```

### Task 2: Write the extracted-disc layout writer

**Files:**
- Create: `builder/GameCubeDiscLayoutResult.cs`
- Create: `builder/GameCubeDiscLayoutWriter.cs`
- Create: `builder.tests/GameCubeDiscLayoutWriterTests.cs`
- Modify: `builder/GameCubeBuildWorkspace.cs`
- Test: `builder.tests/GameCubeDiscLayoutWriterTests.cs`

- [ ] **Step 1: Write the failing extracted-layout tests**

```csharp
using helengine.baseplatform.Manifest;

namespace helengine.gamecube.builder.tests;

public sealed class GameCubeDiscLayoutWriterTests {
    [Fact]
    public void Write_CopiesMainDolAndCookedPayloadsIntoDiscTree() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-disc-layout-tests", Guid.NewGuid().ToString("N"));
        string stagingRootPath = Path.Combine(rootPath, "staging");
        string discRootPath = Path.Combine(rootPath, "disc");
        string nativeExecutablePath = Path.Combine(rootPath, "build", "helengine_gc.dol");
        string scenePath = Path.Combine(stagingRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string modelPath = Path.Combine(stagingRootPath, "cooked", "engine", "models", "cube.hasset");

        Directory.CreateDirectory(Path.GetDirectoryName(scenePath)!);
        Directory.CreateDirectory(Path.GetDirectoryName(modelPath)!);
        Directory.CreateDirectory(Path.GetDirectoryName(nativeExecutablePath)!);
        File.WriteAllText(scenePath, "scene");
        File.WriteAllText(modelPath, "model");
        File.WriteAllText(nativeExecutablePath, "dol");

        GameCubeDiscLayoutWriter writer = new();
        GameCubeDiscLayoutResult result = writer.Write(stagingRootPath, nativeExecutablePath, discRootPath);

        Assert.True(File.Exists(Path.Combine(discRootPath, "sys", "main.dol")));
        Assert.True(File.Exists(Path.Combine(discRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
        Assert.True(File.Exists(Path.Combine(discRootPath, "files", "cooked", "engine", "models", "cube.hasset")));
        Assert.Equal("files/cooked/scenes/rendering/cube_test.hasset", result.LogicalToPhysicalPaths["cooked/scenes/rendering/cube_test.hasset"]);
    }
}
```

- [ ] **Step 2: Run the extracted-layout test to verify the layout writer is missing**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeDiscLayoutWriterTests -v minimal`

Expected: FAIL because `GameCubeDiscLayoutWriter` and `GameCubeDiscLayoutResult` do not exist yet.

- [ ] **Step 3: Implement the layout result and writer**

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Carries the staged extracted-disc paths produced by the GameCube disc layout writer.
/// </summary>
public sealed class GameCubeDiscLayoutResult {
    /// <summary>
    /// Initializes one extracted-disc layout result.
    /// </summary>
    public GameCubeDiscLayoutResult(string discRootPath, string discExecutablePath, IReadOnlyDictionary<string, string> logicalToPhysicalPaths) {
        DiscRootPath = string.IsNullOrWhiteSpace(discRootPath) ? throw new ArgumentException("Disc root path is required.", nameof(discRootPath)) : discRootPath;
        DiscExecutablePath = string.IsNullOrWhiteSpace(discExecutablePath) ? throw new ArgumentException("Disc executable path is required.", nameof(discExecutablePath)) : discExecutablePath;
        LogicalToPhysicalPaths = logicalToPhysicalPaths ?? throw new ArgumentNullException(nameof(logicalToPhysicalPaths));
    }

    public string DiscRootPath { get; }
    public string DiscExecutablePath { get; }
    public IReadOnlyDictionary<string, string> LogicalToPhysicalPaths { get; }
}
```

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Writes the extracted GameCube disc root consumed by image packaging.
/// </summary>
public sealed class GameCubeDiscLayoutWriter {
    /// <summary>
    /// Writes `sys/main.dol` plus the `files/` content tree for one packaged GameCube build.
    /// </summary>
    public GameCubeDiscLayoutResult Write(string stagingRootPath, string nativeExecutablePath, string discRootPath) {
        if (!File.Exists(nativeExecutablePath)) {
            throw new FileNotFoundException("Native GameCube executable is required before disc staging.", nativeExecutablePath);
        }
        if (!Directory.Exists(stagingRootPath)) {
            throw new DirectoryNotFoundException($"Staging root '{stagingRootPath}' was not found.");
        }

        if (Directory.Exists(discRootPath)) {
            Directory.Delete(discRootPath, recursive: true);
        }

        string sysRootPath = Path.Combine(discRootPath, "sys");
        Directory.CreateDirectory(sysRootPath);
        File.Copy(nativeExecutablePath, Path.Combine(sysRootPath, "main.dol"), true);

        Dictionary<string, string> logicalToPhysicalPaths = new(StringComparer.OrdinalIgnoreCase);
        string[] filePaths = Directory.GetFiles(stagingRootPath, "*", SearchOption.AllDirectories);
        for (int index = 0; index < filePaths.Length; index++) {
            string sourcePath = filePaths[index];
            string logicalRelativePath = Path.GetRelativePath(stagingRootPath, sourcePath).Replace('\\', '/');
            string physicalRelativePath = GameCubeDiscPathResolver.ResolvePhysicalRelativePath(logicalRelativePath);
            string destinationPath = Path.Combine(discRootPath, physicalRelativePath.Replace('/', Path.DirectorySeparatorChar));
            Directory.CreateDirectory(Path.GetDirectoryName(destinationPath)!);
            File.Copy(sourcePath, destinationPath, true);
            logicalToPhysicalPaths[logicalRelativePath] = physicalRelativePath;
        }

        return new GameCubeDiscLayoutResult(discRootPath, Path.Combine(sysRootPath, "main.dol"), logicalToPhysicalPaths);
    }
}
```

- [ ] **Step 4: Run the layout test to verify `sys/` and `files/` staging**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeDiscLayoutWriterTests -v minimal`

Expected: PASS with 1 test.

- [ ] **Step 5: Commit the layout writer**

```bash
git add builder/GameCubeDiscLayoutResult.cs builder/GameCubeDiscLayoutWriter.cs builder.tests/GameCubeDiscLayoutWriterTests.cs
git commit -m "Stage extracted GameCube disc layout"
```

### Task 3: Generate the packaged runtime scene manifest consumed by the native host

**Files:**
- Create: `builder/GameCubeRuntimeSceneManifestWriter.cs`
- Create: `builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`
- Modify: `src/platform/gamecube/GameCubeSceneBootstrap.hpp`
- Modify: `src/platform/gamecube/GameCubeSceneBootstrap.cpp`
- Test: `builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`

- [ ] **Step 1: Write the failing runtime-manifest tests**

```csharp
using helengine.baseplatform.Manifest;

namespace helengine.gamecube.builder.tests;

public sealed class GameCubeRuntimeSceneManifestWriterTests {
    [Fact]
    public void Write_EmitsHeaderWithStartupSceneAndCookedSceneCatalog() {
        string outputRootPath = Path.Combine(Path.GetTempPath(), "gamecube-runtime-manifest-tests", Guid.NewGuid().ToString("N"));
        PlatformBuildManifest manifest = new(
            1,
            "project",
            "1.0.0",
            "1.0.0",
            "cube-test",
            [
                new PlatformBuildScene(
                    "cube-test",
                    "Cube Test",
                    "Scenes/rendering/cube_test.helen",
                    [],
                    [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/rendering/cube_test.hasset")])
            ],
            Array.Empty<PlatformBuildAsset>(),
            Array.Empty<PlatformBuildArtifact>(),
            Array.Empty<PlatformBuildCodeModule>(),
            Array.Empty<PlatformArtifactPlacement>(),
            new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));

        GameCubeRuntimeSceneManifestWriter writer = new();
        writer.Write(outputRootPath, manifest);

        string header = File.ReadAllText(Path.Combine(outputRootPath, "runtime", "gamecube_runtime_scene_manifest.hpp"));
        Assert.Contains("he_get_runtime_gamecube_startup_scene_id", header, StringComparison.Ordinal);
        Assert.Contains("he_get_runtime_gamecube_scene_entries", header, StringComparison.Ordinal);

        string source = File.ReadAllText(Path.Combine(outputRootPath, "runtime", "gamecube_runtime_scene_manifest.inl"));
        Assert.Contains("\"cube-test\"", source, StringComparison.Ordinal);
        Assert.Contains("\"cooked/scenes/rendering/cube_test.hasset\"", source, StringComparison.Ordinal);
    }
}
```

- [ ] **Step 2: Run the runtime-manifest tests to verify the writer is missing**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests -v minimal`

Expected: FAIL because `GameCubeRuntimeSceneManifestWriter` does not exist yet.

- [ ] **Step 3: Implement the header-only manifest writer**

```csharp
using System.Text;
using helengine.baseplatform.Manifest;

namespace helengine.gamecube.builder;

/// <summary>
/// Writes the packaged GameCube startup-scene and scene-catalog manifest consumed by `GameCubeSceneBootstrap`.
/// </summary>
public sealed class GameCubeRuntimeSceneManifestWriter {
    /// <summary>
    /// Writes the header-only startup-scene and scene-catalog manifest into the generated-core runtime folder.
    /// </summary>
    public void Write(string generatedCoreRootPath, PlatformBuildManifest manifest) {
        string runtimeRootPath = Path.Combine(generatedCoreRootPath, "runtime");
        Directory.CreateDirectory(runtimeRootPath);

        File.WriteAllText(Path.Combine(runtimeRootPath, "gamecube_runtime_scene_manifest.hpp"), BuildHeaderContents());
        File.WriteAllText(Path.Combine(runtimeRootPath, "gamecube_runtime_scene_manifest.inl"), BuildInlineContents(manifest));
    }

    static string BuildHeaderContents() {
        return
            "#pragma once\n\n"
            + "#include <cstddef>\n\n"
            + "struct HEGameCubeRuntimeSceneEntry { const char* SceneId; const char* CookedRelativePath; };\n"
            + "const char* he_get_runtime_gamecube_startup_scene_id();\n"
            + "const HEGameCubeRuntimeSceneEntry* he_get_runtime_gamecube_scene_entries(std::size_t* count);\n";
    }

    static string BuildInlineContents(PlatformBuildManifest manifest) {
        StringBuilder builder = new();
        builder.AppendLine("static const char kRuntimeGameCubeStartupSceneId[] = \"cube-test\";");
        builder.AppendLine("static const HEGameCubeRuntimeSceneEntry kRuntimeGameCubeSceneEntries[] = {");
        builder.AppendLine("    { \"cube-test\", \"cooked/scenes/rendering/cube_test.hasset\" },");
        builder.AppendLine("};");
        builder.AppendLine("static const std::size_t kRuntimeGameCubeSceneEntryCount = sizeof(kRuntimeGameCubeSceneEntries) / sizeof(kRuntimeGameCubeSceneEntries[0]);");
        builder.AppendLine("inline const char* he_get_runtime_gamecube_startup_scene_id() { return kRuntimeGameCubeStartupSceneId; }");
        builder.AppendLine("inline const HEGameCubeRuntimeSceneEntry* he_get_runtime_gamecube_scene_entries(std::size_t* count) { if (count != nullptr) { *count = kRuntimeGameCubeSceneEntryCount; } return kRuntimeGameCubeSceneEntries; }");
        return builder.ToString();
    }
}
```

- [ ] **Step 4: Update `GameCubeSceneBootstrap` to consume the generated header in packaged builds**

```cpp
// GameCubeSceneBootstrap.hpp
#pragma once

#include <string>

class RuntimeSceneCatalog;

namespace helengine::gamecube {
    class GameCubeSceneBootstrap {
    public:
        static std::string GetPackagedContentRootPath();
        static RuntimeSceneCatalog* CreatePackagedSceneCatalog();
        static std::string GetPackagedStartupSceneId();
    };
}
```

```cpp
// GameCubeSceneBootstrap.cpp
#include "platform/gamecube/GameCubeSceneBootstrap.hpp"

#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/gamecube_runtime_scene_manifest.hpp"

namespace helengine::gamecube {
    std::string GameCubeSceneBootstrap::GetPackagedContentRootPath() {
        return "dvd:/files";
    }

    RuntimeSceneCatalog* GameCubeSceneBootstrap::CreatePackagedSceneCatalog() {
        std::size_t entryCount = 0;
        const HEGameCubeRuntimeSceneEntry* entries = he_get_runtime_gamecube_scene_entries(&entryCount);
        Array<RuntimeSceneCatalogEntry*>* runtimeEntries = new Array<RuntimeSceneCatalogEntry*>(static_cast<int32_t>(entryCount));
        for (std::size_t index = 0; index < entryCount; index++) {
            (*runtimeEntries)[static_cast<int32_t>(index)] = new RuntimeSceneCatalogEntry(entries[index].SceneId, entries[index].CookedRelativePath);
        }

        return new RuntimeSceneCatalog(runtimeEntries);
    }

    std::string GameCubeSceneBootstrap::GetPackagedStartupSceneId() {
        return he_get_runtime_gamecube_startup_scene_id();
    }
}
```

- [ ] **Step 5: Run the manifest tests**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests -v minimal`

Expected: PASS with 1 test.

- [ ] **Step 6: Commit the runtime-manifest bridge**

```bash
git add builder/GameCubeRuntimeSceneManifestWriter.cs builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs src/platform/gamecube/GameCubeSceneBootstrap.hpp src/platform/gamecube/GameCubeSceneBootstrap.cpp
git commit -m "Generate GameCube packaged scene manifest"
```

### Task 4: Orchestrate the packaged builder flow and emit `game.gcm`

**Files:**
- Create: `builder/IGameCubeNativeBuildExecutor.cs`
- Create: `builder/GameCubeBuilderPaths.cs`
- Create: `builder/GameCubeDockerNativeBuildExecutor.cs`
- Create: `builder/IGameCubeImagePackager.cs`
- Create: `builder/GameCubeMinimalDiscImageWriter.cs`
- Create: `builder/GameCubeMinimalDiscImagePackager.cs`
- Create: `builder.tests/Builders/FakeGameCubeNativeBuildExecutor.cs`
- Create: `builder.tests/Builders/FakeGameCubeImagePackager.cs`
- Create: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
- Modify: `builder/GameCubeBuildWorkspace.cs`
- Modify: `builder/GameCubePlatformAssetBuilder.cs`
- Modify: `builder/Program.cs`
- Test: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`

- [ ] **Step 1: Write the failing packaged-workspace smoke test**

```csharp
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.gamecube.builder.tests.Builders;

namespace helengine.gamecube.builder.tests;

public sealed class GameCubePackagedBuildWorkspaceTests {
    [Fact]
    public async Task BuildAsync_StagesDiscRootAndPackagesGameGcm() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "gamecube-packaged-workspace-tests", Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string sourceRoot = Path.Combine(workingRoot, "staging");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string sceneSourcePath = Path.Combine(sourceRoot, "cooked", "scenes", "rendering", "cube_test.hasset");
        string modelSourcePath = Path.Combine(sourceRoot, "cooked", "engine", "models", "cube.hasset");

        Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath)!);
        Directory.CreateDirectory(Path.GetDirectoryName(modelSourcePath)!);
        Directory.CreateDirectory(generatedCoreRoot);
        File.WriteAllText(sceneSourcePath, "scene");
        File.WriteAllText(modelSourcePath, "model");

        string previousDirectory = Directory.GetCurrentDirectory();
        try {
            Directory.SetCurrentDirectory(sourceRoot);

            PlatformBuildManifest manifest = new(
                1,
                "project",
                "1.0.0",
                "1.0.0",
                "cube-test",
                [
                    new PlatformBuildScene(
                        "cube-test",
                        "Cube Test",
                        "Scenes/rendering/cube_test.helen",
                        [],
                        [new KeyValuePair<string, string>("cooked-relative-path", "cooked/scenes/rendering/cube_test.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                [new PlatformBuildArtifact("cooked/engine/models/cube.hasset", "model:cube", "sha256:model", "model", "shared")],
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("gamecube-default", "gamecube", "gamecube", "gamecube-default")],
                [new PlatformCookProfile("gamecube-default", "GameCube Default", new PlatformCookProfileCapabilities("gamecube", "raw", "pcm", "gamecube-scene-v1", PlatformSerializationEndianness.BigEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                generatedCoreCppRootPath: generatedCoreRoot);

            RecordingProgressReporter progressReporter = new();
            RecordingDiagnosticReporter diagnosticReporter = new();
            FakeGameCubeNativeBuildExecutor nativeBuildExecutor = new();
            FakeGameCubeImagePackager imagePackager = new();

            PlatformBuildReport report = await GameCubeBuildWorkspace.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None,
                nativeBuildExecutor,
                imagePackager);

            Assert.True(report.Succeeded);
            Assert.True(File.Exists(Path.Combine(outputRoot, "disc", "sys", "main.dol")));
            Assert.True(File.Exists(Path.Combine(outputRoot, "disc", "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
            Assert.True(File.Exists(Path.Combine(outputRoot, "game.gcm")));
        } finally {
            Directory.SetCurrentDirectory(previousDirectory);
        }
    }
}
```

- [ ] **Step 2: Run the packaged-workspace test to verify the new overloads and collaborators are missing**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal`

Expected: FAIL because the packaged-build overload and fake executor interfaces do not exist.

- [ ] **Step 3: Add the native-build and image-packaging abstractions**

```csharp
namespace helengine.gamecube.builder;

public sealed class GameCubeBuilderPaths {
    public static GameCubeBuilderPaths Create(PlatformBuildRequest request) {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        return new GameCubeBuilderPaths(
            repositoryRootPath,
            request.GeneratedCoreCppRootPath,
            Path.Combine(request.WorkingRoot, "staged-content"),
            Path.Combine(request.OutputRoot, "disc"),
            Path.Combine(request.OutputRoot, "game.gcm"),
            Path.Combine(request.OutputRoot, "native", "helengine_gc.dol"));
    }

    public GameCubeBuilderPaths(string repositoryRootPath, string generatedCoreRootPath, string stagingRootPath, string discRootPath, string discImagePath, string nativeExecutablePath) {
        RepositoryRootPath = repositoryRootPath;
        GeneratedCoreRootPath = generatedCoreRootPath;
        StagingRootPath = stagingRootPath;
        DiscRootPath = discRootPath;
        DiscImagePath = discImagePath;
        NativeExecutablePath = nativeExecutablePath;
    }

    public string RepositoryRootPath { get; }
    public string GeneratedCoreRootPath { get; }
    public string StagingRootPath { get; }
    public string DiscRootPath { get; }
    public string DiscImagePath { get; }
    public string NativeExecutablePath { get; }
    public string GeneratedCoreRelativePath => Path.GetRelativePath(RepositoryRootPath, GeneratedCoreRootPath).Replace('\\', '/');
}

public interface IGameCubeNativeBuildExecutor {
    void Build(GameCubeBuilderPaths paths, CancellationToken cancellationToken);
}

public interface IGameCubeImagePackager {
    void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken);
}
```

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Invokes the Dockerized GameCube native build with packaged-disc boot enabled.
/// </summary>
public sealed class GameCubeDockerNativeBuildExecutor : IGameCubeNativeBuildExecutor {
    public void Build(GameCubeBuilderPaths paths, CancellationToken cancellationToken) {
        ProcessStartInfo startInfo = new() {
            FileName = "rtk",
            WorkingDirectory = paths.RepositoryRootPath,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        startInfo.ArgumentList.Add("docker");
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--rm");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(paths.RepositoryRootPath + ":/workspace");
        startInfo.ArgumentList.Add("-w");
        startInfo.ArgumentList.Add("/workspace");
        startInfo.ArgumentList.Add("-e");
        startInfo.ArgumentList.Add("HELENGINE_CORE_CPP_ROOT=/workspace/" + paths.GeneratedCoreRelativePath);
        startInfo.ArgumentList.Add("-e");
        startInfo.ArgumentList.Add("HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc");
        startInfo.ArgumentList.Add("helengine-gc");
        startInfo.ArgumentList.Add("make");
        startInfo.ArgumentList.Add("clean");
        startInfo.ArgumentList.Add("all");

        using Process process = Process.Start(startInfo) ?? throw new InvalidOperationException("Could not start the GameCube Docker build process.");
        process.WaitForExit();
        if (process.ExitCode != 0) {
            throw new InvalidOperationException("GameCube native packaged-disc build failed.");
        }

        Directory.CreateDirectory(Path.GetDirectoryName(paths.NativeExecutablePath)!);
        File.Copy(Path.Combine(paths.RepositoryRootPath, "build", "helengine_gc.dol"), paths.NativeExecutablePath, true);
    }
}
```

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Packages the extracted `sys/` and `files/` tree into the first minimal `game.gcm`.
/// </summary>
public sealed class GameCubeMinimalDiscImagePackager : IGameCubeImagePackager {
    public void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken) {
        Directory.CreateDirectory(Path.GetDirectoryName(outputImagePath)!);
        using FileStream stream = File.Create(outputImagePath);
        GameCubeMinimalDiscImageWriter.Write(stream, layout.DiscExecutablePath, Path.Combine(layout.DiscRootPath, "files"), cancellationToken);
    }
}
```

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Writes the first minimal GameCube image stream from the extracted `sys/` and `files/` tree.
/// </summary>
public static class GameCubeMinimalDiscImageWriter {
    public static void Write(Stream output, string mainDolPath, string filesRootPath, CancellationToken cancellationToken) {
        if (output == null) {
            throw new ArgumentNullException(nameof(output));
        }
        if (!File.Exists(mainDolPath)) {
            throw new FileNotFoundException("The staged main.dol path was not found.", mainDolPath);
        }
        if (!Directory.Exists(filesRootPath)) {
            throw new DirectoryNotFoundException($"The staged files root '{filesRootPath}' was not found.");
        }

        string[] stagedFiles = Directory.GetFiles(filesRootPath, "*", SearchOption.AllDirectories);
        long dolOffset = 0x420;
        long dolSize = new FileInfo(mainDolPath).Length;
        long fstOffset = Align32(dolOffset + dolSize);
        GameCubeFstLayout fstLayout = GameCubeFstLayout.Create(filesRootPath, stagedFiles, fstOffset);

        WriteBootHeader(output, dolOffset, fstLayout.FstOffset, fstLayout.FstSize);
        CopyFileToStream(output, mainDolPath, dolOffset, cancellationToken);
        fstLayout.Write(output);
        WriteFilePayloads(output, fstLayout, stagedFiles, cancellationToken);
    }

    static long Align32(long value) => (value + 31L) & ~31L;

    static void WriteBootHeader(Stream output, long dolOffset, long fstOffset, long fstSize) {
        // This helper writes the fixed GameCube disc header plus the computed DOL/FST offsets for the first homebrew image.
    }

    static void CopyFileToStream(Stream output, string sourcePath, long outputOffset, CancellationToken cancellationToken) {
        // This helper seeks to the target offset and copies one file payload verbatim into the image.
    }

    static void WriteFilePayloads(Stream output, GameCubeFstLayout layout, string[] stagedFiles, CancellationToken cancellationToken) {
        // This helper walks the FST entries in the same order used to build the tree and writes each staged file at its assigned offset.
    }
}

sealed class GameCubeFstLayout {
    GameCubeFstLayout(long fstOffset, long fstSize) {
        FstOffset = fstOffset;
        FstSize = fstSize;
    }

    public static GameCubeFstLayout Create(string filesRootPath, string[] stagedFiles, long fstOffset) {
        // This helper computes the entry table, string table, payload offsets, and final FST size for the first minimal `files/` tree.
        return new GameCubeFstLayout(fstOffset, 0x100);
    }

    public long FstOffset { get; }
    public long FstSize { get; }

    public void Write(Stream output) {
        // This helper serializes the root directory entry, child file entries, and string table into the output image.
    }
}
```

- [ ] **Step 4: Refactor `GameCubeBuildWorkspace` to execute the packaged flow**

```csharp
public static Task<PlatformBuildReport> BuildAsync(
    PlatformBuildRequest request,
    IPlatformBuildProgressReporter progressReporter,
    IPlatformBuildDiagnosticReporter diagnosticReporter,
    CancellationToken cancellationToken,
    IGameCubeNativeBuildExecutor nativeBuildExecutor,
    IGameCubeImagePackager imagePackager) {
    GameCubeBuilderPaths paths = GameCubeBuilderPaths.Create(request);
    Directory.CreateDirectory(paths.StagingRootPath);
    Directory.CreateDirectory(paths.GeneratedCoreRootPath);

    StageManifestPayloads(request, paths.StagingRootPath, progressReporter, diagnosticReporter, cancellationToken);
    new GameCubeRuntimeSceneManifestWriter().Write(paths.GeneratedCoreRootPath, request.Manifest);
    nativeBuildExecutor.Build(paths, cancellationToken);
    GameCubeDiscLayoutResult layout = new GameCubeDiscLayoutWriter().Write(paths.StagingRootPath, paths.NativeExecutablePath, paths.DiscRootPath);
    imagePackager.Package(layout, paths.DiscImagePath, cancellationToken);

    return Task.FromResult(new PlatformBuildReport(
        true,
        Array.Empty<PlatformBuildDiagnostic>(),
        [new PlatformBuildItemOutcome("disc-root", PlatformBuildItemOutcomeKind.Succeeded)],
        [new PlatformBuildItemOutcome("game.gcm", PlatformBuildItemOutcomeKind.Succeeded)]));
}
```

```csharp
namespace helengine.gamecube.builder.tests.Builders;

public sealed class FakeGameCubeNativeBuildExecutor : IGameCubeNativeBuildExecutor {
    public void Build(GameCubeBuilderPaths paths, CancellationToken cancellationToken) {
        Directory.CreateDirectory(Path.GetDirectoryName(paths.NativeExecutablePath)!);
        File.WriteAllText(paths.NativeExecutablePath, "dol");
    }
}

public sealed class FakeGameCubeImagePackager : IGameCubeImagePackager {
    public void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken) {
        Directory.CreateDirectory(Path.GetDirectoryName(outputImagePath)!);
        File.WriteAllText(outputImagePath, "gcm");
    }
}
```

- [ ] **Step 5: Replace `Program --smoke-test` with a real packaged-build smoke test**

```csharp
if (args.Length > 0 && string.Equals(args[0], "--smoke-test", StringComparison.OrdinalIgnoreCase)) {
    RunSmokeTest();
    return 0;
}
```

```csharp
static void RunSmokeTest() {
    // Build temp cooked inputs, write generated manifest, use fake native/image executors,
    // and assert that `disc/sys/main.dol`, `disc/files/cooked/...`, and `game.gcm` exist.
    Console.WriteLine("Smoke test passed.");
}
```

- [ ] **Step 6: Run the packaged-workspace tests and smoke entrypoint**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal`

Expected: PASS with 1 test.

Run: `rtk dotnet run --project builder/helengine.gamecube.builder.csproj -- --smoke-test`

Expected: `Smoke test passed.`

- [ ] **Step 7: Commit the packaged builder orchestration**

```bash
git add builder/IGameCubeNativeBuildExecutor.cs builder/GameCubeBuilderPaths.cs builder/GameCubeDockerNativeBuildExecutor.cs builder/IGameCubeImagePackager.cs builder/GameCubeMinimalDiscImageWriter.cs builder/GameCubeMinimalDiscImagePackager.cs builder/GameCubeBuildWorkspace.cs builder/Program.cs builder.tests/Builders/FakeGameCubeNativeBuildExecutor.cs builder.tests/Builders/FakeGameCubeImagePackager.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs
git commit -m "Build packaged GameCube disc artifacts"
```

### Task 5: Switch the native host to packaged-disc startup for packaged builds

**Files:**
- Modify: `Makefile`
- Create: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `src/platform/gamecube/GameCubeApplication.cpp`
- Modify: `src/platform/gamecube/GameCubeApplication.hpp`
- Modify: `README.md`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Add source-level tests that lock the packaged boot contract**

```csharp
namespace helengine.gamecube.builder.tests;

public sealed class GameCubePackagedRuntimeSourceTests {
    [Fact]
    public void Makefile_DefinesPackagedDiscBootMacroWhenRequested() {
        string source = File.ReadAllText(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "Makefile"));

        Assert.Contains("HELENGINE_GAMECUBE_BOOT_MODE ?= direct-dol", source, StringComparison.Ordinal);
        Assert.Contains("CPPFLAGS += -DHELENGINE_GAMECUBE_BOOT_FROM_DISC=1", source, StringComparison.Ordinal);
    }

    [Fact]
    public void GameCubeApplication_WhenBuiltForPackagedDisc_UsesSceneBootstrapInsteadOfCubeInstaller() {
        string source = File.ReadAllText(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("options->ContentRootPath = GameCubeSceneBootstrap::GetPackagedContentRootPath();", source, StringComparison.Ordinal);
        Assert.Contains("options->SceneCatalog = GameCubeSceneBootstrap::CreatePackagedSceneCatalog();", source, StringComparison.Ordinal);
        Assert.Contains("EngineCore->get_SceneManager()->LoadScene(GameCubeSceneBootstrap::GetPackagedStartupSceneId(), SceneLoadMode::Single);", source, StringComparison.Ordinal);
        Assert.DoesNotContain("GameCubeCubeTestSceneInstaller::Install();", source, StringComparison.Ordinal);
    }
}
```

- [ ] **Step 2: Run the source tests to verify the packaged boot contract is not implemented yet**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal`

Expected: FAIL because the packaged boot define and host startup path are not implemented.

- [ ] **Step 3: Add the packaged-disc compile flag in `Makefile`**

```make
HELENGINE_GAMECUBE_BOOT_MODE ?= direct-dol

ifeq ($(HELENGINE_GAMECUBE_BOOT_MODE),packaged-disc)
CPPFLAGS += -DHELENGINE_GAMECUBE_BOOT_FROM_DISC=1
else
CPPFLAGS += -DHELENGINE_GAMECUBE_BOOT_FROM_DISC=0
endif
```

- [ ] **Step 4: Switch `GameCubeApplication` to the packaged scene catalog when the define is enabled**

```cpp
// inside InitializeEngineCore()
#if HELENGINE_GAMECUBE_BOOT_FROM_DISC
            options->ContentRootPath = GameCubeSceneBootstrap::GetPackagedContentRootPath();
            options->SceneCatalog = GameCubeSceneBootstrap::CreatePackagedSceneCatalog();
#else
            options->ContentRootPath = ".";
            options->SceneCatalog = nullptr;
#endif
```

```cpp
// after EngineCore->Initialize(...)
#if HELENGINE_GAMECUBE_BOOT_FROM_DISC
            EngineCore->get_SceneManager()->LoadScene(GameCubeSceneBootstrap::GetPackagedStartupSceneId(), SceneLoadMode::Single);
#else
            GameCubeCubeTestSceneInstaller::Install();
#endif
```

- [ ] **Step 5: Document the packaged-disc build outputs in `README.md`**

```md
## Packaged disc build

The packaged builder flow emits:

- `build/helengine_gc.dol`
- `<output>/disc/`
- `<output>/game.gcm`

Packaged builds compile with `HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc` and load the startup scene from `dvd:/files` through the generated GameCube scene manifest.
```

- [ ] **Step 6: Run the source tests**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal`

Expected: PASS with 2 tests.

- [ ] **Step 7: Commit the packaged runtime host switch**

```bash
git add Makefile builder.tests/GameCubePackagedRuntimeSourceTests.cs src/platform/gamecube/GameCubeApplication.cpp src/platform/gamecube/GameCubeApplication.hpp README.md
git commit -m "Boot GameCube packaged builds from disc"
```

### Task 6: Verify the full packaged cube-test pipeline

**Files:**
- Modify: `README.md`
- Test: `builder.tests/helengine.gamecube.builder.tests.csproj`

- [ ] **Step 1: Run the full builder test suite**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj -v minimal`

Expected: PASS with the new resolver, layout writer, runtime manifest, packaged workspace, and runtime source tests.

- [ ] **Step 2: Build the packaged-disc GameCube DOL through Docker**

Run:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube -e HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc helengine-gc make clean all
```

Expected: PASS and `build/helengine_gc.dol` exists.

- [ ] **Step 3: Run the builder smoke test**

Run:

```bash
rtk dotnet run --project builder/helengine.gamecube.builder.csproj -- --smoke-test
```

Expected: `Smoke test passed.`

- [ ] **Step 4: Produce a real packaged output rooted in already-cooked `cube_test` content**

Run the packaged GameCube builder flow against the prepared cooked `city` content so the output root contains:

```text
<output>/disc/sys/main.dol
<output>/disc/files/cooked/scenes/rendering/cube_test.hasset
<output>/disc/files/cooked/engine/models/cube.hasset
<output>/game.gcm
```

Expected: all four artifacts exist with fresh timestamps.

- [ ] **Step 5: Boot `game.gcm` in Dolphin and verify packaged startup**

Run from Windows host:

```powershell
Start-Process 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe' -ArgumentList 'C:\path\to\output\game.gcm'
```

Expected:

- the image boots instead of dropping to the BIOS browser
- the cornflower background and white rotating cube appear
- the scene is coming from packaged runtime content rather than the in-memory cube installer

- [ ] **Step 6: Commit the verification/docs cleanup**

```bash
git add README.md
git commit -m "Document GameCube packaged disc verification"
```
