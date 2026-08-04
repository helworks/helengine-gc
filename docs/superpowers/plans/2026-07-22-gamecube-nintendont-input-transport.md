# GameCube Nintendont Input Transport Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make packaged GameCube builds read Nintendont virtual controllers without entering libogc serial-interface initialization, while preserving libogc input on physical GameCube hardware.

**Architecture:** `GameCubeInputManager` uses PowerPC data-BAT register reads to prove that the documented Nintendont `SIInited` address at `0x93003060` maps into Wii MEM2 before it dereferences any `0x93xxxxxx` memory. It reads four pairs only on original Gekko and all eight Wii-family pairs so vWii's high MEM2 mapping is recognized. In Nintendont mode it marks `SIInited` ready, invokes the preloaded `PadStub`, and reads the four `PADStatus` records at `0x93003100`; otherwise it keeps the libogc `PAD_Init` and PAD query path. The engine-facing `InputFrameState` mapping is unchanged.

**Tech Stack:** C++17, libogc PAD APIs, libogc data-cache API, Nintendont loader ABI, xUnit source-contract tests.

---

### Task 1: Lock down input-transport source contracts

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-contract test**

Add one `[Fact]` that reads `GameCubeInputManager.cpp` and asserts these exact contracts:

```csharp
Assert.Contains("constexpr std::uintptr_t NintendontVirtualPadBufferAddress = 0x93003100U;", inputSource, StringComparison.Ordinal);
Assert.Contains("constexpr std::uintptr_t NintendontHollywoodVersionAddress = 0x80003138U;", inputSource, StringComparison.Ordinal);
Assert.Contains("return *reinterpret_cast<volatile const uint32_t*>(NintendontHollywoodVersionAddress) == 0x11U;", inputSource, StringComparison.Ordinal);
Assert.Contains("if (!IsNintendontEnvironment()) {\n            PAD_Init();\n        }", inputSource, StringComparison.Ordinal);
Assert.Contains("DCInvalidateRange(nintendontPadStatuses, sizeof(PADStatus) * 4U);", inputSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the new test and verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~GameCubeInputManagerSource_UsesNintendontVirtualPadTransport" --verbosity minimal
```

Expected: one failing test because the Nintendont constants and branch do not exist.

- [ ] **Step 3: Commit the test-only change**

```powershell
rtk git add builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "test: specify nintendont input transport"
```

### Task 2: Add the Nintendont transport to the input manager

**Files:**
- Modify: `src/platform/gamecube/GameCubeInputManager.hpp`
- Modify: `src/platform/gamecube/GameCubeInputManager.cpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Add explicit transport state and detection**

Add a `bool UsesNintendontInputTransport` field and a documented static `IsNintendontEnvironment` method. Initialize the field in `GameCubeInputManager::GameCubeInputManager()` with the console-type register check:

```cpp
constexpr std::uintptr_t NintendontConsoleTypeRegisterAddress = 0xCC00302CU;

bool GameCubeInputManager::IsNintendontEnvironment() {
    return (*reinterpret_cast<volatile const uint32_t*>(NintendontConsoleTypeRegisterAddress) >> 28U) == 1U;
}
```

- [ ] **Step 2: Select initialization by transport**

Keep `InitializePlatformInput` as the application boundary and only initialize libogc PAD outside Nintendont:

```cpp
void GameCubeInputManager::InitializePlatformInput() {
    if (!IsNintendontEnvironment()) {
        PAD_Init();
    }
}
```

- [ ] **Step 3: Read virtual PAD status in Nintendont mode**

Add a private method that returns one `PADStatus`. It must invalidate the complete four-pad buffer before copying port zero:

```cpp
constexpr std::uintptr_t NintendontVirtualPadBufferAddress = 0x93003100U;
PADStatus* const nintendontPadStatuses = reinterpret_cast<PADStatus*>(NintendontVirtualPadBufferAddress);
DCInvalidateRange(nintendontPadStatuses, sizeof(PADStatus) * 4U);
return nintendontPadStatuses[0];
```

For physical hardware, keep `PAD_ScanPads`, populate a `PADStatus` from the existing `PAD_*` functions, and preserve the current input button/axis mapping. Set `Connected` from `padStatus.err == PAD_ERR_NONE`.

- [ ] **Step 4: Run the targeted test and verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~GameCubeInputManagerSource_UsesNintendontVirtualPadTransport" --verbosity minimal
```

Expected: one passing test.

- [ ] **Step 5: Check formatting and commit the implementation**

```powershell
rtk git diff --check -- src/platform/gamecube/GameCubeInputManager.hpp src/platform/gamecube/GameCubeInputManager.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git add src/platform/gamecube/GameCubeInputManager.hpp src/platform/gamecube/GameCubeInputManager.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "feat: add nintendont input transport"
```

### Task 3: Package and verify on Nintendont

**Files:**
- Modify: none
- Output: `C:/dev/helprojs/demodisc/output/gamecube-nintendont-input-20260722/game.gcm`

- [ ] **Step 1: Build the packaged disc image**

Run:

```powershell
rtk dotnet run --project C:/dev/helworks/helengine/helengine.ui/helengine.editor.app/helengine.editor.app.csproj -c Debug --no-build -- --project C:/dev/helprojs/demodisc/project.heproj --build gamecube --output C:/dev/helprojs/demodisc/output/gamecube-nintendont-input-20260722
```

Expected: `gamecube-build-phase.txt` ends with `packaged outputs verified` and `game.gcm` exists.

- [ ] **Step 2: Verify on the user’s vWii/Nintendont hardware**

Load the GCM through Nintendont. Expected result: boot progresses beyond the current green platform-input checkpoint; controller port zero produces mapped buttons and axes once the running scene begins.
