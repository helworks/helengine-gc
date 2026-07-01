# GameCube Static-Mesh Physics Showcase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add one authored packaged static-mesh physics showcase scene that is reachable from the demo-disc main menu and can also be booted directly through a packaged GameCube startup override.

**Architecture:** Keep authored content, menu wiring, and generated `.helen` assets in the `city` project, because that project already owns the physics scene catalog and demo-disc menu content. Keep `helengine-gc` focused on packaged boot selection and staged GameCube content, adding only a small startup-scene override path and the staging updates needed so the packaged menu build can actually load the new physics scene.

**Tech Stack:** C#, Helengine scene-generation pipeline, BEPU static-mesh collision cook path, generated `.helen` scene assets, C++ GameCube packaged runtime bootstrap, PowerShell/bash staging scripts, Dolphin Emulator

---

## File Structure

- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs`
  - Add the new authored scene id constant and include it in the exported scene-id list.
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneFactory.cs`
  - Add the new static-mesh showcase scene builder, any required static-mesh collider record helpers, and playable-showcase routing for the new scene.
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs`
  - Add one curated physics menu entry for the new scene.
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsNintendoDsSceneGenerator.cs`
  - Treat the new scene as a playable physics showcase so DS companion generation stays aligned with the curated menu entry set.
- Modify: `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase.helen`
  - Regenerated authored desktop scene output.
- Modify: `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase_ds.helen`
  - Regenerated Nintendo DS companion scene output.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.hpp`
  - Declare packaged startup override helpers.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.cpp`
  - Implement the packaged startup override and keep the default path on the manifest startup scene id.
- Modify: `C:\dev\helworks\helengine-gc\tools\stage_city_demo_disc_main_menu_content.sh`
  - Copy the curated physics cooked scenes into the staged packaged-content root so menu scene loads can resolve them.
- Inspect: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeApplication.cpp`
  - Verify the runtime still uses `GameCubeSceneBootstrap::GetPackagedStartupSceneId()` in both initialization logging and `LoadScene`.
- Inspect: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\StaticMeshColliderGenericPersistenceTests.cs`
  - Reuse existing engine-side static-mesh persistence confidence instead of duplicating unrelated engine coverage.

### Task 1: Lock Down Scene Ids And Menu Wiring

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsNintendoDsSceneGenerator.cs`

- [ ] **Step 1: Verify the new scene is absent before editing**

Run: `rg -n "static_mesh_showcase|MeshGroundStability|CreatePhysicsSceneEntries|DynamicMixedStack" C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsNintendoDsSceneGenerator.cs`

Expected: no `static_mesh_showcase` scene id or curated menu entry exists yet.

- [ ] **Step 2: Add the failing coverage target**

Define the new stable scene ids up front:
- desktop scene id: `scenes/physics/test_scene_static_mesh_showcase.helen`
- DS companion scene id: `test_scene_static_mesh_showcase_ds`
- runtime menu scene id: `test_scene_static_mesh_showcase`

Edit:
- `PhysicsSceneCatalog.cs` to add the desktop scene id constant and include it in `SceneIds`
- `DemoDiscSceneCatalog.cs` to add one curated `DemoDiscPhysicsSceneEntry`
- `PhysicsNintendoDsSceneGenerator.cs` so the new curated entry is treated as a playable showcase scene

- [ ] **Step 3: Re-read the routing files**

Run: `rg -n "test_scene_static_mesh_showcase|physics-static-mesh|Static Mesh|SceneIds" C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsNintendoDsSceneGenerator.cs`

Expected: the new scene id appears in the catalog, curated menu entry set, and DS playable-scene filter.

- [ ] **Step 4: Commit the menu-routing slice**

```bash
git -C C:\dev\helprojs\city add assets/codebase/physics.tools/PhysicsSceneCatalog.cs assets/codebase/menu/DemoDiscSceneCatalog.cs assets/codebase/physics.tools/PhysicsNintendoDsSceneGenerator.cs
git -C C:\dev\helprojs\city commit -m "Add curated static mesh physics scene ids"
```

### Task 2: Author The Static-Mesh Showcase Scene Through The City Generator

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase.helen`
- Modify: `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase_ds.helen`

- [ ] **Step 1: Confirm the generator does not yet expose the new scene**

Run: `rg -n "CreateMeshGroundStabilityScene|StaticMeshCollider|CreatePlayablePhysicsShowcaseSceneDefinition|test_scene_static_mesh_showcase" C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneFactory.cs`

Expected: there is no `test_scene_static_mesh_showcase` branch and no helper that serializes the intended showcase layout yet.

- [ ] **Step 2: Add the failing authored-output target**

Run: `if (Test-Path 'C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase.helen') { Write-Error 'scene already exists' }`

Expected: the file does not exist yet.

- [ ] **Step 3: Implement the new scene generator path**

Edit `PhysicsSceneFactory.cs` to:
- route the new scene id in `CreateSceneAsset`
- treat the new scene as a playable physics showcase in `IsPlayablePhysicsShowcaseScene`
- extend `CreatePlayablePhysicsShowcaseSceneDefinition` camera selection for the new scene
- add a new scene builder that authors:
  - a broad flat start area
  - several wall boundaries
  - one or two slopes
  - a visible spawn area
- add whatever helper is required to serialize a real `StaticMeshCollider3DComponent` payload for the environment geometry instead of primitive-only collider records

- [ ] **Step 4: Regenerate the authored scene outputs**

Use the city editor command path that already owns these generated scenes:
- run `Generate Physics Scenes`
- run `Generate Physics DS Scenes`

Expected: both desktop and DS `.helen` outputs are rewritten with the new scene included.

