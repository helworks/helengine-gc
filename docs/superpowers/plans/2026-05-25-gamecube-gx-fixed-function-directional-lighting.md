# GameCube GX Fixed-Function Directional Lighting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the remaining CPU-side per-vertex lighting work in the GameCube lit 3D path with GX fixed-function ambient plus one directional light, while keeping the cached indexed mesh path as the default geometry path.

**Architecture:** Extend `GameCubeCachedMeshData` so lit meshes own packed indexed normals alongside positions, texture coordinates, and indices. Refactor `GameCubeRasterRenderer` so lit meshes bind those cached normal arrays and configure GX light/channel state from extracted render-frame light submissions instead of calling `EvaluateLitVertexColor(...)` during vertex submission.

**Tech Stack:** C++20, libogc GX, devkitPPC, existing GameCube builder source-shape tests, headless editor CLI export path, Dolphin manual smoke validation.

---

## File Structure

**Existing files and responsibilities**

- `src/platform/gamecube/GameCubeCachedMeshData.hpp`
  Owns the cached GX-native mesh payload. This file should add packed normal storage and metadata for the lit indexed path.
- `src/platform/gamecube/GameCubeMeshCache.cpp`
  Builds cached mesh payloads. This file should pack and flush normal arrays during cache construction.
- `src/platform/gamecube/GameCubeRasterRenderer.hpp`
  Declares the narrow GX raster path. This file should expose focused helpers for lit GX state and directional-light setup.
- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  Executes the 3D draw path. This file should stop evaluating lit vertex colors on the CPU and should move lit draws onto GX color-channel lighting.
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  Source-shape guardrail file. Extend it to watch for packed cached normals and the removal of CPU-lit per-vertex submission from the steady-state lit path.

**No new runtime files are required**

Keep the implementation inside the current mesh-cache and raster-renderer boundaries. Do not introduce a parallel lighting subsystem.

## Task 1: Add packed cached normals to the GameCube mesh payload

**Files:**
- Modify: `src/platform/gamecube/GameCubeCachedMeshData.hpp`
- Modify: `src/platform/gamecube/GameCubeMeshCache.cpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions that the cached mesh payload now owns a packed normal type and that the mesh cache populates it:

```csharp
string cachedMeshDataSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeCachedMeshData.hpp"));
string meshCacheSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeMeshCache.cpp"));

