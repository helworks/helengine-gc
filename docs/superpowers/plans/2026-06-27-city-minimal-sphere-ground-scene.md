# City Minimal Sphere Ground Scene Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a new generated playable physics scene that contains only one controllable sphere, one ground cube, and the existing third-person follow camera path.

**Architecture:** Extend the existing generated physics-scene catalog with one new playable scene id, then add a sibling scene generator in `PhysicsSceneFactory` that reuses the current `PlayerSphere` and `DemoFollowCameraComponent` flow. Keep the implementation narrow by reusing the existing playable-scene authoring pipeline, including the post-save-id camera target rebinding path.

**Tech Stack:** C#, xUnit, generated `.helen` scene assets, HelEngine editor scene-generation pipeline, Windows runtime build flow

---

## File Structure

**Modify:**

- `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs`
  - Add the new minimal playable scene id and include it in the stable ordered scene-id list.
- `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneFactory.cs`
  - Add the new authored scene generator, wire it into playable-scene detection, normalize its short id, create its live camera path, and reuse the existing camera-target rebinding pattern.
- `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityPhysicsSceneSourceTests.cs`
  - Add the failing source-level regression test first, then expand assertions to cover the final generated-scene path.
- `C:\dev\helprojs\city\user_settings\build_config.json`
  - Temporarily reorder the Windows startup scene only during direct-test verification, then restore it.

**Generate / verify:**

- `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_minimal.helen`
  - Generated desktop scene output.
- `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_minimal_ds.helen`
  - Generated DS companion scene output if the existing generation command emits one for playable scenes.
- `C:\dev\helprojs\city\windows-build-staticmesh\`
  - Windows direct-test build output used for runtime verification.

### Task 1: Add the Failing Catalog and Source Tests

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityPhysicsSceneSourceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`

- [ ] **Step 1: Write the failing source regression tests**

Add two tests near the existing physics source assertions:

```csharp
/// <summary>
/// Ensures the physics scene catalog exports the minimal static-mesh playable scene.
/// </summary>
[Fact]
public void City_physics_scene_catalog_source_exports_static_mesh_minimal_scene() {
    string sourcePath = @"C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs";
    string source = File.ReadAllText(sourcePath);

    Assert.Contains("public const string StaticMeshMinimalSceneId = \"scenes/physics/test_scene_static_mesh_minimal.helen\";", source, StringComparison.Ordinal);
    Assert.Contains("StaticMeshMinimalSceneId,", source, StringComparison.Ordinal);
}

/// <summary>
/// Ensures the minimal static-mesh playable scene keeps only a ground cube, player sphere, and follow-camera playable path.
/// </summary>
[Fact]
public void City_static_mesh_minimal_scene_source_uses_ground_player_sphere_and_follow_camera() {
    string sourcePath = @"C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneFactory.cs";
    string source = File.ReadAllText(sourcePath);

    Assert.Contains("CreateStaticMeshMinimalScene()", source, StringComparison.Ordinal);
    Assert.Contains("test_scene_static_mesh_minimal", source, StringComparison.Ordinal);
    Assert.Contains("\"Ground\"", source, StringComparison.Ordinal);
    Assert.Contains("\"PlayerSphere\"", source, StringComparison.Ordinal);
    Assert.Contains("CreateLiveStaticMeshShowcaseCameraEntity(", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the tests to verify they fail for the expected missing-scene reason**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityPhysicsSceneSourceTests -v minimal
```

Expected:

```text
FAIL City_physics_scene_catalog_source_exports_static_mesh_minimal_scene
FAIL City_static_mesh_minimal_scene_source_uses_ground_player_sphere_and_follow_camera
```

The expected failure mode is missing `StaticMeshMinimalSceneId` and missing `CreateStaticMeshMinimalScene` references in source, not a compile error.

- [ ] **Step 3: Commit the failing tests**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor.tests/CityPhysicsSceneSourceTests.cs
git commit -m "test: cover minimal static mesh scene source"
```

### Task 2: Implement the Minimal Playable Scene and Make the Tests Pass

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneCatalog.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\physics.tools\PhysicsSceneFactory.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityPhysicsSceneSourceTests.cs`

- [ ] **Step 1: Add the new catalog id and stable export ordering**

