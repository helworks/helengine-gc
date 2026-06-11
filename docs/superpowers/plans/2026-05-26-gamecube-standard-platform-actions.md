# GameCube Standard Platform Actions Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the GameCube menu use the existing standard platform input contract so `A` acts as `Accept` and `B` acts as `Return`.

**Architecture:** Keep the fix at the platform-settings layer instead of adding GameCube-only runtime behavior. Seed GameCube default standard actions in the shared editor profile service, then update the `city` GameCube platform settings so the current project exports those bindings into the packaged runtime manifest.

**Tech Stack:** C#, .NET 9, shared editor profile settings, city project platform JSON settings, Dolphin manual verification

---

## File Structure

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorProfileSettingsService.cs`
  - Add GameCube standard action default seeding alongside the existing `ds` and `ps2` cases.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorProfileSettingsServiceTests.cs`
  - Add a focused regression test proving a missing GameCube platform file seeds `Accept` and `Return`.
- Modify: `C:\dev\helprojs\city\settings\platform.gamecube.json`
  - Persist the authored GameCube standard action bindings for the current project.

## Task 1: Add GameCube Default Standard Action Seeding

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorProfileSettingsServiceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorProfileSettingsService.cs`

- [ ] **Step 1: Write the failing editor profile seeding test**

Add this test near the existing DS seeding coverage in `EditorProfileSettingsServiceTests.cs`:

```csharp
/// <summary>
/// Ensures one missing GameCube platform file seeds standard platform action mappings.
/// </summary>
[Fact]
public void Load_WhenGameCubePlatformFileIsMissing_SeedsStandardPlatformActions() {
    EditorProfileSettingsService service = new EditorProfileSettingsService(TempRootPath);

    EditorProfileSettingsDocument document = service.Load(new List<string> { "gamecube" });

    EditorPlatformProfileSettingsDocument platform = Assert.Single(document.Platforms);
    Assert.Equal("gamecube", platform.PlatformId);
    Assert.NotNull(platform.Input);
    Assert.NotNull(platform.Input.StandardActions);
    Assert.NotNull(platform.Input.StandardActions.Accept);
    Assert.NotNull(platform.Input.StandardActions.Return);
    Assert.Equal(0, platform.Input.StandardActions.Accept.ControlIndex);
    Assert.Equal(1, platform.Input.StandardActions.Return.ControlIndex);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter Load_WhenGameCubePlatformFileIsMissing_SeedsStandardPlatformActions
```

Expected: FAIL because GameCube currently leaves both standard actions unset.

- [ ] **Step 3: Implement minimal GameCube default seeding**

In `EditorProfileSettingsService.cs`, extend the existing platform id chain so GameCube seeds:

```csharp
if (string.Equals(platformId, "ds", StringComparison.OrdinalIgnoreCase)) {
    platform.Input.StandardActions.Accept ??= CreateGamepadButtonControl((int)InputGamepadButton.South);
    platform.Input.StandardActions.Return ??= CreateGamepadButtonControl((int)InputGamepadButton.East);
} else if (string.Equals(platformId, "ps2", StringComparison.OrdinalIgnoreCase)) {
    platform.Input.StandardActions.Accept ??= CreateGamepadButtonControl((int)InputGamepadButton.South);
    platform.Input.StandardActions.Return ??= CreateGamepadButtonControl((int)InputGamepadButton.North);
} else if (string.Equals(platformId, "gamecube", StringComparison.OrdinalIgnoreCase)) {
    platform.Input.StandardActions.Accept ??= CreateGamepadButtonControl((int)InputGamepadButton.South);
    platform.Input.StandardActions.Return ??= CreateGamepadButtonControl((int)InputGamepadButton.East);
}
```

- [ ] **Step 4: Run the focused test to verify it passes**

Run:

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter Load_WhenGameCubePlatformFileIsMissing_SeedsStandardPlatformActions
```

Expected: PASS.

- [ ] **Step 5: Commit the editor default seeding slice**

```bash
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorProfileSettingsService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorProfileSettingsServiceTests.cs
rtk git commit -m "Seed GameCube standard platform actions"
```

## Task 2: Persist GameCube Standard Actions In The City Project

**Files:**
- Modify: `C:\dev\helprojs\city\settings\platform.gamecube.json`

- [ ] **Step 1: Write the authored GameCube standard action bindings**

Replace the current `null` GameCube standard actions with:

```json
"input": {
  "standardActions": {
    "accept": {
      "deviceKind": 0,
      "controlKind": 0,
      "deviceIndex": 0,
      "controlIndex": 0
    },
    "return": {
      "deviceKind": 0,
      "controlKind": 0,
      "deviceIndex": 0,
      "controlIndex": 1
    }
  }
}
```

- [ ] **Step 2: Verify the settings file matches the intended semantic mapping**

Run:

```bash
rtk powershell -NoProfile -Command "Get-Content 'C:\dev\helprojs\city\settings\platform.gamecube.json' | Out-String | Write-Output"
```

Expected:
- `accept.controlIndex = 0`
- `return.controlIndex = 1`

- [ ] **Step 3: Commit the authored GameCube project binding**

```bash
rtk git add C:\dev\helprojs\city\settings\platform.gamecube.json
rtk git commit -m "Bind GameCube standard menu actions"
```

## Task 3: Verify The Packaged Main Menu Uses The New Bindings

**Files:**
- Modify: none
- Verify: `C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`

- [ ] **Step 1: Rebuild the GameCube main-menu export**

Run:

```bash
rtk powershell -NoProfile -Command "& 'C:\dev\helworks\temp\logo-override-tool\Rebuild-GameCubeMainMenu.ps1' 2>&1 | Select-Object -First 120 | Out-String | Write-Output; exit $LASTEXITCODE"
```

Expected:
- build completes with `EXIT=0`
- packaged output lands in `C:\dev\helprojs\output\gamecube-mainmenu-smoke`

- [ ] **Step 2: Relaunch Dolphin on the rebuilt main-menu ISO**

Run:

```bash
rtk powershell -NoProfile -Command "Get-Process Dolphin -ErrorAction SilentlyContinue | Stop-Process -Force"
rtk powershell -NoProfile -Command "Start-Process 'C:\dev\tools\dolphin\2603a\Dolphin-x64\Dolphin.exe' -ArgumentList @('-e','C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm')"
```

Expected:
- Dolphin launches the fresh `gamecube-mainmenu-smoke` ISO

- [ ] **Step 3: Manually verify the runtime behavior**

Check in Dolphin:

- `A` activates the currently selected menu item
- `B` triggers return behavior on scenes that include `DemoDiscReturnToMenuComponent`
- no runtime fallback or GameCube-specific menu code path was needed

- [ ] **Step 4: Commit if any verification-driven follow-up edits were required**

```bash
rtk git status --short
```

Expected:
- no new changes if the first-pass binding fix worked
- otherwise commit only the minimal follow-up needed

## Self-Review

- Spec coverage: the plan covers both required layers from the spec: editor default seeding and the authored GameCube project binding.
- Placeholder scan: no `TODO`, `TBD`, or hand-wavy “add tests later” steps remain.
- Type consistency: the plan uses the existing `StandardPlatformAction`, `InputGamepadButton`, and `EditorStandardPlatformActionSettingsDocument` names already present in the codebase.
