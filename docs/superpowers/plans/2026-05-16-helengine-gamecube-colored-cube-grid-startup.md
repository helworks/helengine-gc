# GameCube Colored Cube Grid Startup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the packaged GameCube build boot directly into `scenes/rendering/colored_cube_grid.helen` from the `city` project while preserving the current stable packaged-disc boot chain and renderer behavior.

**Architecture:** Keep packaged startup-scene selection on the existing builder/runtime-manifest path, not a runtime override. Stage the real colored scene payloads into the packaged-content root, recook its authored materials into the PS2/DS-style `PlatformMaterialAsset` contract already used by GameCube, then update the startup/probe assumptions and verify the packaged `.gcm` boots straight into the colored grid.

**Tech Stack:** C#, xUnit, generated C++ runtime manifest, libogc/GameCube native runtime, Dolphin, RTK commands, packaged-disc retail-check harness.

---

## File Structure

- `C:/dev/helworks/helengine-gc/builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`
  Verifies the generated packaged runtime scene manifest emits the colored-grid startup scene id and cooked scene path.
- `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
  Verifies the packaged-disc workspace copies the colored-grid scene asset and its referenced cooked material payloads into `disc/files/...`.
- `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  Verifies the native packaged runtime source no longer probes `cube_test.hasset` and instead looks for the colored-grid cooked scene path.
- `C:/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
  Holds the packaged FST and asset-read diagnostics that currently still assume `cube_test.hasset`.
- `C:/dev/helworks/helengine-gc/tmp/builder-retail-check/Program.cs`
  Seeds the packaged runtime manifest, recooks staged material payloads, and packages the `.gcm` used by the Dolphin probe loop.
- `C:/dev/helworks/helengine-gc/tools/stage_city_cube_test_content.sh`
  Existing city-content staging script reference; either generalize it or replace it with a colored-grid packaged-content staging script.
- `C:/dev/helworks/helengine-gc/README.md`
  Documents the packaged-scene staging/build commands if the verification flow changes enough to be user-facing.

### Task 1: Lock the colored-grid packaged startup contract in tests

**Files:**
- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`
- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing manifest-writer expectation**

Add a second scene-fixture shape that uses the colored-grid authored scene id and cooked path instead of `cube_test`:

```csharp
const string StartupSceneId = "scenes/rendering/colored_cube_grid.helen";
const string StartupSceneCookedRelativePath = "cooked/scenes/rendering/colored_cube_grid.hasset";

PlatformBuildManifest manifest = new(
    1,
    "project",
    "1.0.0",
    "1.0.0",
    "gamecube",
    "1.0.0",
    StartupSceneId,
    [
        new PlatformBuildScene(
            StartupSceneId,
            "Colored Cubes",
            "Scenes/rendering/colored_cube_grid.helen",
            [],
            [new KeyValuePair<string, string>("cooked-relative-path", StartupSceneCookedRelativePath)])
    ],
    Array.Empty<PlatformBuildAsset>(),
    [new PlatformBuildArtifact(StartupSceneCookedRelativePath, "scene-content-hash", "scene", "gamecube-default")],
    Array.Empty<PlatformBuildCodeModule>(),
    Array.Empty<PlatformArtifactPlacement>(),
    new PlatformContainerWritePlan("gamecube-disc-layout", Array.Empty<PlatformContainerArtifact>()));
```

and assert:

```csharp
Assert.Contains("\"scenes/rendering/colored_cube_grid.helen\"", source, StringComparison.Ordinal);
Assert.Contains("\"cooked/scenes/rendering/colored_cube_grid.hasset\"", source, StringComparison.Ordinal);
```

- [ ] **Step 2: Write the failing packaged-workspace expectation**

Update the packaged workspace fixture to stage the colored-grid scene and at least one cube material path:

```csharp
string sceneSourcePath = Path.Combine(sourceRootPath, "cooked", "scenes", "rendering", "colored_cube_grid.hasset");
string cubeMaterialPath = Path.Combine(sourceRootPath, "cooked", "materials", "rendering", "colored_cube_grid", "Cube00.hasset");
```

and assert:

```csharp
Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "scenes", "rendering", "colored_cube_grid.hasset")));
Assert.True(File.Exists(Path.Combine(outputRootPath, "disc", "files", "cooked", "materials", "rendering", "colored_cube_grid", "Cube00.hasset")));
```

- [ ] **Step 3: Write the failing packaged-runtime source expectation**

Replace the hard-coded probe strings in the source-contract test:

```csharp
Assert.Contains("const char expectedAssetName[] = \"colored_cube_grid.hasset\";", applicationSource, StringComparison.Ordinal);
Assert.Contains("const std::string expectedPath = \"dvd:/cooked/scenes/rendering/colored_cube_grid.hasset\";", applicationSource, StringComparison.Ordinal);
Assert.Contains("foundColoredCubeGrid", applicationSource, StringComparison.Ordinal);
```

- [ ] **Step 4: Run the focused test slice to verify failure**

Run:

```bash
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal
```

Expected: FAIL with remaining `cube_test` assertions and missing `colored_cube_grid` file/path references.

- [ ] **Step 5: Commit the red tests**

```bash
git add builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "test: lock colored grid packaged startup contract"
```

### Task 2: Switch the packaged startup scene and staged payload assumptions

**Files:**
- Modify: `C:/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
- Modify: `C:/dev/helworks/helengine-gc/tmp/builder-retail-check/Program.cs`
- Modify or Create: `C:/dev/helworks/helengine-gc/tools/stage_city_cube_test_content.sh` or `C:/dev/helworks/helengine-gc/tools/stage_city_colored_cube_grid_content.sh`

- [ ] **Step 1: Update the packaged-disc probe path in `GameCubeApplication.cpp`**

Change the packaged FST/asset probe constants from `cube_test` to `colored_cube_grid`:

```cpp
const char expectedAssetName[] = "colored_cube_grid.hasset";
...
SYS_Report(
    "[GC] Packaged FST probe rootEntryCount=%08lX foundColoredCubeGrid=%d\n",
    static_cast<unsigned long>(rootEntryCount),
    foundExpectedAsset ? 1 : 0);
...
const std::string expectedPath = "dvd:/cooked/scenes/rendering/colored_cube_grid.hasset";
```

- [ ] **Step 2: Switch the retail-check manifest seed to the authored colored-grid scene id**

In `tmp/builder-retail-check/Program.cs`, replace the startup-scene constants:

```csharp
const string StartupSceneId = "scenes/rendering/colored_cube_grid.helen";
const string StartupSceneCookedRelativePath = "cooked/scenes/rendering/colored_cube_grid.hasset";
```

and update the manifest scene entry:

```csharp
new PlatformBuildScene(
    StartupSceneId,
    "Colored Cubes",
    "Scenes/rendering/colored_cube_grid.helen",
    [],
    [new KeyValuePair<string, string>("cooked-relative-path", StartupSceneCookedRelativePath)])
```

- [ ] **Step 3: Extend the retail-check harness to recook the colored-grid materials**

Replace the single `standard.hasset` rewrite with one loop over the sixteen authored colored materials:

```csharp
static readonly string[] ColoredCubeGridMaterialColors = {
    "#FF4040FF", "#FF8040FF", "#FFC040FF", "#FFFF40FF",
    "#C0FF40FF", "#80FF40FF", "#40FF40FF", "#40FF80FF",
    "#40FFC0FF", "#40FFFFFF", "#40C0FFFF", "#4080FFFF",
    "#4040FFFF", "#8040FFFF", "#C040FFFF", "#FF40FFFF"
};

for (int cubeIndex = 0; cubeIndex < ColoredCubeGridMaterialColors.Length; cubeIndex++) {
    string materialPath = Path.Combine(
        repositoryRootPath,
        "tmp",
        "city-packaged-gcm-root",
        "files",
        "cooked",
        "materials",
        "rendering",
        "colored_cube_grid",
        "Cube" + cubeIndex.ToString("00") + ".hasset");

    PlatformMaterialCookResult result = new GameCubePlatformAssetBuilder().CookMaterial(new PlatformMaterialCookRequest(
        "materials/rendering/colored_cube_grid/Cube" + cubeIndex.ToString("00"),
        "materials/rendering/colored_cube_grid/Cube" + cubeIndex.ToString("00") + ".helmat",
        "gamecube",
        "gamecube-default",
        "gx",
        GameCubeMaterialSchemaIds.StandardTexturedSchemaId,
        new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
            [GameCubeMaterialSchemaIds.TextureRelativePathFieldId] = string.Empty,
            [GameCubeMaterialSchemaIds.DoubleSidedFieldId] = "false",
            [GameCubeMaterialSchemaIds.VertexColorModeFieldId] = "multiply",
            [GameCubeMaterialSchemaIds.BaseColorFieldId] = ColoredCubeGridMaterialColors[cubeIndex],
            [GameCubeMaterialSchemaIds.LightingModeFieldId] = "lit"
        }));

    Directory.CreateDirectory(Path.GetDirectoryName(materialPath) ?? throw new InvalidOperationException("Material directory path could not be resolved."));
    File.WriteAllBytes(materialPath, result.CookedMaterialBytes);
}
```

- [ ] **Step 4: Stage the colored scene payloads into `tmp/city-packaged-gcm-root/files`**

Either generalize the existing script or add a new one that copies the colored scene plus its cooked materials:

```bash
#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/city/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-packaged-gcm-root/files/cooked"

