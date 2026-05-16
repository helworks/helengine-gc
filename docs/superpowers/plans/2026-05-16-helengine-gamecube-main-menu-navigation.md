# GameCube Main Menu Navigation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the packaged GameCube main menu navigable with D-pad selection, `A` confirm, and `B` back by populating the shared gamepad input contract from the native GameCube controller backend.

**Architecture:** Keep menu behavior in the shared generated `MenuComponent` runtime. Implement native controller polling only in `GameCubeInputManager`, map libogc button state to `InputGamepadState`, and verify the resulting source contract plus end-to-end packaged runtime behavior.

**Tech Stack:** C++, libogc PAD API, generated shared `InputSystem` / `InputFrameState` / `InputGamepadState`, C# xUnit builder source-contract tests, Dolphin for packaged runtime verification.

---

### Task 1: Lock The Expected GC Menu Input Contract In Tests

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Add a focused failing source-contract test for menu gamepad mapping**

Add a new test near the existing packaged runtime/menu source tests in `builder.tests/GameCubePackagedRuntimeSourceTests.cs` that asserts the GameCube input backend source contains the expected shared-contract mappings:

```csharp
[Fact]
public void GameCubeInputManager_WhenBuiltForMenu_MapsNativePadButtonsToSharedGamepadState() {
    string source = File.ReadAllText(
        Path.Combine(
            RepositoryRootPath,
            "src",
            "platform",
            "gamecube",
            "GameCubeInputManager.cpp"));

    Assert.Contains("PAD_ScanPads();", source, StringComparison.Ordinal);
    Assert.Contains("frame.set_GamepadCount(1);", source, StringComparison.Ordinal);
    Assert.Contains("Array<InputGamepadState>* gamepads = new Array<InputGamepadState>(1);", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.set_Connected(true);", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadUp", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadDown", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadLeft", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadRight", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::South", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::East", source, StringComparison.Ordinal);
    Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::Start", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeInputManager_WhenBuiltForMenu_MapsNativePadButtonsToSharedGamepadState -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because `GameCubeInputManager.cpp` still returns `GamepadCount=0` and does not contain the required mapping code.

- [ ] **Step 3: Commit the failing test checkpoint**

```bash
git add builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "Add GameCube menu input source contract test"
```

### Task 2: Implement Native GameCube Menu Input Capture

**Files:**
- Modify: `src/platform/gamecube/GameCubeInputManager.hpp`
- Modify: `src/platform/gamecube/GameCubeInputManager.cpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Update the GC input backend header includes if needed**

If the current header is missing generated shared input types needed by the implementation, keep the interface minimal and only add what is required. The header should remain structurally the same:

```cpp
#pragma once

#include "IInputBackend.hpp"
#include "InputFrameState.hpp"

namespace helengine::gamecube {
    class GameCubeInputManager : public IInputBackend {
    public:
        GameCubeInputManager();
        ~GameCubeInputManager();

        InputFrameState CaptureFrame() override;
    };
}
```

- [ ] **Step 2: Implement native controller polling in `GameCubeInputManager.cpp`**

Replace the stub `CaptureFrame()` with a real libogc-backed implementation that populates one shared `InputGamepadState` from controller port 0:

```cpp
#include "platform/gamecube/GameCubeInputManager.hpp"

#include <gccore.h>

#include "InputGamepadButton.hpp"
#include "InputGamepadState.hpp"
#include "runtime/array.hpp"

namespace helengine::gamecube {
    GameCubeInputManager::GameCubeInputManager() {
    }

    GameCubeInputManager::~GameCubeInputManager() {
    }

    InputFrameState GameCubeInputManager::CaptureFrame() {
        PAD_ScanPads();

        InputFrameState frame;

        const u32 connectedPads = PAD_ButtonsHeld(0) | PAD_ButtonsDown(0) | PAD_ButtonsUp(0);
        const bool isConnected = PAD_ScanPads(), (PAD_Probe(0) >= 0);

        if (!isConnected) {
            frame.set_GamepadCount(0);
            frame.set_Gamepads(Array<InputGamepadState>::Empty());
            return frame;
        }

        frame.set_GamepadCount(1);

        Array<InputGamepadState>* gamepads = new Array<InputGamepadState>(1);
        InputGamepadState gamepadState;
        gamepadState.set_Connected(true);

        const u16 heldButtons = PAD_ButtonsHeld(0);

        gamepadState.SetButtonDown(InputGamepadButton::DPadUp, (heldButtons & PAD_BUTTON_UP) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadDown, (heldButtons & PAD_BUTTON_DOWN) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadLeft, (heldButtons & PAD_BUTTON_LEFT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadRight, (heldButtons & PAD_BUTTON_RIGHT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::South, (heldButtons & PAD_BUTTON_A) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::East, (heldButtons & PAD_BUTTON_B) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::West, (heldButtons & PAD_BUTTON_X) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::North, (heldButtons & PAD_BUTTON_Y) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::LeftShoulder, (heldButtons & PAD_TRIGGER_L) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::RightShoulder, (heldButtons & PAD_TRIGGER_R) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::Start, (heldButtons & PAD_BUTTON_START) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::Select, (heldButtons & PAD_TRIGGER_Z) != 0);

        gamepadState.set_LeftStickX(static_cast<int16_t>(PAD_StickX(0) * 256));
        gamepadState.set_LeftStickY(static_cast<int16_t>(PAD_StickY(0) * 256));
        gamepadState.set_RightStickX(static_cast<int16_t>(PAD_SubStickX(0) * 256));
        gamepadState.set_RightStickY(static_cast<int16_t>(PAD_SubStickY(0) * 256));
        gamepadState.set_LeftTrigger(static_cast<int16_t>(PAD_TriggerL(0) * 256));
        gamepadState.set_RightTrigger(static_cast<int16_t>(PAD_TriggerR(0) * 256));

        (*gamepads)[0] = gamepadState;
        frame.set_Gamepads(gamepads);
        return frame;
    }
}
```

