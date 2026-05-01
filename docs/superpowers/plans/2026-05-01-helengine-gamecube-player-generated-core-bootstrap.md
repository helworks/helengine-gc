# GameCube Player Generated-Core Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first `helengine-gc` player milestone that can compile and boot converted `helengine.core` C++ output on GameCube by pairing a GameCube-aware `cs2.cpp` profile with a GameCube-native player host.

**Architecture:** The work is split across `csharpcodegen` and `helengine-gc`. First, `cs2.cpp` gains a real `GCC + GameCubeHeadless` target profile so generated output stops advertising itself as Windows-headless. Then `helengine-gc` evolves from a green-screen bootstrap into a small GameCube player host that conditionally includes generated core, constructs minimal native render/input bridge objects, and calls `Core::Initialize()`, `Update()`, and `Draw()` inside the Dolphin loop.

**Tech Stack:** C#, `cs2.cpp`, devkitPro `devkitPPC`, libogc, GNU Make, C++17, Dolphin

---

## File Structure

- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPPlatformProfile.cs`
  Purpose: add a concrete GameCube target profile factory with correct endianness and host flags.
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPConversionOptions.cs`
  Purpose: expose a first-class option path for non-Windows profile selection without breaking the existing Windows default.
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/CPPProgram.cs`
  Purpose: add a helper entry point that prepares a GameCube conversion option set when needed.
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPConversionOptionsTests.cs`
  Purpose: verify GameCube profile selection and preserve the Windows default behavior.
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPGeneratedConfigWriterTests.cs`
  Purpose: verify generated config output reflects the GameCube profile instead of Windows.
- Modify: `/mnt/c/dev/helworks/helengine-gc/Makefile`
  Purpose: validate generated-core roots and compile `helengine_core_unity.cpp` when generated core is enabled.
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/main.cpp`
  Purpose: enter a GameCube player host rather than the current proof-of-life bootstrap directly.
- Replace: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootHost.hpp`
  Purpose: turn the current boot host into a small application-style player runner or split it into a new application host.
- Replace: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootHost.cpp`
  Purpose: own native startup, optional engine-core initialization, and the main frame loop.
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.hpp`
  Purpose: declare the minimal GameCube 3D bridge required by generated `Core`.
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.cpp`
  Purpose: implement the minimum `RenderManager3D` contract while preserving the current clear/present loop.
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager2D.hpp`
  Purpose: declare the minimal GameCube 2D bridge required by generated `Core`.
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager2D.cpp`
  Purpose: implement no-op or minimal `RenderManager2D` methods sufficient for initialization.
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeInputManager.hpp`
  Purpose: declare the minimal generated `InputManager` bridge and native keyboard/mouse backends.
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeInputManager.cpp`
  Purpose: provide default-state `Keyboard`, `Mouse`, and `InputManager` behavior sufficient for first boot.
- Modify: `/mnt/c/dev/helworks/helengine-gc/README.md`
  Purpose: document the optional generated-core inclusion path and the first player-boot verification target.

### Task 1: Add A Real GameCube Codegen Profile

**Files:**
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPPlatformProfile.cs`
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPConversionOptions.cs`
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPConversionOptionsTests.cs`

- [ ] **Step 1: Inspect the current compiler/platform defaults before changing them**

Run:

```bash
rtk sed -n '1,220p' /mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPCompilerProfile.cs
rtk sed -n '1,220p' /mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPPlatformProfile.cs
rtk sed -n '1,220p' /mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPConversionOptions.cs
```

Expected: GCC support already exists, Windows-headless is the only concrete platform profile, and the default conversion options still point at MSVC + Windows.

- [ ] **Step 2: Add the failing option/profile tests first**

Update `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPConversionOptionsTests.cs` with tests equivalent to:

```csharp
[Fact]
public void CreateDefault_UsesExistingWindowsHeadlessDefaults() {
    CPPConversionOptions options = CPPConversionOptions.CreateDefault();

    Assert.Equal(CPPCompilerKind.Msvc, options.CompilerProfile.Kind);
    Assert.Equal(CPPPlatformKind.WindowsHeadless, options.PlatformProfile.Kind);
}

