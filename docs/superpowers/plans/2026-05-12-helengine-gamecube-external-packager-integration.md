# GameCube External Packager Integration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Integrate a real external GameCube image packager into `helengine-gc` so packaged builds can produce `game.gcm` from the extracted `disc/` root using an explicitly configured `wit` executable path.

**Architecture:** Keep the existing packaged-build orchestration and `IGameCubeImagePackager` interface, then add a `Wiimms ISO Tools` backend plus a testable process-runner boundary. Keep canonical `disc/sys/main.dol` and `disc/files/...` as the builder-owned layout, and add a tool-specific adapter only if real `wit` behavior proves that layout is insufficient.

**Tech Stack:** .NET 9, xUnit, existing GameCube builder/test projects, external `wit` CLI from Wiimms ISO Tools, packaged-disc native DOL flow

---

## File Map

- `builder/GameCubeWiimmsIsoToolsOptions.cs`
  Responsibility: carry explicit `wit` executable path configuration for packaged builds.
- `builder/IGameCubeProcessRunner.cs`
  Responsibility: abstract external process execution for image packaging.
- `builder/GameCubeProcessRunResult.cs`
  Responsibility: carry exit code, stdout, and stderr from one external process invocation.
- `builder/GameCubeProcessRunner.cs`
  Responsibility: run real external processes and capture outputs for diagnostics.
- `builder/GameCubeWiimmsIsoToolsImagePackager.cs`
  Responsibility: validate `wit` configuration, invoke the external tool, and verify `game.gcm` output.
- `builder/GameCubeWiimmsDiscLayoutAdapter.cs`
  Responsibility: adapt canonical extracted-disc layout into a `wit`-consumable temp root only if real integration requires it.
- `builder/GameCubeBuildWorkspace.cs`
  Responsibility: read explicit `wit` configuration and select the real external packager in packaged builds.
- `builder/Program.cs`
  Responsibility: extend smoke mode so the builder can report packaged-tool configuration state.
- `builder.tests/Builders/FakeGameCubeProcessRunner.cs`
  Responsibility: simulate `wit` process outcomes for packaging tests.
- `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`
  Responsibility: verify missing-path, invalid-path, failed-process, and success packaging behavior.
- `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
  Responsibility: verify packaged workspace can route through explicit `wit` configuration instead of the fake placeholder packager.
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  Responsibility: continue guarding the packaged-disc native boot contract after builder changes.
- `README.md`
  Responsibility: document packaged-build prerequisites and the explicit `HELENGINE_GAMECUBE_WIT_PATH` contract.

Conditional runtime/test artifacts if real integration needs layout adaptation:

- `builder.tests/GameCubeWiimmsDiscLayoutAdapterTests.cs`
  Responsibility: lock the adapter behavior if `wit` does not accept the canonical `disc/` root directly.

### Task 1: Add explicit `wit` configuration and process-runner boundary

**Files:**
- Create: `builder/GameCubeWiimmsIsoToolsOptions.cs`
- Create: `builder/IGameCubeProcessRunner.cs`
- Create: `builder/GameCubeProcessRunResult.cs`
- Create: `builder/GameCubeProcessRunner.cs`
- Create: `builder.tests/Builders/FakeGameCubeProcessRunner.cs`
- Create: `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`
- Test: `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`

- [ ] **Step 1: Write the failing tests for explicit configuration and process results**

```csharp
using helengine.gamecube.builder.tests.Builders;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the explicit Wiimms ISO Tools configuration boundary before real packaging logic is added.
/// </summary>
public sealed class GameCubeWiimmsIsoToolsImagePackagerTests {
    /// <summary>
    /// Ensures the packager rejects a missing configured executable path.
    /// </summary>
    [Fact]
    public void Package_WhenExecutablePathIsBlank_ThrowsInvalidOperationException() {
        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(string.Empty),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, string.Empty, string.Empty)));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
            packager.Package(CreateLayout(), Path.Combine(Path.GetTempPath(), "game.gcm"), CancellationToken.None));

        Assert.Contains("wit", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Ensures the packager rejects a configured executable path that does not exist.
    /// </summary>
    [Fact]
    public void Package_WhenExecutablePathDoesNotExist_ThrowsFileNotFoundException() {
        GameCubeWiimmsIsoToolsImagePackager packager = new(
            new GameCubeWiimmsIsoToolsOptions(Path.Combine(Path.GetTempPath(), "missing", "wit.exe")),
            new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, string.Empty, string.Empty)));

        Assert.Throws<FileNotFoundException>(() =>
            packager.Package(CreateLayout(), Path.Combine(Path.GetTempPath(), "game.gcm"), CancellationToken.None));
    }

    static GameCubeDiscLayoutResult CreateLayout() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
        string discRootPath = Path.Combine(rootPath, "disc");
        string mainDolPath = Path.Combine(discRootPath, "sys", "main.dol");
        string scenePath = Path.Combine(discRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset");
        Directory.CreateDirectory(Path.GetDirectoryName(mainDolPath)!);
        Directory.CreateDirectory(Path.GetDirectoryName(scenePath)!);
        File.WriteAllText(mainDolPath, "dol");
        File.WriteAllText(scenePath, "scene");
        return new GameCubeDiscLayoutResult(
            discRootPath,
            mainDolPath,
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
                ["cooked/scenes/rendering/cube_test.hasset"] = "files/cooked/scenes/rendering/cube_test.hasset"
            });
    }
}
```

- [ ] **Step 2: Run the targeted tests to verify the new types do not exist yet**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeWiimmsIsoToolsImagePackagerTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan1/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan1/bin/`

