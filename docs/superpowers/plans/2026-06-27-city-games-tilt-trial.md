# City Games Tilt Trial Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a new game-owned `Tilt Trial` scene path with a box-built tilt course, stage-tilt gameplay, a dedicated follow camera, a soft ball reset, and a new `Games` top-level menu category.

**Architecture:** Introduce a new runtime `game` module for the tilt controller, reset controller, and game follow camera; introduce a new editor `game.tools` module for generated game scenes; keep the demo-disc menu in `menu` but extend it with a new `Games` section that loads the generated game scene. Validation stays source-driven in the engine test project plus one Windows smoke build that boots directly into the new game scene.

**Tech Stack:** C#, city asset codegen/editor tooling, helengine editor tests, generated `.helen` scenes, Windows runtime smoke testing

---

### Task 1: Lock The New Game Scene Boundary In Source Tests

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameSceneSourceTests.cs`
- Inspect: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscMenuDefinitionProvider.cs`
- Inspect: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs`
- Inspect: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneCatalog.cs`
- Inspect: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs`

- [ ] **Step 1: Write the failing test file**

```csharp
namespace helengine.editor.tests;

/// <summary>
/// Verifies the authored city game-scene source keeps the intended Tilt Trial wiring.
/// </summary>
public sealed class CityGameSceneSourceTests {
    /// <summary>
    /// Ensures the new game catalog exports the Tilt Trial scene id.
    /// </summary>
    [Fact]
    public void City_game_scene_catalog_source_exports_tilt_trial_scene() {
        string sourcePath = @"C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneCatalog.cs";
        string source = File.ReadAllText(sourcePath);

        Assert.Contains("public const string TiltTrialSceneId = \"scenes/games/tilt_trial.helen\";", source, StringComparison.Ordinal);
        Assert.Contains("TiltTrialSceneId,", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the demo-disc menu exposes a top-level Games panel and one Tilt Trial item.
    /// </summary>
    [Fact]
    public void City_demo_disc_menu_source_exposes_games_panel_and_tilt_trial_item() {
        string menuProviderPath = @"C:\dev\helprojs\city\assets\codebase\menu\DemoDiscMenuDefinitionProvider.cs";
        string menuProviderSource = File.ReadAllText(menuProviderPath);
        string sceneCatalogPath = @"C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs";
        string sceneCatalogSource = File.ReadAllText(sceneCatalogPath);

        Assert.Contains("\"main-games\", \"Games\"", menuProviderSource, StringComparison.Ordinal);
        Assert.Contains("\"games-select\",", menuProviderSource, StringComparison.Ordinal);
        Assert.Contains("sceneCatalog.CreateGameSceneItems()", menuProviderSource, StringComparison.Ordinal);
        Assert.Contains("\"games-tilt-trial\"", sceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("\"Tilt Trial\"", sceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("\"tilt_trial\"", sceneCatalogSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the game-scene factory builds Tilt Trial from box gameplay geometry and game-owned runtime components.
    /// </summary>
    [Fact]
    public void City_tilt_trial_source_uses_box_course_stage_tilt_reset_and_game_camera() {
        string sourcePath = @"C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs";
        string source = File.ReadAllText(sourcePath);

        Assert.Contains("CreateTiltTrialScene()", source, StringComparison.Ordinal);
        Assert.Contains("\"StageRoot\"", source, StringComparison.Ordinal);
        Assert.Contains("\"StartPad\"", source, StringComparison.Ordinal);
        Assert.Contains("\"Ramp\"", source, StringComparison.Ordinal);
        Assert.Contains("\"GoalPad\"", source, StringComparison.Ordinal);
        Assert.Contains("\"CatchFloor\"", source, StringComparison.Ordinal);
        Assert.Contains("\"PlayerSphere\"", source, StringComparison.Ordinal);
        Assert.Contains("new city.game.DemoTiltStageComponent", source, StringComparison.Ordinal);
        Assert.Contains("new city.game.DemoTiltBallResetComponent", source, StringComparison.Ordinal);
        Assert.Contains("new city.game.DemoTiltFollowCameraComponent", source, StringComparison.Ordinal);
    }
}
```

- [ ] **Step 2: Run the new source tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameSceneSourceTests
```

Expected:

```text
FAIL
Could not find file 'C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneCatalog.cs'
```

- [ ] **Step 3: Keep the red test output in a short log for comparison**

Run:

```powershell
$o = rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameSceneSourceTests 2>&1 | Out-String
Set-Content C:\dev\helworks\helengine-gc\city-game-scene-tests-red.log $o
```

Expected:

```text
The log contains the missing-file or missing-string failures for the new game scene source.
```

- [ ] **Step 4: Verify no existing test file needs to be modified for this slice**

Run:

```powershell
rtk rg -n "CityGameSceneSourceTests|TiltTrialSceneId|main-games" C:\dev\helworks\helengine\engine\helengine.editor.tests
```

Expected:

```text
Only the new CityGameSceneSourceTests.cs path appears.
```

- [ ] **Step 5: Commit the failing-test checkpoint**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/CityGameSceneSourceTests.cs
git -C C:\dev\helworks\helengine commit -m "test: cover tilt trial game scene source"
```

### Task 2: Add The Runtime Game Module And Controllers

**Files:**
- Create: `C:\dev\helprojs\city\assets\codebase\game\code.module.json`
- Create: `C:\dev\helprojs\city\assets\codebase\game\DemoTiltStageComponent.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\game\DemoTiltBallResetComponent.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\game\DemoTiltFollowCameraComponent.cs`
- Inspect: `C:\dev\helprojs\city\assets\codebase\rendering\DemoFollowCameraComponent.cs`

- [ ] **Step 1: Create the new runtime module manifest**

```json
{
  "moduleId": "game",
  "dependencyModuleIds": [
    "menu"
  ],
  "loadScopes": [
    "always-loaded"
  ],
  "moduleKind": "runtime"
}
```

- [ ] **Step 2: Add the stage tilt controller**

```csharp
namespace city.game {
    /// <summary>
    /// Rotates the authored tilt stage from keyboard and gamepad input so gravity drives the player sphere across the course.
    /// </summary>
    public sealed class DemoTiltStageComponent : UpdateComponent {
        const double GamepadDeadzone = 0.18d;

        /// <summary>
        /// Gets or sets the maximum tilt angle in radians applied on pitch and roll.
        /// </summary>
        public float MaximumTiltRadians { get; set; }

        /// <summary>
        /// Gets or sets the tilt speed in radians per second.
        /// </summary>
        public float TiltSpeedRadians { get; set; }

        float CurrentPitchRadians;
        float CurrentRollRadians;

        /// <summary>
        /// Initializes medium-tilt defaults for Tilt Trial.
        /// </summary>
        public DemoTiltStageComponent() {
            MaximumTiltRadians = 0.2617994f;
            TiltSpeedRadians = 1.8f;
        }

        /// <summary>
        /// Advances the stage tilt from `WASD` and left-stick input.
        /// </summary>
        public override void Update() {
            Core core = Core.Instance ?? throw new InvalidOperationException("A core instance must exist before stage tilt updates can run.");
            InputSystem inputSystem = core.Input ?? throw new InvalidOperationException("Tilt Trial requires an initialized input system.");
            double elapsedSeconds = core.FrameDeltaSeconds;
            double tiltX = ResolveRollInput(inputSystem);
            double tiltY = ResolvePitchInput(inputSystem);

            CurrentRollRadians = Math.Clamp(CurrentRollRadians + (float)(tiltX * TiltSpeedRadians * elapsedSeconds), -MaximumTiltRadians, MaximumTiltRadians);
            CurrentPitchRadians = Math.Clamp(CurrentPitchRadians + (float)(tiltY * TiltSpeedRadians * elapsedSeconds), -MaximumTiltRadians, MaximumTiltRadians);
            Parent.LocalOrientation = float4.CreateFromYawPitchRoll(0f, CurrentPitchRadians, CurrentRollRadians);
        }
    }
}
```

- [ ] **Step 3: Add the fall reset controller**

```csharp
namespace city.game {
    /// <summary>
    /// Respawns the Tilt Trial player sphere when it falls below the configured reset height.
    /// </summary>
    public sealed class DemoTiltBallResetComponent : UpdateComponent {
        /// <summary>
        /// Gets or sets the serialized target sphere reference.
        /// </summary>
        public SceneEntityReference TargetEntityReference { get; set; }

        /// <summary>
        /// Gets or sets the reset world height.
        /// </summary>
        public float ResetHeight { get; set; }

        /// <summary>
        /// Gets or sets the respawn position.
        /// </summary>
        public float3 SpawnPosition { get; set; }

        /// <summary>
        /// Gets or sets the respawn orientation.
        /// </summary>
        public float4 SpawnOrientation { get; set; }

        /// <summary>
        /// Restores the sphere when it falls below the reset plane.
        /// </summary>
        public override void Update() {
            Entity sphereEntity = ResolveTargetEntity();
            if (sphereEntity.Position.Y >= ResetHeight) {
                return;
            }

            sphereEntity.LocalPosition = SpawnPosition;
            sphereEntity.LocalOrientation = SpawnOrientation;
            PhysicsComponent physicsComponent = sphereEntity.GetComponent<PhysicsComponent>() ?? throw new InvalidOperationException("Tilt Trial reset requires a physics component on the tracked sphere.");
            physicsComponent.LinearVelocity = float3.Zero;
            physicsComponent.AngularVelocity = float3.Zero;
        }
    }
}
```

- [ ] **Step 4: Add the dedicated game follow camera**

```csharp
namespace city.game {
    /// <summary>
    /// Follows the Tilt Trial sphere while orbiting from arrow keys and right-stick input.
    /// </summary>
    public sealed class DemoTiltFollowCameraComponent : UpdateComponent {
        /// <summary>
        /// Gets or sets the serialized sphere reference.
        /// </summary>
        public SceneEntityReference TargetEntityReference { get; set; }

        /// <summary>
        /// Gets or sets the target offset above the sphere center.
        /// </summary>
        public float3 TargetOffset { get; set; }

        /// <summary>
        /// Advances orbit yaw and pitch from arrows and right stick, then reapplies the camera pose.
        /// </summary>
        public override void Update() {
            Core core = Core.Instance ?? throw new InvalidOperationException("A core instance must exist before follow camera updates can run.");
            InputSystem inputSystem = core.Input ?? throw new InvalidOperationException("Tilt Trial requires an initialized input system.");
            double elapsedSeconds = core.FrameDeltaSeconds;
            double yawInput = ResolveYawInput(inputSystem);
            double pitchInput = ResolvePitchInput(inputSystem);

            CurrentYawRadians += (float)(yawInput * ManualYawSpeedRadians * elapsedSeconds);
            CurrentPitchRadians -= (float)(pitchInput * ManualPitchSpeedRadians * elapsedSeconds);
            CurrentPitchRadians = ClampPitch(CurrentPitchRadians);

            ApplyOrbitPose();
        }
    }
}
```

- [ ] **Step 5: Commit the runtime module checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/codebase/game/code.module.json assets/codebase/game/DemoTiltStageComponent.cs assets/codebase/game/DemoTiltBallResetComponent.cs assets/codebase/game/DemoTiltFollowCameraComponent.cs
git -C C:\dev\helprojs\city commit -m "feat: add tilt trial runtime controllers"
```

### Task 3: Add The Editor Game Scene Factory And Generator

**Files:**
- Create: `C:\dev\helprojs\city\assets\codebase\game.tools\code.module.json`
- Create: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneCatalog.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneGenerator.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\menu.tools\GenerateGameScenesCommand.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\code.module.json`

- [ ] **Step 1: Create the editor module manifest for generated game scenes**

```json
{
  "moduleId": "game.tools",
  "dependencyModuleIds": [
    "game",
    "menu",
    "rendering.tools"
  ],
  "loadScopes": [
    "always-loaded"
  ],
  "moduleKind": "editor"
}
```

- [ ] **Step 2: Add the game scene catalog**

```csharp
namespace city.game.tools {
    /// <summary>
    /// Enumerates the exportable game-owned scenes authored for playable demo-disc content.
    /// </summary>
    public static class GameSceneCatalog {
        /// <summary>
        /// Relative scene id for the first tilt-course prototype.
        /// </summary>
        public const string TiltTrialSceneId = "scenes/games/tilt_trial.helen";

        static readonly string[] SceneIds = new[] {
            TiltTrialSceneId
        };

        /// <summary>
        /// Gets the stable ordered list of exportable game scene ids.
        /// </summary>
        public static string[] GetSceneIds() {
            string[] copy = new string[SceneIds.Length];
            Array.Copy(SceneIds, copy, SceneIds.Length);
            return copy;
        }
    }
}
```

- [ ] **Step 3: Add the game scene factory and first Tilt Trial scene**

```csharp
namespace city.game.tools {
    /// <summary>
    /// Creates exportable game scenes for player-facing demo-disc content.
    /// </summary>
    public sealed class GameSceneFactory {
        /// <summary>
        /// Writes all generated game scenes into the active city project.
        /// </summary>
        public void WriteScenes(string projectRootPath) {
            WriteScene(projectRootPath, CreateTiltTrialScene());
        }

        SceneAsset CreateTiltTrialScene() {
            SceneEntityAsset scenarioEntity = CreateScenarioRoot("tilt_trial.scenario", new[] {
                CreateGameplayBoxEntity("tilt_trial.stage_root", "StageRoot", float3.Zero, float3.One, float4.Identity, CreateStageChildren()),
                CreatePhysicsSphereMeshEntity("tilt_trial.player", "PlayerSphere", new float3(0f, 1.2f, -4f), float4.Identity, DynamicBodyKindCode, true, CreateSphereStackMaterialReference(0)),
                CreateMarkerEntity("tilt_trial.spawn", "PlayerSpawn", new float3(0f, 1.2f, -4f)),
                CreateMarkerEntity("tilt_trial.goal", "GoalMarker", new float3(0f, 2.15f, 5.5f))
            });
            SceneEntityAsset cameraEntity = CreateTiltTrialCameraEntity("tilt_trial.camera", new float3(6.5f, 4.5f, -8f), CreateYawPitchRollDegrees(145.0, -18.0, 0.0), FindRequiredSceneEntityAssetByName(scenarioEntity.Children, "PlayerSphere").Id);
            return CreateGameSceneAsset(GameSceneCatalog.TiltTrialSceneId, cameraEntity, scenarioEntity);
        }
    }
}
```

- [ ] **Step 4: Add the generator command and wire the editor module dependency**

```csharp
using city.game.tools;

namespace city.menu.tools {
    /// <summary>
    /// Generates the authored game-scene set inside the active project.
    /// </summary>
    public sealed class GenerateGameScenesCommand : IEditorCommand {
        /// <summary>
        /// Gets the stable editor command identifier.
        /// </summary>
        public string CommandId => "menu.generate-game-scenes";

        /// <summary>
        /// Gets the human-readable command label.
        /// </summary>
        public string DisplayName => "Generate Game Scenes";

        /// <summary>
        /// Generates the current authored game scenes.
        /// </summary>
        public void Execute(IEditorCommandContext context) {
            if (context == null) {
                throw new ArgumentNullException(nameof(context));
            }

            GameSceneGenerator generator = new GameSceneGenerator();
            generator.Generate(context.ProjectRootPath);
        }
    }
}
```

```json
{
  "moduleId": "menu.tools",
  "dependencyModuleIds": [
    "game.tools",
    "physics.tools",
    "rendering.tools"
  ],
  "loadScopes": [
    "always-loaded"
  ],
  "moduleKind": "editor"
}
```

- [ ] **Step 5: Commit the generated game-scene pipeline checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/codebase/game.tools/code.module.json assets/codebase/game.tools/GameSceneCatalog.cs assets/codebase/game.tools/GameSceneFactory.cs assets/codebase/game.tools/GameSceneGenerator.cs assets/codebase/menu.tools/GenerateGameScenesCommand.cs assets/codebase/menu.tools/code.module.json
git -C C:\dev\helprojs\city commit -m "feat: add tilt trial game scene generation"
```

### Task 4: Extend The Demo Menu With A Games Category

**Files:**
- Create: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscGameSceneEntry.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscSceneCatalog.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscMenuDefinitionProvider.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameSceneSourceTests.cs`

- [ ] **Step 1: Add a dedicated menu entry type for game-owned scenes**

```csharp
namespace city.menu {
    /// <summary>
    /// Stores one curated playable game scene entry shown by the demo-disc Games panel.
    /// </summary>
    public sealed class DemoDiscGameSceneEntry {
        /// <summary>
        /// Initializes one curated game scene entry.
        /// </summary>
        public DemoDiscGameSceneEntry(string menuItemId, string displayName, string sceneId) {
            if (string.IsNullOrWhiteSpace(menuItemId)) {
                throw new ArgumentException("Menu item id must be provided.", nameof(menuItemId));
            } else if (string.IsNullOrWhiteSpace(displayName)) {
                throw new ArgumentException("Display name must be provided.", nameof(displayName));
            } else if (string.IsNullOrWhiteSpace(sceneId)) {
                throw new ArgumentException("Scene id must be provided.", nameof(sceneId));
            }

            MenuItemId = menuItemId;
            DisplayName = displayName;
            SceneId = sceneId;
        }

        public string MenuItemId { get; }
        public string DisplayName { get; }
        public string SceneId { get; }
    }
}
```

- [ ] **Step 2: Extend the scene catalog with game entries and game menu items**

```csharp
public MenuItemDefinition[] CreateGameSceneItems() {
    IReadOnlyList<DemoDiscGameSceneEntry> gameSceneEntries = CreateGameSceneEntries();
    MenuItemDefinition[] items = new MenuItemDefinition[gameSceneEntries.Count + 1];
    for (int index = 0; index < gameSceneEntries.Count; index++) {
        DemoDiscGameSceneEntry sceneEntry = gameSceneEntries[index];
        items[index] = new MenuItemDefinition(
            sceneEntry.MenuItemId,
            sceneEntry.DisplayName,
            true,
            new MenuActionDefinition(MenuActionKind.LoadScene, sceneEntry.SceneId));
    }

    items[gameSceneEntries.Count] = new MenuItemDefinition("games-back", "Back", true, new MenuActionDefinition(MenuActionKind.Back, string.Empty));
    return items;
}

public IReadOnlyList<DemoDiscGameSceneEntry> CreateGameSceneEntries() {
    return [
        new DemoDiscGameSceneEntry(
            "games-tilt-trial",
            "Tilt Trial",
            "tilt_trial")
    ];
}
```

- [ ] **Step 3: Add the top-level Games panel beside Demo Scenes and Physics Scenes**

```csharp
new MenuPanelDefinition(
    "main",
    "Main Menu",
    6,
    new[] {
        new MenuItemDefinition("main-scenes", "Demo Scenes", true, new MenuActionDefinition(MenuActionKind.OpenPanel, "scene-select")),
        new MenuItemDefinition("main-physics", "Physics Scenes", true, new MenuActionDefinition(MenuActionKind.OpenPanel, "physics-select")),
        new MenuItemDefinition("main-games", "Games", true, new MenuActionDefinition(MenuActionKind.OpenPanel, "games-select")),
        new MenuItemDefinition("main-options", "Options", true, new MenuActionDefinition(MenuActionKind.OpenPanel, "options"))
    }),
new MenuPanelDefinition(
    "games-select",
    "Games",
    4,
    sceneCatalog.CreateGameSceneItems()),
```

- [ ] **Step 4: Re-run only the new source tests and get them green**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameSceneSourceTests
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the menu integration checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/codebase/menu/DemoDiscGameSceneEntry.cs assets/codebase/menu/DemoDiscSceneCatalog.cs assets/codebase/menu/DemoDiscMenuDefinitionProvider.cs
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/CityGameSceneSourceTests.cs
git -C C:\dev\helprojs\city commit -m "feat: add games menu category"
git -C C:\dev\helworks\helengine commit -m "test: verify tilt trial menu source"
```

### Task 5: Generate Assets, Wire Direct Launch, And Run A Windows Smoke Test

**Files:**
- Modify: `C:\dev\helprojs\city\user_settings\build_config.json`
- Generate: `C:\dev\helprojs\city\assets\scenes\games\tilt_trial.helen`
- Generate: `C:\dev\helprojs\city\assets\Scenes\DemoDiscMainMenu.helen`
- Verify: `C:\dev\helprojs\output\windows\helengine_windows.exe`

- [ ] **Step 1: Add the new game scene id to the Windows build config so it can be packaged and directly started**

```json
{
  "platformId": "windows",
  "selectedSceneIds": [
    "DemoDiscMainMenu",
    "cube_test",
    "scaled_cube",
    "colored_cube_grid",
    "textured_cube_grid",
    "axis_test",
    "axis_test2",
    "directional_shadow_plaza",
    "test_scene_dynamic_stack_boxes",
    "test_scene_dynamic_sphere_stack",
    "test_scene_dynamic_mixed_stack",
    "test_scene_static_mesh_showcase",
    "test_scene_static_mesh_minimal",
    "tilt_trial"
  ],
  "sceneOrders": [
    { "sceneId": "DemoDiscMainMenu", "orderNumber": 1 },
    { "sceneId": "tilt_trial", "orderNumber": 14 }
  ]
}
```

- [ ] **Step 2: Regenerate the new game scene and the demo-disc main menu**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.generate-game-scenes
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.regenerate-demo-disc-main-menu
```

Expected:

```text
The commands complete successfully and write tilt_trial.helen plus an updated DemoDiscMainMenu scene.
```

- [ ] **Step 3: Temporarily place `tilt_trial` first for the smoke build, build Windows output, and launch**

Run:

```powershell
# Temporarily move tilt_trial to order 1 in user_settings/build_config.json, keeping the file change local to verification.
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --build windows --output C:\dev\helprojs\output\windows-tilt-trial
Start-Process -FilePath 'C:\dev\helprojs\output\windows-tilt-trial\helengine_windows.exe' -WorkingDirectory 'C:\dev\helprojs\output\windows-tilt-trial'
```

Expected:

```text
The Windows build boots directly into Tilt Trial without a fatal startup exception.
```

- [ ] **Step 4: Restore main-menu-first ordering and run the broader targeted source suite**

Run:

```powershell
# Restore DemoDiscMainMenu to order 1 while keeping tilt_trial included in selectedSceneIds and sceneOrders.
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter "FullyQualifiedName~CityGameSceneSourceTests|FullyQualifiedName~CityPhysicsSceneSourceTests"
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the generated scene and build-config checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/scenes/games/tilt_trial.helen assets/Scenes/DemoDiscMainMenu.helen user_settings/build_config.json
git -C C:\dev\helprojs\city commit -m "feat: add tilt trial generated game scene"
```

### Task 6: Final Verification And Handoff

**Files:**
- Verify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameSceneSourceTests.cs`
- Verify: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs`
- Verify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscMenuDefinitionProvider.cs`
- Verify: `C:\dev\helprojs\city\assets\codebase\game\DemoTiltStageComponent.cs`

- [ ] **Step 1: Run the exact targeted test suite one final time**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter "FullyQualifiedName~CityGameSceneSourceTests|FullyQualifiedName~CityPhysicsSceneSourceTests"
```

Expected:

```text
PASS
```

- [ ] **Step 2: Rebuild the generated city assets one final time**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.generate-game-scenes
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.regenerate-demo-disc-main-menu
```

Expected:

```text
No scene-generation exceptions.
```

- [ ] **Step 3: Capture a final Windows runtime smoke log**

Run:

```powershell
$process = Start-Process -FilePath 'C:\dev\helprojs\output\windows-tilt-trial\helengine_windows.exe' -WorkingDirectory 'C:\dev\helprojs\output\windows-tilt-trial' -PassThru
Start-Sleep -Seconds 10
if (!$process.HasExited) { Stop-Process -Id $process.Id }
```

Expected:

```text
The process stays alive during the smoke window and does not report a fatal startup failure in the runtime logs.
```

- [ ] **Step 4: Review git status in both repos before reporting completion**

Run:

```powershell
rtk git -C C:\dev\helprojs\city status --short
rtk git -C C:\dev\helworks\helengine status --short
```

Expected:

```text
Only intentional Tilt Trial changes remain uncommitted, or both repos are clean if every checkpoint commit landed cleanly.
```

- [ ] **Step 5: Commit any final touch-ups and summarize verification**

```bash
git -C C:\dev\helprojs\city add assets/codebase/game/code.module.json assets/codebase/game/DemoTiltStageComponent.cs assets/codebase/game/DemoTiltBallResetComponent.cs assets/codebase/game/DemoTiltFollowCameraComponent.cs assets/codebase/game.tools/code.module.json assets/codebase/game.tools/GameSceneCatalog.cs assets/codebase/game.tools/GameSceneFactory.cs assets/codebase/game.tools/GameSceneGenerator.cs assets/codebase/menu/DemoDiscGameSceneEntry.cs assets/codebase/menu/DemoDiscSceneCatalog.cs assets/codebase/menu/DemoDiscMenuDefinitionProvider.cs assets/codebase/menu.tools/GenerateGameScenesCommand.cs assets/codebase/menu.tools/code.module.json assets/scenes/games/tilt_trial.helen user_settings/build_config.json
git -C C:\dev\helprojs\city commit -m "feat: finalize tilt trial gameplay prototype"
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/CityGameSceneSourceTests.cs
git -C C:\dev\helworks\helengine commit -m "test: finalize tilt trial source coverage"
```

## Self-Review

### Spec coverage

- New `GameSceneFactory` and `GameSceneCatalog`: covered by Tasks 1 and 3.
- New `Games` menu category: covered by Task 4.
- Tiny box-built course: covered by Task 3.
- Authentic stage tilt with medium clamp: covered by Task 2.
- Soft reset below height: covered by Task 2.
- Dedicated follow camera with split controls: covered by Task 2.
- Direct startup for testing: covered by Task 5.
- Windows smoke validation: covered by Tasks 5 and 6.

### Placeholder scan

- No `TODO` or `TBD` markers remain in the plan body.
- The code snippets now name the concrete runtime types, scene ids, menu ids, and verification commands needed for execution.

### Type consistency

- `TiltTrialSceneId`, `DemoTiltStageComponent`, `DemoTiltBallResetComponent`, and `DemoTiltFollowCameraComponent` are named consistently across tests, menu wiring, and factory tasks.
- The internal menu scene id is consistently `tilt_trial`, while the authored asset path is consistently `scenes/games/tilt_trial.helen`.