Assert.Contains("struct GameCubePackedNormal3", cachedMeshDataSource, StringComparison.Ordinal);
Assert.Contains("Array<GameCubePackedNormal3>* PackedNormals;", cachedMeshDataSource, StringComparison.Ordinal);
Assert.Contains("cachedMeshData->PackedNormals", meshCacheSource, StringComparison.Ordinal);
Assert.Contains("DCFlushRange(&(*cachedMeshData->PackedNormals)[0]", meshCacheSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because the packed normal type and cache-build path do not exist yet. If the test project still fails first due to the known unrelated texture-format compile break, record that and continue with source inspection plus a clean builder compile for verification.

- [ ] **Step 3: Implement packed normal ownership and cache build**

Update `GameCubeCachedMeshData.hpp` so normals use a POD layout instead of `Array<float3>*`:

```cpp
struct GameCubePackedNormal3 {
    float X;
    float Y;
    float Z;
};

Array<GameCubePackedNormal3>* PackedNormals;
```

Update `GameCubeMeshCache.cpp` so the cache builder:

```cpp
cachedMeshData->PackedNormals = gcnew Array<GameCubePackedNormal3>(sourceNormals->get_Length());
for (int32_t normalIndex = 0; normalIndex < sourceNormals->get_Length(); normalIndex++) {
    float3 normal = (*sourceNormals)[normalIndex];
    (*cachedMeshData->PackedNormals)[normalIndex] = GameCubePackedNormal3 { normal.X, normal.Y, normal.Z };
}

DCFlushRange(&(*cachedMeshData->PackedNormals)[0], sizeof(GameCubePackedNormal3) * cachedMeshData->PackedNormals->Length);
cachedMeshData->HasNormals = true;
```

Keep the existing strict failure behavior for lit meshes that do not provide normals.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the new assertions, or the same known unrelated test-project failures with these new assertions no longer failing first.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeCachedMeshData.hpp src/platform/gamecube/GameCubeMeshCache.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Pack GameCube cached normal arrays"
```

## Task 2: Add GX lit-pipeline helpers for ambient plus one directional light

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions that the raster renderer declares GX-lighting helpers and lit indexed normal binding:

```csharp
string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

Assert.Contains("void ConfigureLitPipeline(bool useTexturedBranch, bool useIndexedGeometry);", rasterRendererHeaderSource, StringComparison.Ordinal);
Assert.Contains("void ConfigureDirectionalLight(", rasterRendererHeaderSource, StringComparison.Ordinal);
Assert.Contains("GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("GX_SetChanCtrl(GX_COLOR0A0", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("GX_InitLightDir(", rasterRendererSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because the dedicated lit-pipeline helpers and GX normal binding do not exist yet.

- [ ] **Step 3: Implement focused lit GX-state helpers**

Split the current lit setup out of `ConfigurePipeline(...)` into a dedicated helper path:

```cpp
void ConfigureLitPipeline(bool useTexturedBranch, bool useIndexedGeometry);
void ConfigureDirectionalLight(GameCubeFramePlan* framePlan, GXLightObj& lightObject, GXColor& ambientColor, bool& hasDirectionalLight);
```

Implement the lit pipeline so it:

```cpp
GX_ClearVtxDesc();
GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
GX_SetVtxDesc(GX_VA_TEX0, useTexturedBranch ? GX_INDEX16 : GX_NONE);
GX_SetNumChans(1);
GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
```

The directional-light helper should:

```cpp
GX_InitLightDir(&lightObject, lightDirection.X, lightDirection.Y, lightDirection.Z);
GX_InitLightColor(&lightObject, lightColor);
GX_LoadLightObjImm(&lightObject, GX_LIGHT0);
GX_SetChanAmbColor(GX_COLOR0A0, ambientColor);
GX_SetChanMatColor(GX_COLOR0A0, GXColor { 0xFF, 0xFF, 0xFF, 0xFF });
```

Keep the selection policy narrow: first directional light only, ambient accumulation as before, ambient-only when no directional light exists.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the new helper assertions, or unchanged unrelated failures only.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Add GameCube GX lit pipeline helpers"
```

## Task 3: Move lit cached-submesh draws from CPU colors to GX lighting

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions that the lit cached-submesh path now binds packed normals and no longer relies on `EvaluateLitVertexColor(...)` inside the steady-state draw loop:

```csharp
Assert.Contains("GX_SetArray(GX_VA_NRM", rasterRendererSource, StringComparison.Ordinal);
Assert.Contains("DrawCachedLitSubmesh(", rasterRendererSource, StringComparison.Ordinal);
Assert.DoesNotContain("GX_Color4u8(litColor.r, litColor.g, litColor.b, litColor.a);", rasterRendererSource, StringComparison.Ordinal);
Assert.DoesNotContain("EvaluateLitVertexColor(framePlan, entity, material, normal)", rasterRendererSource, StringComparison.Ordinal);
```

If the exact old `GX_Color4u8(...)` line has already changed shape, target the surrounding CPU-lit loop text that still proves per-vertex lighting work is present.

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because `DrawCachedLitSubmesh(...)` still evaluates and submits per-vertex CPU colors.

- [ ] **Step 3: Refactor the lit draw path onto GX lighting**

Update `BindCachedMeshArrays(...)` so lit textured draws can bind all three arrays:

```cpp
GX_SetArray(GX_VA_POS, &(*cachedMeshData->PackedPositions)[0], sizeof(GameCubePackedPosition3));
GX_SetArray(GX_VA_NRM, &(*cachedMeshData->PackedNormals)[0], sizeof(GameCubePackedNormal3));
GX_SetArray(GX_VA_TEX0, &(*cachedMeshData->PackedTexCoords)[0], sizeof(GameCubePackedTexCoord2));
```

Refactor `DrawCachedLitSubmesh(...)` so it:

```cpp
ConfigureLitPipeline(useTexturedBranch, true);
BindCachedMeshArrays(cachedMeshData, useTexturedBranch);
ConfigureDirectionalLight(framePlan, lightObject, ambientColor, hasDirectionalLight);
GX_Begin(GX_TRIANGLES, GX_VTXFMT0, runtimeSubmesh->get_IndexCount());
for (int32_t drawIndex = indexStart; drawIndex < indexEnd; drawIndex++) {
    uint16_t vertexIndex = (*cachedMeshData->Indices16)[drawIndex];
    GX_Position1x16(vertexIndex);
    GX_Normal1x16(vertexIndex);
    if (useTexturedBranch) {
        GX_TexCoord1x16(vertexIndex);
    }
}
GX_End();
```

Delete the steady-state CPU-lit per-vertex color path:

```cpp
GXColor EvaluateLitVertexColor(...);
float3 AccumulateAmbientAndDirectionalLight(...);
GXColor ConvertLightingColorToGx(...);
```

Only remove those helpers once nothing in the runtime path calls them anymore. Keep the unlit cached path and all 2D rendering behavior unchanged.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the lit-path assertions, or unchanged unrelated failures only.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeRasterRenderer.hpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Use GX fixed-function lighting for GameCube lit meshes"
```

## Task 4: Verify the GX-lit path end to end on packaged scenes

**Files:**
- Verify: `src/platform/gamecube/GameCubeCachedMeshData.hpp`
- Verify: `src/platform/gamecube/GameCubeMeshCache.cpp`
- Verify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Verify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Verify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Run the smallest clean compile for the GameCube builder**

Run: `rtk dotnet build builder/helengine.gamecube.builder.csproj`

Expected: PASS with exit code `0`.

- [ ] **Step 2: Rebuild the GameCube exports through the headless editor DLL path**

Run:

```powershell
dotnet C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll --project C:\dev\helprojs\city\project.heproj --build gamecube --output C:\dev\helprojs\output\gamecube-colored-cubes-smoke --common-output
dotnet C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll --project C:\dev\helprojs\city\project.heproj --build gamecube --output C:\dev\helprojs\output\gamecube-textured-cubes-smoke --common-output
dotnet C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll --project C:\dev\helprojs\city\project.heproj --build gamecube --output C:\dev\helprojs\output\gamecube-cube-test-smoke --common-output
```

Expected for each run:

- native build completes
- disc layout completes
- `.gcm` packaging completes

- [ ] **Step 3: Verify the packaged artifacts exist**

Run:

```powershell
Get-Item C:\dev\helprojs\output\gamecube-colored-cubes-smoke\game.gcm
Get-Item C:\dev\helprojs\output\gamecube-textured-cubes-smoke\game.gcm
Get-Item C:\dev\helprojs\output\gamecube-cube-test-smoke\game.gcm
```

Expected: all three files exist with fresh timestamps.

- [ ] **Step 4: Launch Dolphin and smoke-test the lit scenes**

Manual verification targets:

- `cube_test` still renders the white lit cube correctly
- `colored_cube_grid` still renders correctly with depth ordering and scene lighting
- `textured_cube_grid` still renders correctly with textured lit cubes
- lit-scene render time drops from the current 5-6 ms baseline, even if it does not yet reach a final target

Capture visible regressions precisely:

- black or flat cubes
- inverted directional light
- missing textures on the lit path
- normals bound incorrectly

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeCachedMeshData.hpp src/platform/gamecube/GameCubeMeshCache.cpp src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Verify GameCube GX fixed-function lighting end to end"
```

## Self-Review Notes

- Spec coverage: this plan covers packed indexed normals, GX ambient plus one directional light, lit-path migration, and packaged-scene verification.
- Placeholder scan: no `TODO`, `TBD`, or deferred “implement later” steps remain.
- Type consistency: the plan uses `GameCubePackedNormal3`, `PackedNormals`, `ConfigureLitPipeline(...)`, and `ConfigureDirectionalLight(...)` consistently across tasks.

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-05-25-gamecube-gx-fixed-function-directional-lighting.md`.

Two execution options:

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

Which approach?