Expected: FAIL because `GameCubeWiimmsIsoToolsOptions`, `GameCubeWiimmsIsoToolsImagePackager`, `IGameCubeProcessRunner`, and `GameCubeProcessRunResult` do not exist yet.

- [ ] **Step 3: Implement the explicit options and process-runner boundary**

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Carries the explicit Wiimms ISO Tools executable path used by packaged GameCube image builds.
/// </summary>
public sealed class GameCubeWiimmsIsoToolsOptions {
    /// <summary>
    /// Initializes one explicit Wiimms ISO Tools option set.
    /// </summary>
    /// <param name="executablePath">Configured path to the `wit` executable.</param>
    public GameCubeWiimmsIsoToolsOptions(string executablePath) {
        ExecutablePath = executablePath ?? string.Empty;
    }

    /// <summary>
    /// Gets the configured path to the `wit` executable.
    /// </summary>
    public string ExecutablePath { get; }
}
```

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Carries the result of one external process invocation used by the GameCube builder.
/// </summary>
public sealed class GameCubeProcessRunResult {
    /// <summary>
    /// Initializes one captured process result.
    /// </summary>
    public GameCubeProcessRunResult(int exitCode, string standardOutput, string standardError) {
        ExitCode = exitCode;
        StandardOutput = standardOutput ?? string.Empty;
        StandardError = standardError ?? string.Empty;
    }

    /// <summary>
    /// Gets the process exit code.
    /// </summary>
    public int ExitCode { get; }

    /// <summary>
    /// Gets the captured standard output.
    /// </summary>
    public string StandardOutput { get; }

    /// <summary>
    /// Gets the captured standard error.
    /// </summary>
    public string StandardError { get; }
}
```

```csharp
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
```

```csharp
using System.Diagnostics;

namespace helengine.gamecube.builder;

/// <summary>
/// Runs real external processes for the GameCube builder.
/// </summary>
public sealed class GameCubeProcessRunner : IGameCubeProcessRunner {
    /// <summary>
    /// Runs one configured process and captures exit code, stdout, and stderr.
    /// </summary>
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
```

```csharp
using System.Diagnostics;

namespace helengine.gamecube.builder.tests.Builders;

/// <summary>
/// Returns one preconfigured process result so Wiimms ISO Tools tests can avoid starting real processes.
/// </summary>
public sealed class FakeGameCubeProcessRunner : IGameCubeProcessRunner {
    readonly GameCubeProcessRunResult Result;

    /// <summary>
    /// Initializes the fake runner with one preconfigured process result.
    /// </summary>
    public FakeGameCubeProcessRunner(GameCubeProcessRunResult result) {
        Result = result ?? throw new ArgumentNullException(nameof(result));
    }

    /// <summary>
    /// Returns the preconfigured result without starting a real process.
    /// </summary>
    public GameCubeProcessRunResult Run(ProcessStartInfo startInfo, CancellationToken cancellationToken) {
        cancellationToken.ThrowIfCancellationRequested();
        return Result;
    }
}
```