[Fact]
public void CreateGameCube_DefaultsToGccAndGameCubeHeadless() {
    CPPConversionOptions options = CPPConversionOptions.CreateGameCubeDefault();

    Assert.Equal(CPPCompilerKind.Gcc, options.CompilerProfile.Kind);
    Assert.Equal(CPPPlatformKind.GameCubeHeadless, options.PlatformProfile.Kind);
    Assert.False(options.PlatformProfile.IsLittleEndian);
    Assert.False(options.PlatformProfile.IsWindowsHost);
}
```

- [ ] **Step 3: Run the targeted tests to verify the new GameCube path fails first**

Run:

```bash
rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionOptionsTests" -v minimal
```

Expected: FAIL because `CreateGameCubeDefault()` and `CreateGameCubeHeadless()` do not exist yet.

- [ ] **Step 4: Implement the GameCube profile factory and option helper**

Update `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPPlatformProfile.cs` to add:

```csharp
public static CPPPlatformProfile CreateGameCubeHeadless() {
    return new CPPPlatformProfile {
        Kind = CPPPlatformKind.GameCubeHeadless,
        Name = "gamecube-headless",
        DefineName = "HE_CPP_PLATFORM_GAMECUBE",
        IsLittleEndian = false,
        IsWindowsHost = false
    };
}
```

Update `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp/model/CPPConversionOptions.cs` to add:

```csharp
public static CPPConversionOptions CreateGameCubeDefault() {
    return new CPPConversionOptions {
        CompilerProfile = CPPCompilerProfile.CreateGcc(),
        PlatformProfile = CPPPlatformProfile.CreateGameCubeHeadless(),
        RuntimeProfile = CPPRuntimeProfile.CreateStlLite(),
        CollectDiagnostics = true,
        BuildFeatureProfile = CPPBuildFeatureProfile.CreateDefault(),
        LoadNativeRuntimeMetadata = true
    };
}
```

- [ ] **Step 5: Re-run the targeted tests to verify the profile support now passes**

Run:

```bash
rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionOptionsTests" -v minimal
```

Expected: PASS.

- [ ] **Step 6: Commit the generator profile support**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/csharpcodegen add cs2.cpp/model/CPPPlatformProfile.cs cs2.cpp/model/CPPConversionOptions.cs cs2.cpp.tests/CPPConversionOptionsTests.cs
rtk git -C /mnt/c/dev/helworks/csharpcodegen commit -m "Add GameCube cs2.cpp target profile"
```

Expected: commit succeeds with only the profile and test files staged.

### Task 2: Prove Generated Config Output Changes For GameCube

**Files:**
- Modify: `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPGeneratedConfigWriterTests.cs`

- [ ] **Step 1: Add a failing generated-config test for the GameCube profile**

Update `/mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPGeneratedConfigWriterTests.cs` with a test equivalent to:

```csharp
[Fact]
public void Write_WithGameCubeProfile_EmitsGameCubeAndGccDefines() {
    string outputFolder = CreateTemporaryOutputFolder();
    CPPConversionOptions options = CPPConversionOptions.CreateGameCubeDefault();
    CPPRuntimeRequirementRegistrar registrar = CreateRegistrar();

    string filePath = CPPGeneratedConfigWriter.Write(outputFolder, options, registrar);
    string output = File.ReadAllText(filePath);

    Assert.Contains("#define HE_CPP_COMPILER_GCC 1", output);
    Assert.Contains("#define HE_CPP_PLATFORM_GAMECUBE 1", output);
    Assert.Contains("#define HE_CPP_PLATFORM_IS_LITTLE_ENDIAN 0", output);
    Assert.Contains("#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0", output);
}
```

- [ ] **Step 2: Run the generated-config tests to verify the new GameCube assertions fail first**

Run:

```bash
rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPGeneratedConfigWriterTests" -v minimal
```

Expected: FAIL until the new profile factory is available to the test project and emits the expected symbols.

- [ ] **Step 3: Re-run the tests after the profile work is in place**

Run:

