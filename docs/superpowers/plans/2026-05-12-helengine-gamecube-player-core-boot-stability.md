# GameCube Player Core Boot Stability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce a generated-core GameCube build that boots in Dolphin, completes `Core::Initialize(...)`, and keeps running `Core::Update()` and `Core::Draw()` without crashing.

**Architecture:** The implementation keeps the platform boundary explicit. `csharpcodegen` exposes a named GameCube preset that emits big-endian GCC-targeted generated core output, while `helengine-gc` tightens `GameCubeApplication` into explicit boot phases with deterministic failure states and a strict generated-core compile path. Verification stays end-to-end: generate core, build the `.dol`, then validate runtime in Dolphin.

**Tech Stack:** .NET 9, xUnit, `csharpcodegen` CLI, C++17, devkitPro, libogc, Docker, Dolphin

---

## File Structure

- `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/CPPConversionPresetCatalog.cs`
  Owns named C++ conversion presets.
- `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPConversionPresetCatalogTests.cs`
  Verifies named preset resolution.
- `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPFeaturePruningEndToEndTests.cs`
  Verifies generated output shape for named presets.
- `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/README.md`
  Documents the GameCube-targeted code generation flow.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootPhase.hpp`
  Declares the native boot phase model used by the GameCube host.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.hpp`
  Declares explicit boot-phase methods and generated-core state owned by the host.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
  Implements native init, generated-core bootstrap, phase-specific failure handling, and the steady-state loop.
- `/mnt/c/dev/helworks/helengine-gc/README.md`
  Documents the generated-core build and Dolphin verification flow.
- `/mnt/c/dev/helworks/helengine-gc/Makefile`
  Already owns the generated-core compile path; verification steps in this plan exercise that path without broadening build scope.

### Task 1: Expose a named GameCube core-boot preset in `csharpcodegen`

**Files:**
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPConversionPresetCatalogTests.cs`
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPFeaturePruningEndToEndTests.cs`
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/CPPConversionPresetCatalog.cs`
- Test: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj`

- [ ] **Step 1: Add failing preset-resolution coverage**

```csharp
[Fact]
public void Resolve_GameCubeCoreBoot_UsesNamedPresetProfiles() {
    CPPConversionPreset preset = new CPPConversionPresetCatalog().Resolve("gamecube-core-boot");

    Assert.Equal("gamecube-core-boot", preset.Id);
    Assert.Equal("gcc", preset.CompilerProfile.Name);
    Assert.Equal("gamecube-headless", preset.PlatformProfile.Name);
    Assert.Equal("stl-lite", preset.RuntimeProfile.Name);
    Assert.Equal(CPPFeatureMode.Disabled, preset.BuildFeatureProfile.GetMode(CPPFeatureKind.Shaders));
    Assert.Equal(CPPFeatureMode.Disabled, preset.BuildFeatureProfile.GetMode(CPPFeatureKind.DebugOverlay));
    Assert.Equal("gamecube-core-boot", preset.RestrictionProfile.Name);
    Assert.True(preset.RestrictionProfile.ForbidShaders);
    Assert.True(preset.RestrictionProfile.ForbidDebugOnlySystems);
}
```

- [ ] **Step 2: Add failing end-to-end output coverage for the GameCube preset**

```csharp
[Fact]
public void WriteOutput_WhenPresetIsGameCubeCoreBoot_WritesGameCubeConfigAndDisablesShaders() {
    string source = """
namespace helengine.core.shaders {
    public class ShaderAsset {
    }
}

namespace helengine.core.scene {
    public class SceneNode {
    }
}
""";

    string outputPath = RunConversionWithPreset(source, "gamecube-core-boot");
    string configPath = Path.Combine(outputPath, "helcpp_config.hpp");
    string config = File.ReadAllText(configPath);

    Assert.Contains("#define HE_CPP_COMPILER_GCC 1", config);
    Assert.Contains("#define HE_CPP_PLATFORM_GAMECUBE 1", config);
    Assert.Contains("#define HE_CPP_PLATFORM_IS_LITTLE_ENDIAN 0", config);
    Assert.Contains("#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0", config);
    Assert.Contains("#define HE_CPP_FEATURE_SHADERS 0", config);
    Assert.Contains("#define HE_CPP_FEATURE_DEBUGOVERLAY 0", config);
}
```