- [ ] **Step 4: Run the targeted tests to verify the new boundary compiles**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeWiimmsIsoToolsImagePackagerTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan1/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan1/bin/`

Expected: still FAIL because `GameCubeWiimmsIsoToolsImagePackager` is not implemented yet, but the missing-type surface should be reduced to the packager itself.

- [ ] **Step 5: Commit the options and process-runner slice**

```bash
git add builder/GameCubeWiimmsIsoToolsOptions.cs builder/IGameCubeProcessRunner.cs builder/GameCubeProcessRunResult.cs builder/GameCubeProcessRunner.cs builder.tests/Builders/FakeGameCubeProcessRunner.cs builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs
git commit -m "Add GameCube external process runner scaffolding"
```

### Task 2: Implement the `wit` image packager backend

**Files:**
- Create: `builder/GameCubeWiimmsIsoToolsImagePackager.cs`
- Modify: `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`
- Test: `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`

- [ ] **Step 1: Extend the failing tests to cover failed and successful `wit` runs**

```csharp
[Fact]
public void Package_WhenWitReturnsNonZero_ThrowsInvalidOperationException() {
    string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
    string executablePath = Path.Combine(workingRootPath, "wit.exe");
    Directory.CreateDirectory(workingRootPath);
    File.WriteAllText(executablePath, "fake");

    GameCubeWiimmsIsoToolsImagePackager packager = new(
        new GameCubeWiimmsIsoToolsOptions(executablePath),
        new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(7, "stdout", "stderr")));

    InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
        packager.Package(CreateLayout(), Path.Combine(workingRootPath, "game.gcm"), CancellationToken.None));

    Assert.Contains("exit code", exception.Message, StringComparison.OrdinalIgnoreCase);
    Assert.Contains("stderr", exception.Message, StringComparison.OrdinalIgnoreCase);
}

[Fact]
public void Package_WhenWitSucceedsAndImageExists_WritesGameCubeImageArtifact() {
    string workingRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-tests", Guid.NewGuid().ToString("N"));
    string executablePath = Path.Combine(workingRootPath, "wit.exe");
    string outputImagePath = Path.Combine(workingRootPath, "game.gcm");
    Directory.CreateDirectory(workingRootPath);
    File.WriteAllText(executablePath, "fake");

    GameCubeWiimmsIsoToolsImagePackager packager = new(
        new GameCubeWiimmsIsoToolsOptions(executablePath),
        new FakeGameCubeProcessRunner(new GameCubeProcessRunResult(0, "ok", string.Empty)),
        static (layout, destinationImagePath) => File.WriteAllText(destinationImagePath, "gcm"));

    packager.Package(CreateLayout(), outputImagePath, CancellationToken.None);

    Assert.True(File.Exists(outputImagePath));
}
```

- [ ] **Step 2: Run the targeted tests to verify the packager implementation is still missing**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeWiimmsIsoToolsImagePackagerTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan2/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan2/bin/`

Expected: FAIL because `GameCubeWiimmsIsoToolsImagePackager` does not exist yet.

- [ ] **Step 3: Implement the minimal real `wit` packager**