In `PhysicsSceneCatalog.cs`, add the new constant and include it in `SceneIds` immediately after `StaticMeshShowcaseSceneId` so it stays grouped with the playable physics scenes:

```csharp
/// <summary>
/// Relative scene id for the minimal playable static-mesh validation scene.
/// </summary>
public const string StaticMeshMinimalSceneId = "scenes/physics/test_scene_static_mesh_minimal.helen";
```

And in the stable list:

```csharp
static readonly string[] SceneIds = new[] {
    CharacterSlopeSceneId,
    CharacterStepsSceneId,
    CharacterMovingPlatformSceneId,
    DynamicStackBoxesSceneId,
    SingleFallingCubeSceneId,
    DynamicSphereStackSceneId,
    DynamicMixedStackSceneId,
    KinematicPushSceneId,
    MeshGroundStabilitySceneId,
    StaticMeshShowcaseSceneId,
    StaticMeshMinimalSceneId,
    TriggerVolumeSceneId
};
```

- [ ] **Step 2: Add the authored minimal scene generator**

In `PhysicsSceneFactory.cs`, add a sibling method near `CreateStaticMeshShowcaseScene()`:

```csharp
/// <summary>
/// Creates the minimal playable static-mesh validation scene.
/// </summary>
/// <returns>Authored minimal static-mesh playable scene asset.</returns>
SceneAsset CreateStaticMeshMinimalScene() {
    SceneEntityAsset[] scenarioChildren = new[] {
        CreatePhysicsBoxMeshEntity(
            "static_mesh_minimal.ground",
            "Ground",
            new float3(0f, -0.5f, 0f),
            new float3(18f, 1f, 18f),
            float4.Identity,
            StaticBodyKindCode,
            false,
            CreatePhysicsDemoMaterialReference(PhysicsDemoGroundMaterialRelativePath)),
        CreatePhysicsSphereMeshEntity(
            "static_mesh_minimal.player",
            "PlayerSphere",
            new float3(0f, 0.75f, 0f),
            float4.Identity,
            DynamicBodyKindCode,
            true,
            CreatePhysicsDemoMaterialReference(PhysicsDemoSphereStackBlueMaterialRelativePath))
    };
    SceneEntityAsset playerSphereEntity = FindRequiredSceneEntityAssetByName(scenarioChildren, "PlayerSphere");
    SceneEntityAsset scenarioEntity = CreateScenarioRoot("static_mesh_minimal.scenario", scenarioChildren);
    SceneEntityAsset cameraEntity = CreateStaticMeshShowcaseCameraEntity(
        "static_mesh_minimal.camera",
        new float3(8f, 5f, 8f),
        CreateYawPitchRollDegrees(-135.0, -18.0, 0.0),
        playerSphereEntity.Id);
    return CreatePhysicsShowcaseSceneAsset(PhysicsSceneCatalog.StaticMeshMinimalSceneId, cameraEntity, scenarioEntity);
}
```

Keep the helper choice intentionally narrow: reuse `CreateStaticMeshShowcaseCameraEntity(...)` instead of inventing a second follow-camera serializer.

- [ ] **Step 3: Wire the new scene into scene creation and playable-scene routing**

Update the existing factory branches:

```csharp
} else if (string.Equals(sceneId, PhysicsSceneCatalog.StaticMeshShowcaseSceneId, StringComparison.Ordinal)) {
    return CreateStaticMeshShowcaseScene();
} else if (string.Equals(sceneId, PhysicsSceneCatalog.StaticMeshMinimalSceneId, StringComparison.Ordinal)) {
    return CreateStaticMeshMinimalScene();
} else if (string.Equals(sceneId, PhysicsSceneCatalog.TriggerVolumeSceneId, StringComparison.Ordinal)) {
    return CreateTriggerVolumeScene();
}
```

Extend the playable-scene predicate:

```csharp
static bool IsPlayablePhysicsShowcaseScene(string sceneId) {
    return string.Equals(sceneId, PhysicsSceneCatalog.DynamicStackBoxesSceneId, StringComparison.Ordinal)
        || string.Equals(sceneId, PhysicsSceneCatalog.SingleFallingCubeSceneId, StringComparison.Ordinal)
        || string.Equals(sceneId, PhysicsSceneCatalog.DynamicSphereStackSceneId, StringComparison.Ordinal)
        || string.Equals(sceneId, PhysicsSceneCatalog.DynamicMixedStackSceneId, StringComparison.Ordinal)
        || string.Equals(sceneId, PhysicsSceneCatalog.StaticMeshShowcaseSceneId, StringComparison.Ordinal)
        || string.Equals(sceneId, PhysicsSceneCatalog.StaticMeshMinimalSceneId, StringComparison.Ordinal);
}
```