- [ ] **Step 3: Run the focused preset tests to verify they fail first**

Run: `rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionPresetCatalogTests|FullyQualifiedName~CPPFeaturePruningEndToEndTests" -v minimal`

Expected: FAIL with an unknown preset error for `gamecube-core-boot`.

- [ ] **Step 4: Implement the named GameCube core-boot preset**

```csharp
if (string.Equals(presetId, "gamecube-core-boot", StringComparison.OrdinalIgnoreCase)) {
    return CreateGameCubeCoreBootPreset();
}

static CPPConversionPreset CreateGameCubeCoreBootPreset() {
    CPPBuildFeatureProfile featureProfile = CPPBuildFeatureProfile.CreateDefault()
        .WithMode(CPPFeatureKind.Shaders, CPPFeatureMode.Disabled)
        .WithMode(CPPFeatureKind.DebugOverlay, CPPFeatureMode.Disabled);

    return new CPPConversionPreset {
        Id = "gamecube-core-boot",
        CompilerProfile = CPPCompilerProfile.CreateGcc(),
        PlatformProfile = CPPPlatformProfile.CreateGameCubeHeadless(),
        RuntimeProfile = CPPRuntimeProfile.CreateStlLite(),
        BuildFeatureProfile = featureProfile,
        RestrictionProfile = new CPPRestrictionProfile {
            Name = "gamecube-core-boot",
            ForbidShaders = true,
            ForbidDebugOnlySystems = true
        }
    };
}
```

- [ ] **Step 5: Run the focused preset tests again**

Run: `rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionPresetCatalogTests|FullyQualifiedName~CPPFeaturePruningEndToEndTests" -v minimal`

Expected: PASS.

- [ ] **Step 6: Commit the preset work**

```bash
git -C /mnt/c/dev/helworks/csharpcodegen add cs2.cpp/CPPConversionPresetCatalog.cs cs2.cpp.tests/CPPConversionPresetCatalogTests.cs cs2.cpp.tests/CPPFeaturePruningEndToEndTests.cs
git -C /mnt/c/dev/helworks/csharpcodegen commit -m "Add GameCube core boot conversion preset"
```

### Task 2: Refactor `GameCubeApplication` into explicit boot phases

**Files:**
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootPhase.hpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.hpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
- Test: `/mnt/c/dev/helworks/helengine-gc/Makefile`

- [ ] **Step 1: Declare the boot-phase model**

```cpp
#pragma once

namespace helengine::gamecube {
    /// Identifies the current GameCube host boot phase for deterministic runtime diagnostics.
    enum class GameCubeBootPhase {
        NativeVideo,
        NativeGraphics,
        CoreConstruction,
        CoreOptions,
        BridgeConstruction,
        CoreInitialization,
        CoreUpdate,
        CoreDraw,
        Running,
        Failed
    };
}
```

- [ ] **Step 2: Expand `GameCubeApplication.hpp` with phase-owned methods and state**

```cpp
#pragma once

#include <gccore.h>

#include "platform/gamecube/GameCubeBootPhase.hpp"

class Core;

namespace helengine::gamecube {
    class GameCubeInputManager;
    class GameCubeRenderManager2D;
    class GameCubeRenderManager3D;

    /// Owns GameCube host startup, generated-core boot, and steady-state runtime diagnostics.
    class GameCubeApplication {
    public:
        /// Creates the GameCube application with no initialized native or engine state.
        GameCubeApplication();

        /// Releases generated-core bridge objects after the application loop finishes.
        ~GameCubeApplication();

        /// Initializes the native host and enters the steady-state frame loop.
        int Run();

    private:
        /// Initializes the VI display state and allocates the first framebuffer.
        bool InitializeVideo();

        /// Initializes GX for the host clear-and-present loop.
        bool InitializeGraphics();

        /// Constructs and initializes the generated engine core when generated sources are enabled.
        bool InitializeEngineCore();

        /// Advances the generated engine state by one update tick.
        bool UpdateEngineCore();

        /// Executes one generated draw tick.
        bool DrawEngineCore();

        /// Presents one frame to the active framebuffer.
        void PresentFrame();

        /// Presents the current failure color forever after a boot-phase failure.
        void PresentFailureLoop();

        /// Sets the current boot phase and its diagnostic clear color.
        void SetBootPhase(GameCubeBootPhase phase, GXColor color);

        /// Marks the host as failed at one specific boot phase.
        void FailBootPhase(GameCubeBootPhase phase, GXColor color);

        GXRModeObj* RenderMode;
        void* FrameBuffer;
        void* FifoBuffer;
        GXColor ClearColor;
        GameCubeBootPhase BootPhase;
        bool EngineInitialized;

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        Core* EngineCore;
        GameCubeRenderManager3D* EngineRenderManager3D;
        GameCubeRenderManager2D* EngineRenderManager2D;
        GameCubeInputManager* EngineInputManager;
#endif
    };
}
```