```csharp
using System.Diagnostics;

namespace helengine.gamecube.builder;

/// <summary>
/// Packages one extracted GameCube disc layout into a final image artifact by invoking Wiimms ISO Tools.
/// </summary>
public sealed class GameCubeWiimmsIsoToolsImagePackager : IGameCubeImagePackager {
    readonly GameCubeWiimmsIsoToolsOptions Options;
    readonly IGameCubeProcessRunner ProcessRunner;
    readonly Action<GameCubeDiscLayoutResult, string> SuccessImageWriter;

    /// <summary>
    /// Initializes one Wiimms ISO Tools image packager.
    /// </summary>
    public GameCubeWiimmsIsoToolsImagePackager(
        GameCubeWiimmsIsoToolsOptions options,
        IGameCubeProcessRunner processRunner,
        Action<GameCubeDiscLayoutResult, string> successImageWriter = null) {
        Options = options ?? throw new ArgumentNullException(nameof(options));
        ProcessRunner = processRunner ?? throw new ArgumentNullException(nameof(processRunner));
        SuccessImageWriter = successImageWriter;
    }

    /// <summary>
    /// Packages one extracted GameCube disc layout into the final output image path.
    /// </summary>
    public void Package(GameCubeDiscLayoutResult layout, string outputImagePath, CancellationToken cancellationToken) {
        if (layout == null) {
            throw new ArgumentNullException(nameof(layout));
        } else if (string.IsNullOrWhiteSpace(outputImagePath)) {
            throw new ArgumentException("Output image path is required.", nameof(outputImagePath));
        } else if (string.IsNullOrWhiteSpace(Options.ExecutablePath)) {
            throw new InvalidOperationException("GameCube image packager is not configured. Set HELENGINE_GAMECUBE_WIT_PATH to the installed wit executable.");
        } else if (!File.Exists(Options.ExecutablePath)) {
            throw new FileNotFoundException("Configured GameCube image packager executable was not found.", Options.ExecutablePath);
        }

        string outputDirectoryPath = Path.GetDirectoryName(outputImagePath) ?? throw new InvalidOperationException("Output image directory path could not be resolved.");
        Directory.CreateDirectory(outputDirectoryPath);

        ProcessStartInfo startInfo = new() {
            FileName = Options.ExecutablePath,
            WorkingDirectory = Path.GetDirectoryName(layout.DiscRootPath) ?? layout.DiscRootPath
        };
        startInfo.ArgumentList.Add("COPY");
        startInfo.ArgumentList.Add(layout.DiscRootPath);
        startInfo.ArgumentList.Add(outputImagePath);

        GameCubeProcessRunResult result = ProcessRunner.Run(startInfo, cancellationToken);
        if (result.ExitCode != 0) {
            throw new InvalidOperationException(
                $"Wiimms ISO Tools failed with exit code {result.ExitCode}.{Environment.NewLine}{result.StandardOutput}{Environment.NewLine}{result.StandardError}");
        }

        if (SuccessImageWriter != null) {
            SuccessImageWriter(layout, outputImagePath);
        }

        if (!File.Exists(outputImagePath)) {
            throw new InvalidOperationException("Wiimms ISO Tools reported success but the GameCube image artifact was not created.");
        }
    }
}
```

- [ ] **Step 4: Run the targeted tests to verify the packager behavior**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeWiimmsIsoToolsImagePackagerTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan2/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan2/bin/`

Expected: PASS.

- [ ] **Step 5: Commit the `wit` packager backend**

```bash
git add builder/GameCubeWiimmsIsoToolsImagePackager.cs builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs
git commit -m "Add Wiimms ISO Tools GameCube packager"
```

### Task 3: Route packaged builds through explicit `wit` configuration

**Files:**
- Modify: `builder/GameCubeBuildWorkspace.cs`
- Modify: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
- Modify: `builder/Program.cs`
- Test: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`

- [ ] **Step 1: Extend the packaged workspace test to require explicit `HELENGINE_GAMECUBE_WIT_PATH` configuration**

```csharp
[Fact]
public async Task BuildPackagedAsync_WhenWitPathEnvironmentVariableIsMissing_ThrowsInvalidOperationException() {
    string previousWitPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
    try {
        Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", null);

        await Assert.ThrowsAsync<InvalidOperationException>(() =>
            GameCubeBuildWorkspace.BuildPackagedAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None,
                nativeBuildExecutor,
                null));
    } finally {
        Environment.SetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH", previousWitPath);
    }
}
```

- [ ] **Step 2: Run the workspace tests to verify packaged builder wiring does not satisfy the new contract yet**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan3/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan3/bin/`

Expected: FAIL because `BuildPackagedAsync` does not yet read explicit `wit` configuration or construct the real packager.

- [ ] **Step 3: Update packaged workspace and smoke mode to read explicit `wit` configuration**

```csharp
public static Task<PlatformBuildReport> BuildPackagedAsync(
    PlatformBuildRequest request,
    IPlatformBuildProgressReporter progressReporter,
    IPlatformBuildDiagnosticReporter diagnosticReporter,
    CancellationToken cancellationToken,
    IGameCubeNativeBuildExecutor nativeBuildExecutor,
    IGameCubeImagePackager imagePackager = null) {
    // Existing validation and staging work stays unchanged.

    IGameCubeImagePackager effectiveImagePackager = imagePackager ?? CreateConfiguredImagePackager();
    effectiveImagePackager.Package(discLayout, paths.DiscImagePath, cancellationToken);
}