Extend the playable live-authoring branch:

```csharp
} else if (string.Equals(normalizedSceneId, PhysicsSceneCatalog.StaticMeshMinimalSceneId, StringComparison.Ordinal)) {
    authoredSceneAsset = CreateStaticMeshMinimalScene();
    SceneEntityAsset staticMeshScenarioRoot = ResolveRequiredPlayablePhysicsShowcaseScenarioRoot(authoredSceneAsset);
    SceneEntityAsset playerSphereEntity = FindRequiredSceneEntityAssetByName(staticMeshScenarioRoot.Children, "PlayerSphere");
    cameraEntity = CreateLiveStaticMeshShowcaseCameraEntity(
        "StaticMeshMinimalCamera",
        new float3(8f, 5f, 8f),
        CreateYawPitchRollDegrees(-135.0, -18.0, 0.0),
        playerSphereEntity.Id);
}
```

Extend short-id normalization:

```csharp
} else if (string.Equals(sceneId, "test_scene_static_mesh_minimal", StringComparison.Ordinal)) {
    return PhysicsSceneCatalog.StaticMeshMinimalSceneId;
}
```

Extend the post-save-id rebinding condition so the new scene uses the same fixed target serialization path:

```csharp
if (string.Equals(normalizedSceneId, PhysicsSceneCatalog.StaticMeshShowcaseSceneId, StringComparison.Ordinal)
    || string.Equals(normalizedSceneId, PhysicsSceneCatalog.StaticMeshMinimalSceneId, StringComparison.Ordinal)) {
    RebindStaticMeshShowcaseCameraTarget(cameraEntity, scenarioRoots);
}
```

- [ ] **Step 4: Update the source test assertions to match the final implementation**

Tighten the new source test in `CityPhysicsSceneSourceTests.cs` so it checks the intended concrete strings:

```csharp
Assert.Contains("CreatePhysicsBoxMeshEntity(", source, StringComparison.Ordinal);
Assert.Contains("\"static_mesh_minimal.ground\", \"Ground\"", source, StringComparison.Ordinal);
Assert.Contains("\"static_mesh_minimal.player\", \"PlayerSphere\"", source, StringComparison.Ordinal);
Assert.Contains("PhysicsSceneCatalog.StaticMeshMinimalSceneId", source, StringComparison.Ordinal);
Assert.Contains("\"test_scene_static_mesh_minimal\"", source, StringComparison.Ordinal);
Assert.Contains("CreateLiveStaticMeshShowcaseCameraEntity(", source, StringComparison.Ordinal);
Assert.Contains("RebindStaticMeshShowcaseCameraTarget(cameraEntity, scenarioRoots);", source, StringComparison.Ordinal);
```

- [ ] **Step 5: Run the source tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityPhysicsSceneSourceTests -v minimal
```

Expected:

```text
PASS
```

If MSBuild file locking occurs, rerun the same command once instead of widening scope to unrelated test projects.

- [ ] **Step 6: Commit the scene implementation**

```bash
git add C:/dev/helprojs/city/assets/codebase/physics.tools/PhysicsSceneCatalog.cs C:/dev/helprojs/city/assets/codebase/physics.tools/PhysicsSceneFactory.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/CityPhysicsSceneSourceTests.cs
git commit -m "feat: add minimal static mesh playable scene"
```

### Task 3: Regenerate Assets and Verify the New Scene in a Windows Direct-Test Build

**Files:**
- Modify temporarily: `C:\dev\helprojs\city\user_settings\build_config.json`
- Generate: `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_minimal.helen`
- Generate: `C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_minimal_ds.helen`
- Verify: `C:\dev\helprojs\city\windows-build-staticmesh\helengine_windows.exe`
- Verify: `C:\dev\helprojs\city\windows-build-staticmesh\helengine_windows.startup.log`
- Verify: `C:\dev\helprojs\city\windows-build-staticmesh\helengine_windows.diagnostics.log`

- [ ] **Step 1: Regenerate the playable physics scenes**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.generate-physics-scenes
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.generate-physics-nintendo-ds-scenes
```