- [ ] **Step 3: Build the Docker image used by the native GameCube build**

Run: `rtk docker build -t helengine-gc .`

Expected: PASS and create the local `helengine-gc` image.

- [ ] **Step 4: Run the native build after the header change to verify it fails before implementation catches up**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make clean all`

Expected: FAIL because `GameCubeApplication.cpp` does not yet implement the new declarations.

- [ ] **Step 5: Implement explicit phase transitions and phase-specific failure boundaries**

```cpp
int GameCubeApplication::Run() {
    SetBootPhase(GameCubeBootPhase::NativeVideo, GXColor { 0x20, 0x20, 0x20, 0xFF });
    if (!InitializeVideo()) {
        FailBootPhase(GameCubeBootPhase::NativeVideo, GXColor { 0x80, 0x00, 0x80, 0xFF });
        PresentFailureLoop();
        return 1;
    }

    SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x40, 0x40, 0x40, 0xFF });
    if (!InitializeGraphics()) {
        FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x80, 0x00, 0x80, 0xFF });
        PresentFailureLoop();
        return 1;
    }

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
    if (!InitializeEngineCore()) {
        PresentFailureLoop();
        return 1;
    }

    while (true) {
        if (!UpdateEngineCore()) {
            PresentFailureLoop();
            return 1;
        }

        if (!DrawEngineCore()) {
            PresentFailureLoop();
            return 1;
        }

        PresentFrame();
    }
#else
    while (true) {
        PresentFrame();
    }
#endif
}

bool GameCubeApplication::InitializeEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
    SetBootPhase(GameCubeBootPhase::CoreConstruction, GXColor { 0xFF, 0xFF, 0x00, 0xFF });
    EngineCore = new Core();

    SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x80, 0x00, 0xFF });
    CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
    if (options == nullptr) {
        FailBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
        return false;
    }

    options->ContentRootPath = ".";
    options->UpdateOrderLayers = 4;
    options->RenderOrderLayers3D = 4;
    options->UpdateListInitialCapacity = 64;
    options->RenderList2DInitialCapacity = 64;
    options->RenderList3DInitialCapacity = 64;

    SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x00, 0xFF, 0xFF, 0xFF });
    EngineRenderManager3D = new GameCubeRenderManager3D();
    EngineRenderManager2D = new GameCubeRenderManager2D();
    EngineInputManager = new GameCubeInputManager();
    EngineInputManager->SetKeyboardActive(true);
    EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);

    SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0x00, 0x00, 0xFF, 0xFF });
    EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, options);

    EngineInitialized = true;
    SetBootPhase(GameCubeBootPhase::Running, GXColor { 0x00, 0xFF, 0x00, 0xFF });
    return true;
#else
    return true;
#endif
}

bool GameCubeApplication::UpdateEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
    if (!EngineInitialized || EngineCore == nullptr) {
        FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
        return false;
    }

    SetBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0x00, 0xA0, 0x00, 0xFF });
    EngineCore->Update();
    SetBootPhase(GameCubeBootPhase::Running, GXColor { 0x00, 0xFF, 0x00, 0xFF });
    return true;
#else
    return true;
#endif
}

bool GameCubeApplication::DrawEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
    if (!EngineInitialized || EngineCore == nullptr) {
        FailBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0xFF, 0x00, 0x00, 0xFF });
        return false;
    }

    SetBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0x00, 0x60, 0x00, 0xFF });
    EngineCore->Draw();
    SetBootPhase(GameCubeBootPhase::Running, GXColor { 0x00, 0xFF, 0x00, 0xFF });
    return true;
#else
    return true;
#endif
}
```

- [ ] **Step 6: Run the host-only build to verify the refactor compiles cleanly**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make clean all`

Expected: PASS and emit `build/helengine_gc.dol`.

- [ ] **Step 7: Commit the boot-phase refactor**