static IGameCubeImagePackager CreateConfiguredImagePackager() {
    string witPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
    if (string.IsNullOrWhiteSpace(witPath)) {
        throw new InvalidOperationException("GameCube packaged-disc builds require HELENGINE_GAMECUBE_WIT_PATH to point at the installed wit executable.");
    }

    return new GameCubeWiimmsIsoToolsImagePackager(
        new GameCubeWiimmsIsoToolsOptions(witPath),
        new GameCubeProcessRunner());
}
```

```csharp
if (args.Length > 0 && string.Equals(args[0], "--smoke-test", StringComparison.OrdinalIgnoreCase)) {
    string witPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_WIT_PATH");
    Console.WriteLine(string.IsNullOrWhiteSpace(witPath)
        ? "gamecube.builder smoke test entrypoint (wit: missing)"
        : "gamecube.builder smoke test entrypoint (wit: configured)");
    return 0;
}
```

- [ ] **Step 4: Run the packaged workspace tests to verify explicit configuration routing**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter 'FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests' -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan3/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan3/bin/`

Expected: PASS.

- [ ] **Step 5: Commit the packaged workspace routing**

```bash
git add builder/GameCubeBuildWorkspace.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs builder/Program.cs
git commit -m "Wire GameCube packaged builds to explicit wit configuration"
```

### Task 4: Probe real `wit` disc-layout compatibility and add adapter only if needed

**Files:**
- Create if needed: `builder/GameCubeWiimmsDiscLayoutAdapter.cs`
- Create if needed: `builder.tests/GameCubeWiimmsDiscLayoutAdapterTests.cs`
- Modify if needed: `builder/GameCubeWiimmsIsoToolsImagePackager.cs`
- Test: `builder.tests/GameCubeWiimmsDiscLayoutAdapterTests.cs`

- [ ] **Step 1: Run one real `wit` packaging probe against the canonical extracted-disc layout**

Run:

```bash
HELENGINE_GAMECUBE_WIT_PATH="C:/path/to/wit.exe" rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan4/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan4/bin/
```

Expected:

- if `wit` accepts the canonical layout, packaging succeeds and no adapter is needed
- if `wit` rejects the layout, capture the exact error and proceed to the adapter steps below

- [ ] **Step 2: If `wit` rejects canonical layout, write the failing adapter test**

```csharp
public sealed class GameCubeWiimmsDiscLayoutAdapterTests {
    [Fact]
    public void Adapt_CopiesCanonicalLayoutIntoToolSpecificRoot() {
        GameCubeDiscLayoutResult layout = CreateLayout();
        string adaptedRootPath = Path.Combine(Path.GetTempPath(), "gamecube-wit-adapter-tests", Guid.NewGuid().ToString("N"));

        GameCubeWiimmsDiscLayoutAdapter adapter = new();
        string toolRootPath = adapter.Adapt(layout, adaptedRootPath);

        Assert.True(Directory.Exists(toolRootPath));
        Assert.True(File.Exists(Path.Combine(toolRootPath, "sys", "main.dol")));
        Assert.True(File.Exists(Path.Combine(toolRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
    }
}
```

- [ ] **Step 3: Implement the minimal adapter only if the probe required it**

```csharp
namespace helengine.gamecube.builder;

/// <summary>
/// Adapts the canonical extracted GameCube disc layout into a Wiimms ISO Tools staging root when required.
/// </summary>
public sealed class GameCubeWiimmsDiscLayoutAdapter {
    /// <summary>
    /// Copies the canonical extracted layout into a dedicated tool-consumable staging root.
    /// </summary>
    public string Adapt(GameCubeDiscLayoutResult layout, string adaptedRootPath) {
        if (layout == null) {
            throw new ArgumentNullException(nameof(layout));
        } else if (string.IsNullOrWhiteSpace(adaptedRootPath)) {
            throw new ArgumentException("Adapted root path is required.", nameof(adaptedRootPath));
        }

        if (Directory.Exists(adaptedRootPath)) {
            Directory.Delete(adaptedRootPath, recursive: true);
        }

        DirectoryCopy(layout.DiscRootPath, adaptedRootPath);
        return adaptedRootPath;
    }

    static void DirectoryCopy(string sourceRootPath, string destinationRootPath) {
        Directory.CreateDirectory(destinationRootPath);
        string[] filePaths = Directory.GetFiles(sourceRootPath, "*", SearchOption.AllDirectories);
        for (int index = 0; index < filePaths.Length; index++) {
            string sourcePath = filePaths[index];
            string relativePath = Path.GetRelativePath(sourceRootPath, sourcePath);
            string destinationPath = Path.Combine(destinationRootPath, relativePath);
            Directory.CreateDirectory(Path.GetDirectoryName(destinationPath)!);
            File.Copy(sourcePath, destinationPath, true);
        }
    }
}
```