- [ ] **Step 5: Verify the generated authored assets**

Run: `rg -n "test_scene_static_mesh_showcase|StaticMesh|Static Mesh|WalkerSpawn|Sphere|ShowcaseUi" C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase.helen C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_showcase_ds.helen`

Expected: the desktop scene exists, the DS companion scene exists, and the scene metadata references the new showcase content.

- [ ] **Step 6: Commit the authored-scene slice**

```bash
git -C C:\dev\helprojs\city add assets/codebase/physics.tools/PhysicsSceneFactory.cs assets/scenes/physics/test_scene_static_mesh_showcase.helen assets/scenes/physics/test_scene_static_mesh_showcase_ds.helen
git -C C:\dev\helprojs\city commit -m "Add static mesh physics showcase scene"
```

### Task 3: Add A Packaged Startup Override In The GameCube Host

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.cpp`
- Inspect: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeApplication.cpp`

- [ ] **Step 1: Verify the current packaged startup path has no override**

Run: `rg -n "GetPackagedStartupSceneId|he_get_runtime_gamecube_startup_scene_id|override|HELENGINE_GAMECUBE" C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.hpp C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.cpp`

Expected: `GetPackagedStartupSceneId()` returns the manifest startup scene id directly with no direct-test override path.

- [ ] **Step 2: Add the override contract**

Implement one explicit packaged startup override using an environment-variable-friendly contract:
- environment variable name: `HELENGINE_GAMECUBE_PACKAGED_STARTUP_SCENE_ID`
- if unset or whitespace: return `he_get_runtime_gamecube_startup_scene_id()`
- if set: return the override scene id and log that the override is active

Keep the `GameCubeApplication.cpp` call sites unchanged so both packaged boot phases continue to flow through `GetPackagedStartupSceneId()`.

- [ ] **Step 3: Re-read the startup flow**

Run: `rg -n "HELENGINE_GAMECUBE_PACKAGED_STARTUP_SCENE_ID|GetPackagedStartupSceneId|LoadScene\\(" C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.hpp C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneBootstrap.cpp C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeApplication.cpp`

Expected: the override helper exists and both runtime packaged-load call sites still go through the single scene-bootstrap accessor.

- [ ] **Step 4: Commit the startup-override slice**

```bash
git -C C:\dev\helworks\helengine-gc add src/platform/gamecube/GameCubeSceneBootstrap.hpp src/platform/gamecube/GameCubeSceneBootstrap.cpp
git -C C:\dev\helworks\helengine-gc commit -m "Add packaged startup scene override"
```

### Task 4: Stage The Packaged Physics Content And Verify Both Boot Paths

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\tools\stage_city_demo_disc_main_menu_content.sh`
- Inspect: `C:\dev\helprojs\city\gamecube-build\disc\files\cooked\scenes\physics`
- Inspect: `C:\dev\helworks\helengine-gc\tmp\city-packaged-gcm-root\files\cooked\scenes\physics`

- [ ] **Step 1: Verify the current staging script omits physics scenes**

Run: `rg -n "scenes/rendering|scenes/physics|DemoDiscMainMenu" C:\dev\helworks\helengine-gc\tools\stage_city_demo_disc_main_menu_content.sh`

Expected: the script stages the main menu and rendering scenes but does not copy the curated physics cooked scenes.

- [ ] **Step 2: Update staging to include the curated physics scenes**

Edit `tools/stage_city_demo_disc_main_menu_content.sh` so the staged packaged-content root copies the physics showcase cooked scenes required by the menu, including the new static-mesh showcase scene.

- [ ] **Step 3: Run the smallest smoke build path**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform gamecube -Output C:\dev\helprojs\city\gamecube-build
```

Then run:

```bash
bash C:/dev/helworks/helengine-gc/tools/stage_city_demo_disc_main_menu_content.sh
```

Expected: the city GameCube build completes and the staged packaged-content root now contains the new physics cooked scene.

- [ ] **Step 4: Verify default packaged boot content**

Run: `rg -n "test_scene_static_mesh_showcase|DemoDiscMainMenu" C:\dev\helprojs\city\gamecube-build\disc\files\cooked\scenes\physics C:\dev\helworks\helengine-gc\tmp\city-packaged-gcm-root\files\cooked\scenes`

Expected: the new scene is present in the cooked output and the staged packaged-content root.

- [ ] **Step 5: Launch default packaged boot and confirm menu path**

Run the normal GameCube launch flow with no startup override and verify:
- boot reaches `DemoDiscMainMenu`
- the `Physics Scenes` panel contains the new `Static Mesh` showcase entry
- selecting it loads the new scene

- [ ] **Step 6: Launch direct packaged boot override**

Run the same launch flow with:

```powershell
$env:HELENGINE_GAMECUBE_PACKAGED_STARTUP_SCENE_ID='test_scene_static_mesh_showcase'
```

Expected: the packaged runtime boots directly into the new showcase scene without changing authored menu content.

- [ ] **Step 7: Commit the host/staging validation slice**

```bash
git -C C:\dev\helworks\helengine-gc add tools/stage_city_demo_disc_main_menu_content.sh
git -C C:\dev\helworks\helengine-gc commit -m "Stage physics scenes for GameCube menu content"
```

## Notes

- Do not add a native C++ test scene for this feature. The direct path must load the same packaged authored scene id as the menu path.
- Reuse the existing engine-side static-mesh cook and runtime support already implemented in `helengine`; this plan is about consuming that path from generated city content and GameCube packaged boot flow.
- If the city generator cannot express `StaticMeshCollider3DComponent` cleanly with the current helper set, add the narrowest possible serialization helper in `PhysicsSceneFactory.cs` rather than rewriting generated `.helen` files by hand.
