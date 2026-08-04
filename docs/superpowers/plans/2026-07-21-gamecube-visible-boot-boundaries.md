# GameCube Visible Boot Boundaries Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make early GameCube boot boundaries visibly distinguishable on vWii/Nintendont without changing normal frame rendering.

**Architecture:** `GameCubeApplication` gains one private boot-frame presenter. It writes a solid RGB phase colour to the VI framebuffers before GX is initialized, then uses the already configured GX display-copy route for subsequent boot boundaries. Explicit calls are placed immediately before every risky initialization action; the regular `PresentFrame` loop remains unchanged.

**Tech Stack:** C++17, libogc VIDEO/GX APIs, xUnit source-contract tests.

---

### Task 1: Guard the visible-boundary source contract

**Files:**

- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs:8-57`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs:8-57`

- [ ] **Step 1: Add the failing test assertions**

Add the following assertions to `PackagedDiscBootSource_UsesPackagedDefineAndBootstrapHelpers` after the existing `GX_CopyDisp` assertion:

```csharp
Assert.Contains("void PresentBootFrame();", applicationHeaderSource, StringComparison.Ordinal);
Assert.Contains("static uint32_t ConvertToVideoClearColor(const GXColor& color);", applicationHeaderSource, StringComparison.Ordinal);
Assert.Contains("PresentBootFrame();\n        if (!InitializeGraphics())", applicationSource, StringComparison.Ordinal);
Assert.Contains("VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], ConvertToVideoClearColor(ClearColor));", applicationSource, StringComparison.Ordinal);
Assert.Contains("GX_SetCopyClear(ClearColor, 0x00FFFFFF);", applicationSource, StringComparison.Ordinal);
Assert.Contains("PresentBootFrame();\n            EngineCore = new Core();", applicationSource, StringComparison.Ordinal);
Assert.Contains("PresentBootFrame();\n            EngineCore->Initialize(", applicationSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused test to verify RED**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --no-restore --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedDiscBootSource_UsesPackagedDefineAndBootstrapHelpers" --verbosity minimal
```

Expected: the test fails because `PresentBootFrame` and `ConvertToVideoClearColor` are absent.

### Task 2: Implement safe VI/GX boot-frame presentation

**Files:**

- Modify: `src/platform/gamecube/GameCubeApplication.hpp:47-93`
- Modify: `src/platform/gamecube/GameCubeApplication.cpp:205-330`

- [ ] **Step 1: Add the private presenter declarations**

Add these declarations after `DrawMinimalSample` in `GameCubeApplication.hpp`:

```cpp
/// Presents the current boot-phase color through VI or GX, depending on which native subsystem is ready.
void PresentBootFrame();

/// Converts one RGB diagnostic color into the packed YCbYCr framebuffer value required by VIDEO_ClearFrameBuffer.
static uint32_t ConvertToVideoClearColor(const GXColor& color);
```

- [ ] **Step 2: Add the minimal presenter implementation**

Add these definitions after `InitializeVideo` in `GameCubeApplication.cpp`:

```cpp
void GameCubeApplication::PresentBootFrame() {
    if (!GraphicsInitialized) {
        const uint32_t videoClearColor = ConvertToVideoClearColor(ClearColor);
        VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], videoClearColor);
        VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[1], videoClearColor);
        VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        return;
    }

    GX_SetCopyClear(ClearColor, 0x00FFFFFF);
    GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);
    GX_DrawDone();
    VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

uint32_t GameCubeApplication::ConvertToVideoClearColor(const GXColor& color) {
    const double red = static_cast<double>(color.r);
    const double green = static_cast<double>(color.g);
    const double blue = static_cast<double>(color.b);
    const uint8_t luminance = static_cast<uint8_t>(std::clamp(static_cast<int32_t>(16.0 + (0.257 * red) + (0.504 * green) + (0.098 * blue)), 0, 255));
    const uint8_t chromaBlue = static_cast<uint8_t>(std::clamp(static_cast<int32_t>(128.0 - (0.148 * red) - (0.291 * green) + (0.439 * blue)), 0, 255));
    const uint8_t chromaRed = static_cast<uint8_t>(std::clamp(static_cast<int32_t>(128.0 + (0.439 * red) - (0.368 * green) - (0.071 * blue)), 0, 255));
    return (static_cast<uint32_t>(luminance) << 24)
        | (static_cast<uint32_t>(chromaBlue) << 16)
        | (static_cast<uint32_t>(luminance) << 8)
        | static_cast<uint32_t>(chromaRed);
}
```

- [ ] **Step 3: Track GX readiness explicitly**

Add `GraphicsInitialized` as a `bool` field after `EngineInitialized` in the header, initialize it to `false` in the constructor, and set it to `true` only as the final successful action in `InitializeGraphics`.

- [ ] **Step 4: Present each early boundary before entering its risky operation**

Call `PresentBootFrame()` immediately after setting `NativeGraphics` and before `InitializeGraphics`. In `InitializeEngineCore`, call it immediately after the phase assignment and before each of these operations:

```cpp
EngineCore = new Core();
CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
if (!GameCubeSceneBootstrap::InitializePackagedDisc()) {
EngineRenderManager3D = new GameCubeRenderManager3D();
EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, EnginePlatformInfo, options);
EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);
```

Do not add calls inside `UpdateEngineCore`, `DrawEngineCore`, or `PresentFrame`.

- [ ] **Step 5: Run the focused test to verify GREEN**

Run the same command from Task 1. Expected: PASS.

### Task 3: Build the native diagnostic artifact

**Files:**

- No source changes.

- [ ] **Step 1: Build the packaged GameCube host**

Run the repository’s existing Docker GameCube build with the same generated-core root used by the package builder. Use the smallest existing packaged build command and preserve its output directory.

Expected: native compilation and DOL conversion succeed.

- [ ] **Step 2: Rebuild the demodisc packaged image into a new timestamped output directory**

Run the normal headless GameCube project build, preserving the previously tested `output/gamecube` artifact.

Expected: a new `game.gcm` is produced with the diagnostic host.

- [ ] **Step 3: Perform hardware validation**

Launch the newly produced image through Nintendont on vWii and record the final persistent colour:

- black: image entry or pre-VI initialization boundary
- blue: GX initialization boundary
- yellow: `Core` construction boundary
- orange: core-options boundary
- dark blue: packaged-disc bootstrap boundary
- cyan: bridge construction boundary
- bright blue: `Core::Initialize` boundary
- purple-blue: scene-load boundary
- green: initialization completed; inspect the render path next