```bash
rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPGeneratedConfigWriterTests|FullyQualifiedName~CPPConversionOptionsTests" -v minimal
```

Expected: PASS with the new GameCube config assertions.

- [ ] **Step 4: Commit the config-writer coverage**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/csharpcodegen add cs2.cpp.tests/CPPGeneratedConfigWriterTests.cs
rtk git -C /mnt/c/dev/helworks/csharpcodegen commit -m "Verify GameCube generated config output"
```

Expected: commit succeeds with only the updated config-writer tests staged.

### Task 3: Regenerate A GameCube-Profiled Core Drop

**Files:**
- Generated output only

- [ ] **Step 1: Locate the current regeneration entry point before invoking it**

Run:

```bash
rtk rg -n "CreateDefault|CreateGameCubeDefault|WindowsHandoffOutputFolder|WriteOutput" /mnt/c/dev/helworks/csharpcodegen/cs2.cpp
```

Expected: you can identify the call site or temporary harness used to construct `CPPConversionOptions` for the next regeneration run.

- [ ] **Step 2: Run one regeneration using the new GameCube option set**

Run one regeneration command or harness invocation that writes into a disposable folder such as:

```text
/mnt/c/dev/helworks/helengine/tmp/helengine-core-cpp-regenerated-gamecube
```

Expected: the output folder contains at least:

```text
Core.hpp
helcpp_config.hpp
helengine_core_unity.cpp
```

- [ ] **Step 3: Inspect the regenerated config header before touching helengine-gc**

Run:

```bash
rtk sed -n '1,80p' /mnt/c/dev/helworks/helengine/tmp/helengine-core-cpp-regenerated-gamecube/helcpp_config.hpp
```

Expected: the header contains `HE_CPP_COMPILER_GCC 1`, `HE_CPP_PLATFORM_GAMECUBE 1`, `HE_CPP_PLATFORM_IS_LITTLE_ENDIAN 0`, and `HE_CPP_PLATFORM_IS_WINDOWS_HOST 0`.

- [ ] **Step 4: Do not commit generated output yet**

Run:

```bash
rtk printf '%s\n' "Generated output stays uncommitted and is consumed through HELENGINE_CORE_CPP_ROOT."
```

Expected: the next tasks use the generated folder as an external input only.

### Task 4: Teach helengine-gc To Include Generated Core

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/Makefile`

- [ ] **Step 1: Add a failing generated-core build validation to the Makefile**

Update `Makefile` so it defines generated-core paths when `HELENGINE_CORE_CPP_ROOT` is non-empty:

```makefile
GENERATED_CORE_UNITY :=
GENERATED_CORE_CONFIG :=

ifneq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
GENERATED_CORE_UNITY := $(HELENGINE_CORE_CPP_ROOT)/helengine_core_unity.cpp
GENERATED_CORE_CONFIG := $(HELENGINE_CORE_CPP_ROOT)/helcpp_config.hpp
endif
```

- [ ] **Step 2: Add hard validation instead of silently compiling with a bad root**

Append to `Makefile`:

```makefile
ifneq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
ifeq ($(wildcard $(GENERATED_CORE_CONFIG)),)
$(error HELENGINE_CORE_CPP_ROOT does not contain helcpp_config.hpp)
endif
ifeq ($(wildcard $(GENERATED_CORE_UNITY)),)
$(error HELENGINE_CORE_CPP_ROOT does not contain helengine_core_unity.cpp)
endif
endif
```

- [ ] **Step 3: Add the generated unity file to the source/object lists**

Change `SOURCES` and object resolution to include:

```makefile
SOURCES := \
	$(SOURCE_DIR)/main.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeBootHost.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager3D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager2D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeInputManager.cpp

ifneq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
SOURCES += $(GENERATED_CORE_UNITY)
endif
```

and replace the object rule derivation with a form that can handle both local and external `.cpp` files:

```makefile
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.cpp=.o)))
```

- [ ] **Step 4: Replace the compile rule so external generated sources can build**

Replace the current pattern compile rule with:

```makefile
$(BUILD_DIR)/%.o:
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(firstword $(filter %/$*.cpp,$(SOURCES)) $(filter $*.cpp,$(notdir $(SOURCES)))) -o $@
```

If this exact form proves too brittle during implementation, replace it with an explicit source-to-object macro table, but keep the outcome the same: both repo-local and generated-core `.cpp` files compile through the same flags.

- [ ] **Step 5: Dry-run the build with a generated-core root before runtime refactoring**

Run:

```bash
rtk make -n HELENGINE_CORE_CPP_ROOT=/mnt/c/dev/helworks/helengine/tmp/helengine-core-cpp-regenerated-gamecube
```

Expected: make prints a compile line for `helengine_core_unity.cpp` and includes the generated root on the include path.

- [ ] **Step 6: Commit the build integration surface**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/helengine-gc add Makefile
rtk git -C /mnt/c/dev/helworks/helengine-gc commit -m "Add GameCube generated-core build path"
```

Expected: commit succeeds with only the Makefile staged.

### Task 5: Replace The Boot Host With A Minimal Player Host

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/main.cpp`
- Replace: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootHost.hpp`
- Replace: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootHost.cpp`

- [ ] **Step 1: Update the entry point to reflect player semantics**

Replace `src/main.cpp` with:

```cpp
#include "platform/gamecube/GameCubeBootHost.hpp"

int main() {
    helengine::gamecube::GameCubeBootHost host;
    return host.Run();
}
```

Keep the file shape small; the behavior change will live in the host class.

- [ ] **Step 2: Redefine the host interface around player responsibilities**

Replace `src/platform/gamecube/GameCubeBootHost.hpp` with a header equivalent to:

```cpp
#pragma once

#include <gccore.h>

class Core;

namespace helengine::gamecube {
    class GameCubeRenderManager2D;
    class GameCubeRenderManager3D;
    class GameCubeInputManager;

    class GameCubeBootHost {
    public:
        GameCubeBootHost();
        ~GameCubeBootHost();

        int Run();

    private:
        bool InitializeVideo();
        bool InitializeGraphics();
        bool InitializeEngineCore();
        void RunFrame();
        void PresentFrame();

        GXRModeObj* RenderMode;
        void* FrameBuffer;
        void* FifoBuffer;
        Core* EngineCore;
        GameCubeRenderManager3D* EngineRenderManager3D;
        GameCubeRenderManager2D* EngineRenderManager2D;
        GameCubeInputManager* EngineInputManager;
        bool EngineInitialized;
    };
}
```

- [ ] **Step 3: Replace the implementation so the loop can optionally boot generated core**

Replace `src/platform/gamecube/GameCubeBootHost.cpp` so it:

- keeps the existing VI and GX initialization
- creates and destroys the new bridge objects
- conditionally includes generated core via `#if __has_include("Core.hpp")`
- calls `InitializeEngineCore()` after graphics init
- calls `RunFrame()` inside the infinite loop

The core logic should follow this shape:

```cpp
void GameCubeBootHost::RunFrame() {
#if __has_include("Core.hpp")
    if (EngineInitialized && EngineCore != nullptr) {
        EngineCore->Update();
        EngineCore->Draw();
    }
#endif

    PresentFrame();
}
```

and:

```cpp
bool GameCubeBootHost::InitializeEngineCore() {
#if __has_include("Core.hpp")
    EngineRenderManager3D = new GameCubeRenderManager3D(RenderMode, FrameBuffer);
    EngineRenderManager2D = new GameCubeRenderManager2D();
    EngineInputManager = new GameCubeInputManager();
    EngineCore = new Core();

    CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
    options->UpdateOrderLayers = 4;
    options->RenderOrderLayers3D = 4;
    options->UpdateListInitialCapacity = 64;
    options->RenderList2DInitialCapacity = 64;
    options->RenderList3DInitialCapacity = 64;

    EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, options);
    EngineInitialized = true;
#endif

    return true;
}
```

Do not add startup-scene loading in this milestone.

- [ ] **Step 4: Check the rewritten player host before adding bridge implementations**

Run:

```bash
rtk sed -n '1,220p' src/main.cpp
rtk sed -n '1,260p' src/platform/gamecube/GameCubeBootHost.hpp
rtk sed -n '1,360p' src/platform/gamecube/GameCubeBootHost.cpp
```

Expected: the host now has engine-core fields, `InitializeEngineCore()`, and a `RunFrame()` path that can call `Core::Update()` and `Core::Draw()`.

- [ ] **Step 5: Commit the host refactor**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/helengine-gc add src/main.cpp src/platform/gamecube/GameCubeBootHost.hpp src/platform/gamecube/GameCubeBootHost.cpp
rtk git -C /mnt/c/dev/helworks/helengine-gc commit -m "Refactor GameCube host into player bootstrap"
```

Expected: commit succeeds with only the entry point and host files staged.

### Task 6: Add Minimal Generated-Core Bridge Types

**Files:**
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.cpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager2D.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager2D.cpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeInputManager.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeInputManager.cpp`

- [ ] **Step 1: Create the minimal 3D bridge header**

Create `GameCubeRenderManager3D.hpp` with a class derived from generated `RenderManager3D` under `#if __has_include("RenderManager3D.hpp")`, equivalent to:

```cpp
#pragma once

#if __has_include("RenderManager3D.hpp")
#include "RenderManager3D.hpp"

#include <gccore.h>

namespace helengine::gamecube {
    class GameCubeRenderManager3D : public RenderManager3D {
    public:
        GameCubeRenderManager3D(GXRModeObj* renderMode, void* frameBuffer);

        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;
        void Draw() override;

    private:
        GXRModeObj* RenderMode;
        void* FrameBuffer;
    };
}
#endif
```

- [ ] **Step 2: Create the minimal 3D bridge implementation**

Create `GameCubeRenderManager3D.cpp` so:

- `BuildModelFromRaw()` returns `nullptr`
- `Draw()` does not crash and leaves the existing GX clear/copy path intact

Equivalent shape:

```cpp
#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#if __has_include("RenderManager3D.hpp")
namespace helengine::gamecube {
    GameCubeRenderManager3D::GameCubeRenderManager3D(GXRModeObj* renderMode, void* frameBuffer)
        : RenderMode(renderMode)
        , FrameBuffer(frameBuffer) {
    }

    RuntimeModel* GameCubeRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        return nullptr;
    }

    void GameCubeRenderManager3D::Draw() {
    }
}
#endif
```

- [ ] **Step 3: Create the minimal 2D bridge**

Create `GameCubeRenderManager2D.hpp/.cpp` so the class derives from generated `RenderManager2D` and its required methods exist but are no-op:

```cpp
RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override { return nullptr; }
void DrawSprite(ISpriteDrawable2D* sprite) override { }
void DrawText(ITextDrawable2D* text) override { }
void DrawRoundedRect(IRoundedRectDrawable2D* shape) override { }
```

- [ ] **Step 4: Create the minimal input bridge**

Create `GameCubeInputManager.hpp/.cpp` so:

- `GameCubeKeyboard` derives from generated `Keyboard`
- `GameCubeMouse` derives from generated `Mouse`
- `GameCubeInputManager` derives from generated `InputManager`
- keyboard and mouse return default, inactive states
- pointer wrapping methods are harmless no-ops

The first milestone is correctness of boot, not real controller support.

- [ ] **Step 5: Sanity-check the new bridge surfaces before container verification**

Run:

```bash
rtk sed -n '1,220p' src/platform/gamecube/GameCubeRenderManager3D.hpp
rtk sed -n '1,240p' src/platform/gamecube/GameCubeRenderManager3D.cpp
rtk sed -n '1,220p' src/platform/gamecube/GameCubeRenderManager2D.hpp
rtk sed -n '1,240p' src/platform/gamecube/GameCubeRenderManager2D.cpp
rtk sed -n '1,260p' src/platform/gamecube/GameCubeInputManager.hpp
rtk sed -n '1,320p' src/platform/gamecube/GameCubeInputManager.cpp
```

Expected: every generated abstract method used by the bridge bases has at least a minimal implementation and no file tries to pull in Windows-specific behavior.