mkdir -p \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/engine/models" \
    "${STAGE_ROOT}/materials/rendering/colored_cube_grid"

cp "${CITY_COOKED_ROOT}/scenes/rendering/colored_cube_grid.hasset" "${STAGE_ROOT}/scenes/rendering/colored_cube_grid.hasset"
cp "${CITY_COOKED_ROOT}/engine/models/cube.hasset" "${STAGE_ROOT}/engine/models/cube.hasset"
cp "${CITY_COOKED_ROOT}/materials/rendering/colored_cube_grid/"Cube*.hasset "${STAGE_ROOT}/materials/rendering/colored_cube_grid/"
```

- [ ] **Step 5: Re-run the focused source-contract slice to verify it passes**

Run:

```bash
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal
```

Expected: PASS for the colored-grid startup-scene contract and packaged-content assumptions.

- [ ] **Step 6: Commit the startup-scene and staging changes**

```bash
git add src/platform/gamecube/GameCubeApplication.cpp tmp/builder-retail-check/Program.cs tools/stage_city_cube_test_content.sh builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "feat: boot packaged GameCube build into colored cube grid"
```

### Task 3: Verify the packaged colored-grid boot end to end

**Files:**
- Modify: `C:/dev/helworks/helengine-gc/README.md`
- Use: `C:/dev/helworks/helengine-gc/tmp/builder-retail-check/Program.cs`
- Use: `C:/dev/helworks/emus/gc-probe/game.gcm`

- [ ] **Step 1: Document the colored-grid packaged verification flow**

Update the README packaged-disc section from `cube_test` wording to the colored-grid target and the new staging command:

```md
- staged startup scene: `cooked/scenes/rendering/colored_cube_grid.hasset`
- staged material payload root: `cooked/materials/rendering/colored_cube_grid`
- packaged probe target: `scenes/rendering/colored_cube_grid.helen`
```

- [ ] **Step 2: Build the staged colored-grid image**

Run:

```bash
rtk bash tools/stage_city_cube_test_content.sh
rtk dotnet run --project .\\tmp\\builder-retail-check\\builder-retail-check.csproj -- normalize-and-recook-material
rtk dotnet run --project .\\tmp\\builder-retail-check\\builder-retail-check.csproj -- prepare-manifest
rtk dotnet run --project .\\tmp\\builder-retail-check\\builder-retail-check.csproj -- package
```

Expected:

```text
...tmp/generated-core-gamecube/runtime/gamecube_runtime_scene_manifest.hpp
ISO/GC
...tmp/session-2026-05-15-gcm-demo-check/game.gcm
```

- [ ] **Step 3: Copy the packaged image to the stable Dolphin probe path**

Run:

```bash
rtk proxy powershell -NoProfile -Command "Copy-Item 'C:\\dev\\helworks\\helengine-gc\\tmp\\session-2026-05-15-gcm-demo-check\\game.gcm' 'C:\\dev\\helworks\\emus\\gc-probe\\game.gcm' -Force"
```

Expected: no output and the file timestamp under `C:\dev\helworks\emus\gc-probe\game.gcm` updates.

- [ ] **Step 4: Launch Dolphin and verify direct boot into the colored grid**

Run:

```bash
rtk proxy powershell -NoProfile -Command "Start-Process -FilePath 'C:\\dev\\helworks\\emus\\Dolphin-x64\\Dolphin.exe' -ArgumentList '-e','C:\\dev\\helworks\\emus\\gc-probe\\game.gcm'"
```

Expected on screen:
- boot skips the menu and enters the colored cube grid scene directly
- scene stays stable
- visible result is either authored colored cubes or white cubes, but not a scene-load failure

Expected log boundary:

```text
[GC] Packaged startup scene id: scenes/rendering/colored_cube_grid.helen
[GC] Packaged runtime startup scene queued.
[GC] First draw completed.
```

- [ ] **Step 5: Commit the verification/documentation update**

```bash
git add README.md tools/stage_city_cube_test_content.sh tmp/builder-retail-check/Program.cs
git commit -m "docs: capture colored grid packaged verification flow"
```

## Self-Review

- Spec coverage: startup-scene selection, packaged scene availability, and Dolphin boot verification are each covered by their own task.
- Placeholder scan: no `TODO`, `TBD`, or “similar to previous task” shortcuts remain.
- Type consistency: the plan uses one startup scene id contract throughout: `scenes/rendering/colored_cube_grid.helen` with cooked path `cooked/scenes/rendering/colored_cube_grid.hasset`.
