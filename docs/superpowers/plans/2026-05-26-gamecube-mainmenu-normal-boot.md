# GameCube Main Menu Normal Boot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Boot the GameCube export into the authored `DemoDiscMainMenu` scene through the normal project-selected startup path and confirm it reaches a first visible frame or a precise boot failure.

**Architecture:** This pass changes only the city project's GameCube startup selection so the existing packaged startup pipeline loads the main menu scene instead of the current rendering smoke scene. Validation stays narrow: rebuild a dedicated GameCube smoke ISO, inspect the packaged startup manifest, and launch Dolphin to capture the first runtime result without enabling GameCube-specific bootstrap overrides.

**Tech Stack:** C#, generated HelEngine authoring pipeline, `build_config.json` project settings, headless HelEngine editor export flow, Dolphin Emulator, GameCube packaged runtime

---

## File Structure

- Modify: `C:\dev\helprojs\city\user_settings\build_config.json`
  - Switch only the GameCube platform startup selection from `textured_cube_grid` to `DemoDiscMainMenu`.
- Inspect: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
  - Confirm the authored scene id and keep the plan aligned with the project source of truth before editing the build config.
- Inspect: `C:\dev\helprojs\output\gamecube-mainmenu-smoke\disc\files\cooked\scenes`
  - Validate that the export actually packages the main menu cooked scene.
- Inspect: `C:\Users\beatriz\AppData\Local\Temp\helengine-platform-build\gamecube\*\generated-core\runtime\gamecube_runtime_scene_manifest.inl`
  - Confirm the generated packaged startup scene id/path after the export.

### Task 1: Confirm Main Menu Startup Inputs

**Files:**
- Inspect: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Inspect: `C:\dev\helprojs\city\user_settings\build_config.json`

- [ ] **Step 1: Read the authored main menu scene id**

Run: `rtk rg -n "SceneId|DemoDiscMainMenu" C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
Expected: the factory exposes the authored main menu scene id used by the city project.

- [ ] **Step 2: Read the current GameCube startup selection**

Run: `rtk rg -n "\"gamecube\"|selectedSceneIds|sceneOrders|textured_cube_grid|DemoDiscMainMenu" C:\dev\helprojs\city\user_settings\build_config.json`
Expected: the GameCube section still points at `textured_cube_grid` before the change.

- [ ] **Step 3: Commit no code yet**

Expected: no files are modified during the inspection task.

### Task 2: Switch the Normal GameCube Startup Scene

**Files:**
- Modify: `C:\dev\helprojs\city\user_settings\build_config.json`

- [ ] **Step 1: Edit the GameCube startup scene selection**

Change only the GameCube platform block so that:
- `selectedSceneIds` becomes `["DemoDiscMainMenu"]`
- `sceneOrders` becomes a single `DemoDiscMainMenu` entry

- [ ] **Step 2: Re-read the GameCube config block**

Run: `rtk rg -n "\"gamecube\"|selectedSceneIds|sceneOrders|DemoDiscMainMenu|textured_cube_grid" C:\dev\helprojs\city\user_settings\build_config.json`
Expected: `DemoDiscMainMenu` appears in the GameCube block and `textured_cube_grid` no longer does there.

- [ ] **Step 3: Commit the configuration switch**

```bash
git add C:\dev\helprojs\city\user_settings\build_config.json
git commit -m "Point GameCube startup at main menu"
```

### Task 3: Rebuild the Dedicated Main Menu Smoke ISO

**Files:**
- Output: `C:\dev\helprojs\output\gamecube-mainmenu-smoke`

- [ ] **Step 1: Clear only the dedicated smoke output if it already exists**

Run: `rtk powershell -NoProfile -Command "if (Test-Path 'C:\dev\helprojs\output\gamecube-mainmenu-smoke') { Remove-Item 'C:\dev\helprojs\output\gamecube-mainmenu-smoke' -Recurse -Force }"`
Expected: the main-menu smoke output directory is absent before the rebuild.

- [ ] **Step 2: Run the normal headless GameCube export**

Run: `rtk dotnet run --project C:\dev\helworks\helengine.editor.app\helengine.editor.app.csproj -- export-gamecube --project C:\dev\helprojs\city\project.heproj --output C:\dev\helprojs\output\gamecube-mainmenu-smoke`
Expected: export completes successfully and produces `C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`.

- [ ] **Step 3: Confirm the main menu cooked scene was packaged**

Run: `rtk rg -n "DemoDiscMainMenu" C:\dev\helprojs\output\gamecube-mainmenu-smoke`
Expected: the output contains the cooked main menu scene and the generated startup manifest references it.

- [ ] **Step 4: Commit only if export requires source changes beyond config**

Expected: no extra commit is needed if the export succeeds without code changes.

### Task 4: Validate the Packaged Startup Contract

**Files:**
- Inspect: `C:\Users\beatriz\AppData\Local\Temp\helengine-platform-build\gamecube\*\generated-core\runtime\gamecube_runtime_scene_manifest.inl`
- Inspect: `C:\dev\helprojs\output\gamecube-mainmenu-smoke\disc`

- [ ] **Step 1: Check the generated runtime startup manifest**

Run: `rtk rg -n "DemoDiscMainMenu|startup|cooked/scenes" C:\Users\beatriz\AppData\Local\Temp\helengine-platform-build\gamecube`
Expected: the latest generated runtime manifest names the main menu startup scene and its cooked scene path.

- [ ] **Step 2: Check the staged disc tree for the cooked scene**

Run: `rtk rg -n "DemoDiscMainMenu" C:\dev\helprojs\output\gamecube-mainmenu-smoke\disc`
Expected: the staged disc contains the cooked main menu asset path expected by the runtime manifest.

- [ ] **Step 3: Commit no code yet**

Expected: this task is inspection-only.

### Task 5: Launch Dolphin and Capture the First Result

**Files:**
- Output: `C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`

- [ ] **Step 1: Launch Dolphin on the rebuilt ISO**

Run: `rtk "C:\dev\tools\dolphin\2603a\Dolphin-x64\Dolphin.exe" -e C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`
Expected: Dolphin opens on the dedicated main-menu smoke ISO.

- [ ] **Step 2: Record the first visible outcome**

Expected: either the main menu reaches a first visible frame or Dolphin shows the first concrete runtime failure symptom.

- [ ] **Step 3: If boot fails, capture the first concrete `OSREPORT` failure**

Expected: the next engineering action is driven by the first precise boot log, not by speculative menu-system changes.

- [ ] **Step 4: Commit only if runtime debugging required engine/project source changes**

Expected: a boot-only success with no further fixes requires no extra commit beyond the startup-config switch.

## Verification Summary

- The GameCube platform block in `build_config.json` points at `DemoDiscMainMenu` and nothing else changed there.
- `C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm` exists after the rebuild.
- The generated packaged startup manifest references the main menu cooked scene.
- Dolphin either shows the first main menu frame or exposes the first precise runtime blocker.

## Commit Strategy

1. Commit the startup-scene configuration switch after the config edit is verified.
2. Only create additional commits if the boot-only pass requires real engine or project-source fixes discovered during runtime validation.