- [ ] **Step 6: Commit the bridge layer**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeRenderManager3D.hpp src/platform/gamecube/GameCubeRenderManager3D.cpp src/platform/gamecube/GameCubeRenderManager2D.hpp src/platform/gamecube/GameCubeRenderManager2D.cpp src/platform/gamecube/GameCubeInputManager.hpp src/platform/gamecube/GameCubeInputManager.cpp
rtk git -C /mnt/c/dev/helworks/helengine-gc commit -m "Add GameCube generated-core bridge stubs"
```

Expected: commit succeeds with only the new bridge files staged.

### Task 7: Verify The First Generated-Core Boot

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/README.md`

- [ ] **Step 1: Build the GameCube player with generated core included**

Run:

```bash
DOCKER_CONFIG=/tmp/docker-no-creds docker build -t helengine-gc .
DOCKER_CONFIG=/tmp/docker-no-creds docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make HELENGINE_CORE_CPP_ROOT=/mnt/c/dev/helworks/helengine/tmp/helengine-core-cpp-regenerated-gamecube
```

Expected: the build reaches a `helengine_gc.dol` that includes `helengine_core_unity.cpp`.

- [ ] **Step 2: Write the README note for the generated-core path**

Update `README.md` to add a section equivalent to:

````md
## Generated core boot

When `HELENGINE_CORE_CPP_ROOT` points to a valid generated GameCube-profiled `helengine.core` C++ output folder, the native player compiles `helengine_core_unity.cpp` and attempts to boot the generated engine core.

Example:

```bash
DOCKER_CONFIG=/tmp/docker-no-creds docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make HELENGINE_CORE_CPP_ROOT=/mnt/c/dev/helworks/helengine/tmp/helengine-core-cpp-regenerated-gamecube
```
````

- [ ] **Step 3: Sanity-check the README against the actual build path**

Run:

```bash
rtk sed -n '1,260p' README.md
```

Expected: the README names the same `HELENGINE_CORE_CPP_ROOT` usage and generated output root used in verification.

- [ ] **Step 4: Verify the runtime milestone in Dolphin**

Run this manual check:

```text
Load build/helengine_gc.dol in Dolphin.
Expected: the player enters the native loop and does not crash during Core::Initialize(), Core::Update(), or Core::Draw().
```

- [ ] **Step 5: Commit the documentation**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/helengine-gc add README.md
rtk git -C /mnt/c/dev/helworks/helengine-gc commit -m "Document GameCube generated-core boot path"
```

Expected: commit succeeds with only the README staged.

### Task 8: Final Verification And Cleanup

**Files:**
- Verify repository state only

- [ ] **Step 1: Re-run the critical generator tests together**

Run:

```bash
rtk dotnet test /mnt/c/dev/helworks/csharpcodegen/cs2.cpp.tests/cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionOptionsTests|FullyQualifiedName~CPPGeneratedConfigWriterTests" -v minimal
```

Expected: PASS.

- [ ] **Step 2: Re-run the full GameCube generated-core build from a clean workspace**

Run:

```bash
rtk make clean
DOCKER_CONFIG=/tmp/docker-no-creds docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make clean
DOCKER_CONFIG=/tmp/docker-no-creds docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make HELENGINE_CORE_CPP_ROOT=/mnt/c/dev/helworks/helengine/tmp/helengine-core-cpp-regenerated-gamecube
```

Expected: the clean build completes and re-emits `build/helengine_gc.dol` with generated core enabled.

- [ ] **Step 3: Check repository cleanliness before handoff**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/csharpcodegen status --short
rtk git -C /mnt/c/dev/helworks/helengine-gc status --short
```

Expected: only intended source/docs/build-script changes remain. Generated outputs stay external and uncommitted.

- [ ] **Step 4: Prepare the final integration checkpoint**

Run:

```bash
rtk git -C /mnt/c/dev/helworks/csharpcodegen log --oneline --decorate -n 5
rtk git -C /mnt/c/dev/helworks/helengine-gc log --oneline --decorate -n 5
```

Expected: both repos show a clean sequence of generator-first then player-host commits for review.