Expected:

```text
Editor command 'menu.generate-physics-scenes' executed successfully.
Editor command 'menu.generate-physics-nintendo-ds-scenes' executed successfully.
```

- [ ] **Step 2: Verify the generated authored scene references the new minimal id path**

Run:

```powershell
rtk rg -n test_scene_static_mesh_minimal C:\dev\helprojs\city\assets\scenes\physics
```

Expected:

```text
C:\dev\helprojs\city\assets\scenes\physics\test_scene_static_mesh_minimal.helen
```

- [ ] **Step 3: Temporarily set the Windows build startup order to the new minimal scene**

Edit `C:\dev\helprojs\city\user_settings\build_config.json` so the Windows platform section starts with:

```json
"selectedSceneIds": [
  "test_scene_static_mesh_minimal",
  "DemoDiscMainMenu",
  "cube_test"
]
```

And:

```json
"sceneOrders": [
  {
    "sceneId": "test_scene_static_mesh_minimal",
    "orderNumber": 1
  },
  {
    "sceneId": "DemoDiscMainMenu",
    "orderNumber": 2
  }
]
```

Only reorder the existing Windows entries. Do not remove the normal menu scene from the build.

- [ ] **Step 4: Build the Windows direct-test output**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --build windows --output C:\dev\helprojs\city\windows-build-staticmesh
```

Expected:

```text
Build completed successfully
```

If the build logs are noisy, capture them to a file and inspect the exit code instead of assuming success.

- [ ] **Step 5: Restore `build_config.json` to normal main-menu-first ordering**

Restore the Windows section to:

```json
{
  "sceneId": "DemoDiscMainMenu",
  "orderNumber": 1
}
```

And move `test_scene_static_mesh_minimal` back behind the menu in both `selectedSceneIds` and `sceneOrders`.

- [ ] **Step 6: Launch the Windows build and verify the runtime logs**

Run:

```powershell
Start-Process -FilePath 'C:\dev\helprojs\city\windows-build-staticmesh\helengine_windows.exe' -WorkingDirectory 'C:\dev\helprojs\city\windows-build-staticmesh'
```

Then check:

```powershell
rtk rg -n "Loading startup scene|Fatal host/engine exception|could not resolve target scene entity id" C:\dev\helprojs\city\windows-build-staticmesh\helengine_windows.startup.log
rtk rg -n "tracked_scene_ids=|fatal_exception" C:\dev\helprojs\city\windows-build-staticmesh\helengine_windows.diagnostics.log
```

Expected:

```text
[Host] Loading startup scene from runtime scene catalog entry 'test_scene_static_mesh_minimal'.
tracked_scene_ids="test_scene_static_mesh_minimal"
```

And no matches for:

```text
Fatal host/engine exception
could not resolve target scene entity id
fatal_exception
```

- [ ] **Step 7: Commit regenerated scene outputs if they changed**

```bash
git add C:/dev/helprojs/city/assets/scenes/physics/test_scene_static_mesh_minimal.helen C:/dev/helprojs/city/assets/scenes/physics/test_scene_static_mesh_minimal_ds.helen
git commit -m "chore: regenerate minimal static mesh scene assets"
```

## Self-Review

### Spec Coverage

- New sibling playable scene: covered in Task 2.
- Reuse of existing `PlayerSphere` and `DemoFollowCameraComponent` path: covered in Task 2.
- Narrow source-level regression coverage: covered in Task 1 and Task 2.
- Scene generation and Windows direct-test verification: covered in Task 3.

### Placeholder Scan

- No `TODO`, `TBD`, or “similar to Task N” placeholders remain.
- Every code-changing step includes concrete snippets or exact branch additions.
- Every verification step includes a concrete command and expected outcome.

### Type Consistency

- New scene id name is consistently `StaticMeshMinimalSceneId`.
- New short id is consistently `test_scene_static_mesh_minimal`.
- The camera path consistently reuses `CreateLiveStaticMeshShowcaseCameraEntity(...)` and `RebindStaticMeshShowcaseCameraTarget(...)`.