Then immediately correct the connection probe line to avoid the comma-expression anti-pattern. The final implementation should use:

```cpp
const bool isConnected = PAD_Probe(0) >= 0;
```

and remove the unused `connectedPads` variable entirely.

- [ ] **Step 3: Run the focused source-contract test to verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeInputManager_WhenBuiltForMenu_MapsNativePadButtonsToSharedGamepadState -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS.

- [ ] **Step 4: Commit the native input backend implementation**

```bash
git add src/platform/gamecube/GameCubeInputManager.hpp src/platform/gamecube/GameCubeInputManager.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "Add GameCube main menu gamepad input"
```

### Task 3: Verify The Broader GC Packaged Runtime Contract

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs` (only if more assertions are needed)
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Run the broader packaged runtime source slice**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS, confirming the current menu startup/runtime source contracts still hold after the input backend change.

- [ ] **Step 2: If the runtime slice reveals missing assertions, add only narrow source-contract coverage**

If needed, add assertions to the existing runtime-source test file for the exact lines that matter, for example:

```csharp
Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::South, (heldButtons & PAD_BUTTON_A) != 0);", source, StringComparison.Ordinal);
Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::East, (heldButtons & PAD_BUTTON_B) != 0);", source, StringComparison.Ordinal);
Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::Select, (heldButtons & PAD_TRIGGER_Z) != 0);", source, StringComparison.Ordinal);
```

Do not add speculative coverage for behaviors the shared menu runtime does not currently use.

- [ ] **Step 3: Re-run the broader packaged runtime slice if Step 2 changed tests**

Run the same command from Step 1 and confirm PASS.

- [ ] **Step 4: Commit any narrow source-contract follow-up**

```bash
git add builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "Refine GameCube menu input source coverage"
```

### Task 4: Build And Verify The Packaged Main Menu Runtime

**Files:**
- Modify: none expected
- Test: runtime packaged image in Dolphin

- [ ] **Step 1: Rebuild the packaged-disc native target**

Run:

```powershell
rtk docker run --rm -v C:/dev/helworks/helengine-gc:/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube -e HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc -e HELENGINE_GAMECUBE_MINIMAL_SAMPLE=0 helengine-gc sh -lc 'make clean all packaged-disc-assets'
```

Expected: native build succeeds and emits a fresh `build/helengine_gc.dol`.

- [ ] **Step 2: Repackage the fresh image**

Run:

```powershell
rtk dotnet run --project tmp/builder-retail-check -- package
```

Expected: PASS with `ISO/GC` output and a fresh `tmp/session-2026-05-15-gcm-demo-check/game.gcm`.

- [ ] **Step 3: Copy the image to the stable probe path and relaunch Dolphin**

Run:

```powershell
Copy-Item -Force C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm C:\dev\helworks\emus\gc-probe\game.gcm
Get-Process Dolphin* -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Process -FilePath C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe -ArgumentList '-e','C:\dev\helworks\emus\gc-probe\game.gcm'
```

Expected: Dolphin launches the packaged GC main menu.

- [ ] **Step 4: Manually verify D-pad selection movement**

In Dolphin:

- press `D-pad Down`
- observe the menu highlight move to the next item
- press `D-pad Up`
- observe the highlight move back

Expected: shared `MenuComponent` selection visuals update without crashes or scene reload issues.

- [ ] **Step 5: Manually verify `A` confirm**

In Dolphin:

- move selection to a menu item that opens another panel or launches a visible action
- press `A`

Expected: the shared menu action executes through `MenuActionKind` and the selected menu item confirms.

- [ ] **Step 6: Manually verify `B` back**

In Dolphin:

- after confirming into a submenu/panel, press `B`

Expected: the menu navigates back through the shared `MenuComponent::NavigateBack()` flow.

- [ ] **Step 7: Commit the verified packaged-navigation checkpoint**

```bash
git add src/platform/gamecube/GameCubeInputManager.hpp src/platform/gamecube/GameCubeInputManager.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "Enable GameCube main menu navigation"
```

### Task 5: Record The Durable Checkpoint

**Files:**
- Modify: none
- Test: n/a

- [ ] **Step 1: Write the Graphiti memory with the actual runtime boundary**

Record:

- that packaged GC main menu navigation now works
- that the shared `MenuComponent` path was already correct
- that the root cause was `GameCubeInputManager` returning `GamepadCount=0`
- the exact button mapping used (`A -> South`, `B -> East`, `D-pad -> DPad`, `Start -> Start`, `Z -> Select`)

- [ ] **Step 2: Verify the worktree is limited to intentional changes**

Run:

```powershell
git status --short
```

Expected: only the intended input/test changes and any pre-existing unrelated local churn remain.

- [ ] **Step 3: Stop**

Do not continue into analog-stick navigation, pointer emulation, or gameplay input parity in this plan. Those are separate follow-up slices once menu navigation is confirmed stable.

---

## Self-Review

- **Spec coverage:** This plan covers the approved success bar exactly: D-pad selection, `A` confirm, and `B` back on the authored packaged GC main menu. It does not expand into unrelated pointer or gameplay input work.
- **Placeholder scan:** No `TODO`/`TBD` placeholders remain. Every task includes exact files, code, commands, and expected results.
- **Type consistency:** The plan consistently uses the existing shared types and names already present in the generated core: `InputFrameState`, `InputGamepadState`, `InputGamepadButton`, `GameCubeInputManager`, and `MenuComponent`.
