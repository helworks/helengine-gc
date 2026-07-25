# GameCube Nintendont-compatible Disc Driver Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make packaged-disc reads complete on both physical GameCube hardware and Nintendont in one GCM.

**Architecture:** `GameCubeDiscReader` owns a single interrupt-driven DI request. A global PowerPC bridge presents Nintendont's recognised DVD interrupt-hook shape and tail-calls the reader's C++ completion handler. The bootstrap initializes this driver before its header read; it does not mount or reset the disc.

**Tech Stack:** C++20, libogc IRQ/DVD initialization, PowerPC inline global assembly, xUnit source-contract tests.

---

### Task 1: Define the patchable reader boundary

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `src/platform/gamecube/GameCubeDiscReader.hpp`

- [ ] **Step 1: Write the failing source-contract test**

```csharp
Assert.Contains("GameCubeDiscReader::Initialize", bootstrapSource, StringComparison.Ordinal);
Assert.Contains("GameCubeDiscInterruptTrampoline", discReaderSource, StringComparison.Ordinal);
Assert.Contains("lis 3, 0xCC00", discReaderSource, StringComparison.Ordinal);
Assert.Contains("addi 3, 3, 0x6000", discReaderSource, StringComparison.Ordinal);
Assert.Contains("li 5, 0x002A", discReaderSource, StringComparison.Ordinal);
Assert.Contains("li 6, 0x0054", discReaderSource, StringComparison.Ordinal);
Assert.DoesNotContain("__io_gcdvd.readSectors", discReaderSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused test and verify it fails**

Run: `rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj --no-restore --filter "FullyQualifiedName~PackagedDiscReaderSource_UsesNintendontCompatibleInterruptBridge"`

Expected: FAIL because the current reader calls `__io_gcdvd.readSectors` and has no interrupt bridge.

- [ ] **Step 3: Declare explicit reader initialization**

```cpp
/// Installs the single packaged-disc DI completion handler without mounting or resetting the drive.
static bool Initialize();
```

- [ ] **Step 4: Re-run the focused test after implementation**

Expected: PASS.

### Task 2: Implement the single-request DI driver

**Files:**
- Modify: `src/platform/gamecube/GameCubeDiscReader.cpp`

- [ ] **Step 1: Add the PowerPC interrupt bridge**

```cpp
asm(
    ".global GameCubeDiscInterruptTrampoline\\n"
    "GameCubeDiscInterruptTrampoline:\\n"
    "lis 3, 0xCC00\\n"
    "addi 3, 3, 0x6000\\n"
    "li 5, 0x002A\\n"
    "li 6, 0x0054\\n"
    "b GameCubeDiscInterruptComplete\\n");
```

- [ ] **Step 2: Register the bridge after `DVD_Init`**

```cpp
DVD_Init();
IRQ_Request(IRQ_PI_DI, GameCubeDiscInterruptTrampoline, nullptr);
```

- [ ] **Step 3: Submit and complete aligned DI reads**

Use one static request record. Set it pending before writing the A8 command, wait only for the bridge to mark it complete, acknowledge DI status in the completion helper, and reject overlapping/invalid requests.

- [ ] **Step 4: Run the focused source-contract test**

Expected: PASS.

### Task 3: Wire bootstrap to the owned reader

**Files:**
- Modify: `src/platform/gamecube/GameCubeSceneBootstrap.hpp`
- Modify: `src/platform/gamecube/GameCubeSceneBootstrap.cpp`
- Modify: `src/platform/gamecube/GameCubeApplication.cpp`

- [ ] **Step 1: Change bootstrap initialization to report failure**

```cpp
static bool InitializePackagedDiscInterface();
```

- [ ] **Step 2: Gate header verification on successful reader initialization**

```cpp
if (!GameCubeSceneBootstrap::InitializePackagedDiscInterface()) {
    FailBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
    return false;
}
```

- [ ] **Step 3: Run focused source-contract tests**

Expected: PASS.

### Task 4: Compile and perform the discriminating console check

**Files:**
- Modify during build only: `C:\dev\helprojs\demodisc\settings\platform.gamecube.json`

- [ ] **Step 1: Enable `enable-direct-frame-diagnostic` only for this build**

- [ ] **Step 2: Build the packaged GameCube project and verify native compilation includes `GameCubeDiscReader.cpp`**

- [ ] **Step 3: Restore the project setting, copy `game.gcm` to `I:\games\helen\game.gcm`, and compare SHA-256 values**

- [ ] **Step 4: Boot with Nintendont and verify both conditions**

Expected: the screen advances beyond `A005` and `I:\ndebug.log` contains `Patch:[__DVDInterruptHandler]` for the new DOL.