- [ ] **Step 4: Run the adapter tests only if the adapter was required**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeWiimmsDiscLayoutAdapterTests -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan4/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan4/bin/`

Expected: PASS.

- [ ] **Step 5: Commit the adapter slice only if real `wit` behavior required it**

```bash
git add builder/GameCubeWiimmsDiscLayoutAdapter.cs builder.tests/GameCubeWiimmsDiscLayoutAdapterTests.cs builder/GameCubeWiimmsIsoToolsImagePackager.cs
git commit -m "Adapt GameCube disc layout for wit packaging"
```

### Task 5: Document, verify, and boot the real packaged image

**Files:**
- Modify: `README.md`
- Test: `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`
- Test: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
- Verify: packaged-disc Docker build and Dolphin boot

- [ ] **Step 1: Document packaged-build prerequisites and command flow**

```md
## Packaged Disc Build

Packaged GameCube image builds require an installed `wit` executable from Wiimms ISO Tools.

Set:

```powershell
$env:HELENGINE_GAMECUBE_WIT_PATH = 'C:\path\to\wit.exe'
```

Then run the packaged builder flow so it can emit:

- `disc/`
- `game.gcm`

The builder will fail explicitly if `HELENGINE_GAMECUBE_WIT_PATH` is missing or invalid.
```

- [ ] **Step 2: Run the full targeted managed test slice**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter 'FullyQualifiedName~GameCubeWiimmsIsoToolsImagePackagerTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests|FullyQualifiedName~GameCubeDiscPathResolverTests|FullyQualifiedName~GameCubeDiscLayoutWriterTests|FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests' -v minimal -p:BaseIntermediateOutputPath=/tmp/helengine-gc-wit-plan5/obj/ -p:BaseOutputPath=/tmp/helengine-gc-wit-plan5/bin/`

Expected: PASS.

- [ ] **Step 3: Run the packaged-disc native compile verification**

Run:

```bash
rtk docker run --rm -v /mnt/c/dev/helworks/helengine-gc:/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube -e HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc helengine-gc make clean all
```

Expected:

- `build/helengine_gc.dol` produced
- packaged-disc native compile still succeeds

- [ ] **Step 4: Run a real packaged build and boot it in Dolphin**

Run the packaged build with a real installed `wit`, then launch the resulting `game.gcm` in Dolphin.

Expected:

- `output/.../game.gcm` exists
- Dolphin boots the image
- packaged runtime reaches the rotating cube scene

- [ ] **Step 5: Commit the documentation and verified external-packager integration**

```bash
git add README.md builder builder.tests Makefile src/platform/gamecube
git commit -m "Integrate Wiimms ISO Tools for GameCube packaged builds"
```

## Self-Review

Spec coverage:

- explicit external packager choice: covered by Task 2
- explicit launcher-friendly tool discovery: covered by Task 3
- canonical layout with optional tool adapter: covered by Task 4
- diagnostics around missing/bad tool path and failed execution: covered by Tasks 1-3
- runtime compile and Dolphin packaged-image verification: covered by Task 5

Placeholder scan:

- no `TODO` or `TBD` markers remain
- every task includes exact files, code, and commands

Type consistency:

- `GameCubeWiimmsIsoToolsOptions`, `GameCubeProcessRunResult`, `IGameCubeProcessRunner`, `GameCubeWiimmsIsoToolsImagePackager`, and `GameCubeWiimmsDiscLayoutAdapter` are named consistently across tasks