```bash
git -C /mnt/c/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeBootPhase.hpp src/platform/gamecube/GameCubeApplication.hpp src/platform/gamecube/GameCubeApplication.cpp
git -C /mnt/c/dev/helworks/helengine-gc commit -m "Refactor GameCube host boot phases"
```

### Task 3: Document the generated-core build and Dolphin runtime flow

**Files:**
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/README.md`
- Modify: `/mnt/c/dev/helworks/helengine-gc/README.md`

- [ ] **Step 1: Add the GameCube code-generation command to `cs2.cpp/README.md`**

````markdown
## GameCube core-boot generation

Generate GameCube-targeted core output with the named preset:

```bash
rtk dotnet run --project /mnt/c/dev/helworks/csharpcodegen/codegen/codegen.csproj -- --cpp --project /mnt/c/dev/helworks/helengine/engine/helengine.core/helengine.core.csproj --output /mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube --platform gamecube --compiler gcc --endianness big --preset gamecube-core-boot
```

Expected artifacts:

- `helengine_core_unity.cpp`
- `helcpp_config.hpp`
- `cpp-conversion-report.json`
````

- [ ] **Step 2: Add the generated-core build and Dolphin verification flow to `helengine-gc/README.md`**

````markdown
## Generated core build

Generate GameCube-targeted core output:

```bash
rtk dotnet run --project /mnt/c/dev/helworks/csharpcodegen/codegen/codegen.csproj -- --cpp --project /mnt/c/dev/helworks/helengine/engine/helengine.core/helengine.core.csproj --output /mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube --platform gamecube --compiler gcc --endianness big --preset gamecube-core-boot
```

Build the GameCube player with generated core enabled:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all
```

The build emits `build/helengine_gc.dol`.

## Dolphin verification

Load `build/helengine_gc.dol` in Dolphin.

Expected result for this milestone:

- the application reaches the steady-state green phase
- `Core::Initialize(...)` completes
- the process does not immediately crash during `Update()` or `Draw()`
````

- [ ] **Step 3: Commit the documentation updates**

```bash
git -C /mnt/c/dev/helworks/csharpcodegen add cs2.cpp/README.md
git -C /mnt/c/dev/helworks/csharpcodegen commit -m "Document GameCube core boot generation"
git -C /mnt/c/dev/helworks/helengine-gc add README.md
git -C /mnt/c/dev/helworks/helengine-gc commit -m "Document GameCube generated core boot flow"
```

### Task 4: Run end-to-end generation, build, and Dolphin verification

**Files:**
- Generate: `/mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube/*`
- Build: `/mnt/c/dev/helworks/helengine-gc/build/helengine_gc.dol`

- [ ] **Step 1: Run the focused `csharpcodegen` test suite before generating output**

Run: `rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionPresetCatalogTests|FullyQualifiedName~CPPFeaturePruningEndToEndTests|FullyQualifiedName~CPPGeneratedConfigWriterTests" -v minimal`

Expected: PASS.

- [ ] **Step 2: Generate GameCube-targeted core output**

Run: `rtk dotnet run --project /mnt/c/dev/helworks/csharpcodegen/codegen/codegen.csproj -- --cpp --project /mnt/c/dev/helworks/helengine/engine/helengine.core/helengine.core.csproj --output /mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube --platform gamecube --compiler gcc --endianness big --preset gamecube-core-boot`

Expected: PASS with `C++ conversion completed.` and fresh `helengine_core_unity.cpp`, `helcpp_config.hpp`, and `cpp-conversion-report.json` under `tmp/generated-core-gamecube/`.

- [ ] **Step 3: Rebuild the Docker image so the generated-core build uses the latest host sources**

Run: `rtk docker build -t helengine-gc .`

Expected: PASS and refresh the local `helengine-gc` image.

- [ ] **Step 4: Build the generated-core GameCube player**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all`

Expected: PASS and emit `build/helengine_gc.dol`.

- [ ] **Step 5: Validate the `.dol` in Dolphin**

Run: Load `/mnt/c/dev/helworks/helengine-gc/build/helengine_gc.dol` in Dolphin.

Expected:

- the host progresses through the diagnostic boot phases instead of immediately freezing at startup
- the steady-state runtime reaches the green running phase
- the process remains alive past repeated `Core::Update()` and `Core::Draw()` frames
