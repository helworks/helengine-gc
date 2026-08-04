# GameCube First-Frame Trace Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add one default-off GameCube build option that traces every first-frame boundary on the console and holds the last reached checkpoint color.

**Architecture:** The builder resolves one boolean into a native make macro. `GameCubeApplication` exposes one diagnostic checkpoint helper that presents a color only when the macro is enabled. The first normal frame invokes the helper around every boundary, and a successful trace enters a presentation-only loop.

**Tech Stack:** C#/.NET 9 builder tests, C++20/libogc GameCube runtime, Make, Docker devkitPPC image.

---

### Task 1: Define the builder contract

**Files:**
- Modify: `builder.tests/GameCubePlatformDefinitionFactoryTests.cs`
- Modify: `builder.tests/GameCubeDockerNativeBuildExecutorTests.cs`
- Modify: `builder/GameCubePlatformDefinitionFactory.cs`
- Modify: `builder/GameCubeBuilderPaths.cs`
- Modify: `builder/GameCubeDockerNativeBuildExecutor.cs`

- [ ] **Step 1: Write the failing builder tests**

```csharp
[Fact]
public void Create_WhenDefaultBuildProfileIsResolved_PublishesDisabledFirstFrameTraceDiagnosticSetting() {
    PlatformDefinition definition = GameCubePlatformDefinitionFactory.Create();
    PlatformBuildProfileDefinition buildProfile = Assert.Single(definition.BuildProfiles);

    PlatformSettingDefinition setting = Assert.Single(
        buildProfile.Settings,
        candidate => candidate.Id == "enable-first-frame-trace-diagnostic");

    Assert.Equal(PlatformSettingKind.Boolean, setting.Kind);
    Assert.Equal("false", setting.DefaultValue);
}

[Fact]
public void Source_ExportsResolvedFirstFrameTraceDiagnosticSetting() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string source = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));

    Assert.Contains("HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC=", source, StringComparison.Ordinal);
    Assert.Contains("paths.FirstFrameTraceDiagnosticEnabled", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Verify the tests fail because the setting and environment export do not exist**

Run: `rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~FirstFrameTraceDiagnostic" --verbosity minimal`

Expected: both tests fail with the missing setting ID and missing environment-variable text.

- [ ] **Step 3: Implement the resolved builder option**

Add the setting after the two existing diagnostic settings:

```csharp
new PlatformSettingDefinition(
    "enable-first-frame-trace-diagnostic",
    "Enable First-Frame Trace Diagnostic",
    PlatformSettingKind.Boolean,
    "false",
    true,
    [])
```

Extend `GameCubeBuilderPaths.Create` and its constructor with `ResolveFirstFrameTraceDiagnosticEnabled(request)`, store it as `FirstFrameTraceDiagnosticEnabled`, and resolve it through the existing `ResolveOptionalBooleanBuildOption` helper. Forward the value in `GameCubeDockerNativeBuildExecutor` as:

```csharp
"HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC="
    + (paths.FirstFrameTraceDiagnosticEnabled ? "1" : "0")
```

- [ ] **Step 4: Verify the builder tests pass**

Run: `rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~FirstFrameTraceDiagnostic" --verbosity minimal`

Expected: 2 passed.

### Task 2: Add the native first-frame trace

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `Makefile`
- Modify: `src/platform/gamecube/GameCubeApplication.hpp`
- Modify: `src/platform/gamecube/GameCubeApplication.cpp`

- [ ] **Step 1: Write the failing native source-contract test**

```csharp
[Fact]
public void FirstFrameTraceDiagnosticSource_PresentsEveryFirstFrameBoundaryAndStopsAfterSuccess() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
    string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));
    string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));

    Assert.Contains("HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
    Assert.Contains("PresentFirstFrameTraceCheckpoint", applicationSource, StringComparison.Ordinal);
    Assert.Contains("FirstFrameTraceCompleted", applicationSource, StringComparison.Ordinal);
    Assert.Contains("PresentFirstFrameTraceCheckpoint", applicationHeaderSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Verify the test fails because the macro and helper do not exist**

Run: `rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~FirstFrameTraceDiagnosticSource" --verbosity minimal`

Expected: failure on `HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC ?= 0`.

- [ ] **Step 3: Implement the minimal trace**

Define `HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC ?= 0` and add it to `CPPFLAGS`. Add documented `PresentFirstFrameTraceCheckpoint(GXColor color)` and `FirstFrameTraceCompleted` to `GameCubeApplication`. When enabled, the helper must set the clear color and call `PresentBootFrame`; otherwise it must compile to no behavior.

Call the helper at these exact boundaries in the first normal frame, with distinct colors:

```cpp
// Before BeginFrame: violet.
PresentFirstFrameTraceCheckpoint(GXColor { 0xA0, 0x20, 0xFF, 0xFF });
EngineRenderManager2D->BeginFrame();

// Before Update: cyan.
PresentFirstFrameTraceCheckpoint(GXColor { 0x20, 0xE0, 0xFF, 0xFF });
EngineCore->Update();

// Before texture flush: yellow.
PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0xE0, 0x20, 0xFF });

// Before asset flush: orange.
PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0x80, 0x20, 0xFF });

// Before Draw: magenta.
PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0x20, 0xC0, 0xFF });
EngineCore->Draw();

// First frame completed: white.
PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0xFF, 0xFF, 0xFF });
FirstFrameTraceCompleted = true;
```

At the top of the next loop iteration, when `FirstFrameTraceCompleted` is true, retain the final white frame in a presentation-only loop. Disabled builds keep the current normal loop exactly.

- [ ] **Step 4: Verify the native source-contract test passes**

Run: `rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~FirstFrameTraceDiagnosticSource" --verbosity minimal`

Expected: 1 passed.

### Task 3: Verify and package the single console test

**Files:**
- Modify temporarily then restore: `C:\dev\helprojs\demodisc\settings\platform.gamecube.json`

- [ ] **Step 1: Run all first-frame trace tests**

Run: `rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~FirstFrameTraceDiagnostic" --verbosity minimal`

Expected: all first-frame trace tests pass.

- [ ] **Step 2: Compile a minimal native configuration**

Run: `rtk docker run --rm -v C:\dev\helworks\helengine-gc:/workspace -w /workspace -e HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC=1 helengine-gc make clean all HELENGINE_GAMECUBE_MINIMAL_SAMPLE=1`

Expected: `build\helengine_gc.dol` is produced with the trace macro equal to `1`.

- [ ] **Step 3: Package the trace once**

Temporarily set only:

```json
"selectedOptionValues": {
  "enable-first-frame-trace-diagnostic": "true"
}
```

Build `demodisc` to `C:\dev\helprojs\demodisc\output\gamecube-first-frame-trace-20260723`, then restore `selectedOptionValues` to `{}`.

- [ ] **Step 4: Verify the packaged output**

Run: `rtk powershell -NoProfile -Command "Get-FileHash -Algorithm SHA256 -LiteralPath 'C:\dev\helprojs\demodisc\output\gamecube-first-frame-trace-20260723\game.gcm'"`

Expected: a non-empty GCM and a SHA-256 value, with packaging logs showing `HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC=1`, `HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL=0`, and `HELENGINE_GAMECUBE_NINTENDONT_HANDOFF_DIAGNOSTIC=0`.
