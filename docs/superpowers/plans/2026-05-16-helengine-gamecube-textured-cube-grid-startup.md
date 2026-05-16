# GameCube Textured Cube Grid Startup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the packaged GameCube build boot directly into `scenes/rendering/textured_cube_grid.helen` and render the authored textured cube grid through the existing GPU-driven GX path.

**Architecture:** Keep the current packaged-disc boot chain, generated-core scene loading, shared lighting path, and cooked `PlatformMaterialAsset` contract intact. Only switch the startup scene, explicitly stage the textured-grid cooked dependencies from `city`, and extend the GameCube runtime material plus GX raster path so authored diffuse textures are bound and multiplied by the current lit result.

**Tech Stack:** C#, .NET 9, xUnit, C++17, libogc GX, Docker GameCube toolchain, Dolphin, RTK.

---

## File Structure

- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedBuildWorkspaceTests.cs`
  - keep the packaged builder harness locked to the new textured-grid startup scene and artifact list.
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`
  - guard the staged textured-scene contract and the new textured material runtime path.
- Create: `C:\dev\helworks\helengine-gc\tools\stage_city_textured_cube_grid_content.sh`
  - stage the cooked scene, model, font, textured materials, and imported texture artifacts from `C:\dev\helprojs\output\windows\cooked`.
- Modify: `C:\dev\helworks\helengine-gc\tmp\builder-retail-check\Program.cs`
  - switch startup ids and manifest entries from colored-grid to textured-grid, and enumerate the staged textured material and imported texture assets.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeMaterial.hpp`
  - add texture-relative-path ownership on top of the existing base-color runtime contract.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeMaterial.cpp`
  - initialize the authored texture path to an empty string and expose setters/getters.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp`
  - copy cooked texture-relative-path data from `PlatformMaterialAsset` into the GameCube runtime material.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.hpp`
  - declare the first textured-branch helpers and texture-owner expectations.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`
  - load the authored diffuse texture, bind it to GX, and multiply sampled texels by authored base color plus current lighting.
- Modify: `C:\dev\helworks\helengine-gc\Makefile`
  - ensure any new GameCube runtime-material or texture-source units remain part of the packaged-disc native build.

### Task 1: Lock The Textured-Grid Startup Contract In Tests

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedBuildWorkspaceTests.cs`
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`
- Test: `C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj`

- [ ] **Step 1: Write the failing startup-scene and artifact assertions**

```csharp
[Fact]
public void PackagedWorkspaceBuild_UsesTexturedCubeGridStartupScene() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string programSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "builder-retail-check", "Program.cs"));

    Assert.Contains("const string StartupSceneId = \"scenes/rendering/textured_cube_grid.helen\";", programSource, StringComparison.Ordinal);
    Assert.Contains("const string StartupSceneCookedRelativePath = \"cooked/scenes/rendering/textured_cube_grid.hasset\";", programSource, StringComparison.Ordinal);
    Assert.Contains("new PlatformBuildArtifact(\"cooked/materials/rendering/textured_cube_grid/Cube00.hasset\"", programSource, StringComparison.Ordinal);
    Assert.Contains("new PlatformBuildArtifact(\"cooked/imported/0143f809c636f1cd20053a4c3cfc191b7fb5e510eae696a567970db4d0abdf7e\"", programSource, StringComparison.Ordinal);
    Assert.Contains("new PlatformBuildArtifact(\"cooked/imported/0d59b3f08ab55a3b70958d52b2f95d18eb6cf6616d7c1196fbfac091228283e0\"", programSource, StringComparison.Ordinal);
}
```

```csharp
[Fact]
public void PackagedDiscBootSource_StagesTexturedGridSceneAndTextures() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string runtimeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder.tests", "GameCubePackagedRuntimeSourceTests.cs"));
    string stagingScriptSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tools", "stage_city_textured_cube_grid_content.sh"));

    Assert.Contains("textured_cube_grid.hasset", runtimeSource, StringComparison.Ordinal);
    Assert.Contains("cooked/materials/rendering/textured_cube_grid/Cube15.hasset", runtimeSource, StringComparison.Ordinal);
    Assert.Contains("0143f809c636f1cd20053a4c3cfc191b7fb5e510eae696a567970db4d0abdf7e", stagingScriptSource, StringComparison.Ordinal);
    Assert.Contains("0d59b3f08ab55a3b70958d52b2f95d18eb6cf6616d7c1196fbfac091228283e0", stagingScriptSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused tests to verify they fail**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal
```

Expected:

```text
FAIL with missing "textured_cube_grid.helen" or missing imported texture artifact assertions
```

- [ ] **Step 3: Update the test files minimally**

```csharp
Assert.Contains("const string StartupSceneId = \"scenes/rendering/textured_cube_grid.helen\";", programSource, StringComparison.Ordinal);
Assert.Contains("const string StartupSceneCookedRelativePath = \"cooked/scenes/rendering/textured_cube_grid.hasset\";", programSource, StringComparison.Ordinal);
Assert.Contains("new PlatformBuildArtifact(\"cooked/imported/0143f809c636f1cd20053a4c3cfc191b7fb5e510eae696a567970db4d0abdf7e\"", programSource, StringComparison.Ordinal);
Assert.Contains("new PlatformBuildArtifact(\"cooked/imported/0d59b3f08ab55a3b70958d52b2f95d18eb6cf6616d7c1196fbfac091228283e0\"", programSource, StringComparison.Ordinal);
```

```csharp
Assert.Contains("const char expectedAssetName[] = \"textured_cube_grid.hasset\";", applicationSource, StringComparison.Ordinal);
Assert.Contains("const std::string expectedPath = \"dvd:/cooked/scenes/rendering/textured_cube_grid.hasset\";", applicationSource, StringComparison.Ordinal);
```

- [ ] **Step 4: Run the focused tests to verify they pass**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal
```

Expected:

```text
PASS for the textured-grid startup contract slices
```

- [ ] **Step 5: Commit the test contract checkpoint**

```bash
git add builder.tests/GameCubePackagedBuildWorkspaceTests.cs builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "test: lock GameCube textured grid startup contract"
```

### Task 2: Stage The Real Textured-Grid Cooked Content

**Files:**
- Create: `C:\dev\helworks\helengine-gc\tools\stage_city_textured_cube_grid_content.sh`
- Modify: `C:\dev\helworks\helengine-gc\tmp\builder-retail-check\Program.cs`
- Test: `C:\dev\helworks\helengine-gc\tools\stage_city_textured_cube_grid_content.sh`

- [ ] **Step 1: Write the failing staging script and manifest expectations**

```bash
#!/usr/bin/env bash
set -euo pipefail

CITY_COOKED_ROOT="/mnt/c/dev/helprojs/output/windows/cooked"
STAGE_ROOT="/mnt/c/dev/helworks/helengine-gc/tmp/city-packaged-gcm-root/files/cooked"

mkdir -p \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/models" \
    "${STAGE_ROOT}/materials/rendering/textured_cube_grid" \
    "${STAGE_ROOT}/fonts" \
    "${STAGE_ROOT}/imported"

cp "${CITY_COOKED_ROOT}/scenes/rendering/textured_cube_grid.hasset" "${STAGE_ROOT}/scenes/rendering/textured_cube_grid.hasset"
cp "${CITY_COOKED_ROOT}/models/cube.hasset" "${STAGE_ROOT}/models/cube.hasset"
cp "${CITY_COOKED_ROOT}/fonts/default.hefont" "${STAGE_ROOT}/fonts/default.hefont"
cp "${CITY_COOKED_ROOT}/materials/rendering/textured_cube_grid/"Cube*.hasset "${STAGE_ROOT}/materials/rendering/textured_cube_grid/"
cp "${CITY_COOKED_ROOT}/imported/0143f809c636f1cd20053a4c3cfc191b7fb5e510eae696a567970db4d0abdf7e" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/d3a8cdec6238c63ed57434abed80877ab5d5aabbad851a76e18dfc261d855485" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/4c5eb874277e3374705e558af7a9b4603a0e224da4dc92d0815a2a2ebdb06bb4" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/fc9ee8227ea629e6974e3f32924d7e845d8d84bb03c2ebd8e117f1fec186af8d" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/753096d32f70cfc9a1769144e683d19f973e0f24a206e55ea5bb5b6484456ba3" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/a137289998782a48e50bef2c5755c70b420b69ac7f4d31ccad94d08353321e80" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/077764b9996a65108df578775fd4312dfc8de9d9397ae8cf896066ad08981626" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/5a9ed77790c0bc12f53b2f053c946538907d4d79c23bf5888cba00ea445ad486" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/3158011fcbbff95cea13b5fd8390a7b3692248405a02f9b79189f57a9751d48f" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/1ec0f27f988f0c5392b4e1210f3bb3a14061482b2d945dfebc8c87ec24ebda19" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/dd88638e125560144ef71c1618652192b2292e162eb124b23064ac39fb276b71" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/adfd390752d23c78fd066541ef827db2bc5fc09938dba36a5f7f8b85ddd963e9" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/9098e36e79bfce6249e5ce23fb995174c8cebb98520cc6817f311f02f6764d56" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/fd6cd7cde4625b6a48ab2e4ed43842b290c720656f831f572cc2a3b5392deed2" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/3728cc4e9f8b82e5fff2693cb2400ccb51ee9e98f7ec8477b76d5223fb1d94e8" "${STAGE_ROOT}/imported/"
cp "${CITY_COOKED_ROOT}/imported/0d59b3f08ab55a3b70958d52b2f95d18eb6cf6616d7c1196fbfac091228283e0" "${STAGE_ROOT}/imported/"
```

```csharp
const string StartupSceneId = "scenes/rendering/textured_cube_grid.helen";
const string StartupSceneCookedRelativePath = "cooked/scenes/rendering/textured_cube_grid.hasset";
```

- [ ] **Step 2: Run staging to verify any missing paths fail before codegen/build**

Run:

```powershell
rtk bash tools/stage_city_textured_cube_grid_content.sh
```

Expected:

```text
Either successful completion or a concrete missing cooked path under C:\dev\helprojs\output\windows\cooked
```

- [ ] **Step 3: Implement the manifest and startup-scene switch**

```csharp
new PlatformBuildArtifact("cooked/scenes/rendering/textured_cube_grid.hasset", "scene-content-hash", "scene", "gamecube-default"),
new PlatformBuildArtifact("cooked/materials/rendering/textured_cube_grid/Cube00.hasset", "material-content-hash", "material", "gamecube-default"),
new PlatformBuildArtifact("cooked/materials/rendering/textured_cube_grid/Cube15.hasset", "material-content-hash", "material", "gamecube-default"),
new PlatformBuildArtifact("cooked/imported/0143f809c636f1cd20053a4c3cfc191b7fb5e510eae696a567970db4d0abdf7e", "texture-content-hash", "texture", "gamecube-default"),
new PlatformBuildArtifact("cooked/imported/0d59b3f08ab55a3b70958d52b2f95d18eb6cf6616d7c1196fbfac091228283e0", "texture-content-hash", "texture", "gamecube-default")
```

```csharp
BuildSceneBridgeProbe(
    "Scenes/rendering/textured_cube_grid.helen",
    "cooked/scenes/rendering/textured_cube_grid.hasset");
```

- [ ] **Step 4: Run staging and packaging prep to verify the new startup scene is live**

Run:

```powershell
rtk bash tools/stage_city_textured_cube_grid_content.sh
rtk dotnet run --project .\tmp\builder-retail-check\builder-retail-check.csproj -- normalize-and-recook-material
rtk dotnet run --project .\tmp\builder-retail-check\builder-retail-check.csproj -- prepare-manifest
```

Expected:

```text
Successful completion and regenerated tmp/generated-core-gamecube/runtime/gamecube_runtime_scene_manifest.inl containing textured_cube_grid
```

- [ ] **Step 5: Commit the staging and startup-scene checkpoint**

```bash
git add tools/stage_city_textured_cube_grid_content.sh tmp/builder-retail-check/Program.cs
git commit -m "feat: stage GameCube textured cube grid startup content"
```

### Task 3: Preserve Cooked Texture State On The GameCube Runtime Material

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeMaterial.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeMaterial.cpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp`
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-contract assertion for cooked texture propagation**

```csharp
Assert.Contains("std::string GetTextureRelativePath() const;", runtimeMaterialHeaderSource, StringComparison.Ordinal);
Assert.Contains("void SetTextureRelativePath(std::string value);", runtimeMaterialHeaderSource, StringComparison.Ordinal);
Assert.Contains("runtimeMaterial->SetTextureRelativePath(materialAsset->TextureRelativePath);", renderManagerSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused source-contract test and verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal
```

Expected:

```text
FAIL with missing GameCubeRuntimeMaterial texture-path assertions
```

- [ ] **Step 3: Implement the minimal runtime-material texture ownership**

```cpp
class GameCubeRuntimeMaterial final : public RuntimeMaterial {
public:
    GameCubeRuntimeMaterial();
    float3 GetBaseColor() const;
    void SetBaseColor(float3 value);
    const std::string& GetTextureRelativePath() const;
    void SetTextureRelativePath(std::string value);

private:
    float3 BaseColorValue;
    std::string TextureRelativePathValue;
};
```

```cpp
runtimeMaterial->SetBaseColor(float3(
    static_cast<float>(materialAsset->BaseColorR) / 255.0f,
    static_cast<float>(materialAsset->BaseColorG) / 255.0f,
    static_cast<float>(materialAsset->BaseColorB) / 255.0f));
runtimeMaterial->SetTextureRelativePath(materialAsset->TextureRelativePath);
```

- [ ] **Step 4: Run the source-contract slice again**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal
```

Expected:

```text
PASS with the GameCube runtime material carrying cooked texture-relative-path state
```

- [ ] **Step 5: Commit the runtime-material texture seam**

```bash
git add src/platform/gamecube/GameCubeRuntimeMaterial.hpp src/platform/gamecube/GameCubeRuntimeMaterial.cpp src/platform/gamecube/GameCubeRenderManager3D.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
git commit -m "feat: preserve GameCube cooked texture paths"
```

### Task 4: Bind Authored Diffuse Textures In The GX Lit Path

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`
- Modify: `C:\dev\helworks\helengine-gc\Makefile`
- Test: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing textured-branch source assertions**

```csharp
Assert.Contains("GX_InitTexObj(", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("GX_LoadTexObj(", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("material->GetTextureRelativePath()", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("GX_SetNumTexGens(1);", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);", rasterRendererSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused source-contract test and verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal
```

Expected:

```text
FAIL with missing GX texture binding or textured TEV assertions
```

- [ ] **Step 3: Implement the first textured lit branch**

```cpp
const std::string& textureRelativePath = gameCubeRuntimeMaterial->GetTextureRelativePath();
if (!textureRelativePath.empty()) {
    RuntimeTexture* runtimeTexture = material->ResolveTexture();
    if (runtimeTexture == nullptr) {
        throw new InvalidOperationException("GameCube textured material requires a resolved runtime texture.");
    }

    GX_SetNumTexGens(1);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_InitTexObj(&textureObject, texturePixels, textureWidth, textureHeight, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_LoadTexObj(&textureObject, GX_TEXMAP0);
} else {
    GX_SetNumTexGens(0);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}
```

```cpp
GXColor litColor = EvaluateLitVertexColor(framePlan, entity, gameCubeRuntimeMaterial, (*runtimeModel->Normals)[positionIndex]);
GX_Color4u8(litColor.r, litColor.g, litColor.b, litColor.a);
```

- [ ] **Step 4: Run the source-contract tests and the full native packaged build**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal
rtk docker run --rm -v C:/dev/helworks/helengine-gc:/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube -e HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc -e HELENGINE_GAMECUBE_MINIMAL_SAMPLE=0 helengine-gc sh -lc 'make clean all packaged-disc-assets'
```

Expected:

```text
PASS for the source-contract slice and successful native GameCube packaged-disc build
```

- [ ] **Step 5: Package, launch, and commit the textured-grid checkpoint**

Run:

```powershell
rtk dotnet run --project .\tmp\builder-retail-check\builder-retail-check.csproj -- package
Copy-Item 'C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm' 'C:\dev\helworks\emus\gc-probe\game.gcm' -Force
Start-Process -FilePath 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe' -ArgumentList '-e','C:\dev\helworks\emus\gc-probe\game.gcm'
```

Expected:

```text
The packaged GameCube build boots directly into textured_cube_grid and the cubes render with authored textures instead of the old colored-grid scene
```

```bash
git add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp Makefile
git commit -m "feat: render GameCube textured cube grid"
```
