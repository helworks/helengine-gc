# GameCube GX-Native Mesh Cache Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current immediate `GX_Begin(...)` mesh submission path with a cached GX-native mesh path that becomes the default for all GameCube 3D meshes.

**Architecture:** Keep the existing cross-platform cooked `ModelAsset` format and build one GameCube-owned cached mesh representation per `GameCubeRuntimeModel` inside `GameCubeMeshCache`. Update `GameCubeRasterRenderer` to draw submeshes from that cached representation instead of traversing authored arrays every frame, while preserving current material, texture, and depth behavior. Treat dynamic lit color handling as a separate layer on top of cached geometry rather than a reason to keep full immediate-mode geometry submission.

**Tech Stack:** C++20, libogc GX, devkitPPC, existing GameCube builder source-shape tests, headless editor CLI export path, Dolphin manual smoke validation.

---

## File Structure

**Existing files and responsibilities**

- `src/platform/gamecube/GameCubeRuntimeModel.hpp`
  Stores the GameCube runtime model state. This file should become the owner of cached GameCube draw data instead of only authoring arrays.
- `src/platform/gamecube/GameCubeMeshCache.hpp`
  Declares the GameCube backend mesh cache. This file should expose the cache/build entry points for cached mesh data.
- `src/platform/gamecube/GameCubeMeshCache.cpp`
  Validates and resolves runtime models. This file should build and attach the cached mesh representation once per model.
- `src/platform/gamecube/GameCubeRasterRenderer.hpp`
  Declares the GameCube 3D renderer. This file should define the renderer helpers needed to bind and draw cached mesh data.
- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  Executes the 3D draw path. This file should stop traversing authoring arrays every frame and should draw from cached mesh data instead.
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  Existing source-shape test file for packaged runtime behavior. Extend this file to guard the new cached draw path contract where practical.

**New files to create**

- `src/platform/gamecube/GameCubeCachedMeshData.hpp`
  Own the cached GX-native mesh representation and related lightweight structs so the runtime model and renderer do not keep accumulating unrelated responsibilities in one header.

## Task 1: Define cached GameCube mesh ownership

**Files:**
- Create: `src/platform/gamecube/GameCubeCachedMeshData.hpp`
- Modify: `src/platform/gamecube/GameCubeRuntimeModel.hpp`
- Modify: `src/platform/gamecube/GameCubeMeshCache.hpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions in `builder.tests/GameCubePackagedRuntimeSourceTests.cs` that look for the new cached mesh type name and its use from the runtime model and mesh cache. Keep the assertions narrow and textual so they fail before implementation.

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because the cached mesh type and ownership hooks do not exist yet. If the full test project still fails due to unrelated pre-existing issues, capture that and proceed with the smallest reliable verification available for the modified assertions.

- [ ] **Step 3: Add the cached mesh data types and ownership fields**

Implement `GameCubeCachedMeshData.hpp` with focused structs/classes for:

- cached positions
- optional cached normals
- optional cached texture coordinates
- normalized index storage
- submesh index-range metadata

Update `GameCubeRuntimeModel.hpp` to own one cached mesh instance and document the ownership clearly. Update `GameCubeMeshCache.hpp` with any new private helpers needed for building cached data.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the new assertions, or the same known unrelated failures with the new assertions no longer failing first.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeCachedMeshData.hpp src/platform/gamecube/GameCubeRuntimeModel.hpp src/platform/gamecube/GameCubeMeshCache.hpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Define GameCube cached mesh ownership"
```

## Task 2: Build cached mesh data in `GameCubeMeshCache`

**Files:**
- Modify: `src/platform/gamecube/GameCubeMeshCache.cpp`
- Modify: `src/platform/gamecube/GameCubeMeshCache.hpp`
- Modify: `src/platform/gamecube/GameCubeRuntimeModel.hpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions that the mesh cache now builds cached mesh data, validates unsupported mesh shapes explicitly, and attaches the cached representation to the resolved runtime model.

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because the cache build logic is not implemented yet.

- [ ] **Step 3: Implement cached mesh build helpers**

In `GameCubeMeshCache.cpp`, add focused helpers that:

- validate positions and index availability
- normalize index storage for the GameCube path
- fail loudly when the mesh shape is unsupported for the cached path
- build the cached mesh representation only once per runtime model
- keep ownership on `GameCubeRuntimeModel`

Do not introduce a silent fallback to the old immediate path.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the new cache-build assertions, or unchanged unrelated test-project failures only.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeMeshCache.cpp src/platform/gamecube/GameCubeMeshCache.hpp src/platform/gamecube/GameCubeRuntimeModel.hpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Build cached GameCube mesh data in mesh cache"
```

## Task 3: Switch unlit and textured 3D rendering to cached geometry

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeMeshCache.cpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions that `DrawSubmesh(...)` now uses cached mesh data rather than walking `Positions`, `Indices16`, `Indices32`, and `TexCoords` directly in the unlit/textured path.

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because the renderer still references the authoring arrays directly.

- [ ] **Step 3: Implement the cached unlit/textured draw path**

Refactor `GameCubeRasterRenderer.cpp` so the default unlit/textured draw path:

- resolves cached mesh data from `GameCubeRuntimeModel`
- binds cached geometry instead of traversing authoring arrays
- preserves current material selection, texture binding, cull handling, projection, and depth behavior
- removes the old immediate per-vertex authoring-array walk for unlit/textured rendering

Keep the helpers in `GameCubeRasterRenderer.hpp` focused. If a helper becomes too large, split it instead of growing `DrawSubmesh(...)` further.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the renderer-path assertions, or unchanged unrelated project failures only.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeMeshCache.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Render GameCube unlit meshes from cached geometry"
```

## Task 4: Move lit meshes onto the cached geometry path

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRuntimeModel.hpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-shape test**

Add assertions that the lit path now uses cached geometry ownership too, and that any remaining dynamic work is isolated to color handling rather than full immediate geometry traversal.

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: FAIL because the lit path still computes and submits geometry vertex-by-vertex from authoring arrays.

- [ ] **Step 3: Implement the cached lit draw path**

Refactor the lit branch so:

- cached geometry remains the source of topology and base vertex data
- any phase-1 dynamic CPU lighting work is restricted to color handling
- the renderer does not fall back to rebuilding the full geometry stream from authoring arrays for lit meshes

Keep failure behavior strict. If a lit mesh is missing normals, continue failing loudly.

- [ ] **Step 4: Re-run the targeted test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter GameCubePackagedRuntimeSourceTests`

Expected: PASS for the lit-path assertions, or unchanged unrelated project failures only.

- [ ] **Step 5: Commit**

```bash
rtk git add src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRuntimeModel.hpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Render GameCube lit meshes from cached geometry"
```

## Task 5: Verify packaged exports and renderer behavior

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Verify: `src/platform/gamecube/GameCubeMeshCache.cpp`
- Verify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Verify: `src/platform/gamecube/GameCubeRuntimeModel.hpp`

- [ ] **Step 1: Run the smallest clean compile for the GameCube builder**

Run: `rtk dotnet build builder/helengine.gamecube.builder.csproj`

Expected: PASS with exit code `0`.

- [ ] **Step 2: Rebuild the GameCube export through the headless editor DLL path**

Run:

```powershell
dotnet C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll --project C:\dev\helprojs\city\project.heproj --build gamecube --output C:\dev\helprojs\output\gamecube-colored-cubes-smoke --common-output
```

Expected:

- `native build completed`
- `disc image packaged`
- `packaged outputs verified`

- [ ] **Step 3: Verify the packaged artifact exists**

Run:

```powershell
Get-Item C:\dev\helprojs\output\gamecube-colored-cubes-smoke\game.gcm
```

Expected: the ISO exists with a fresh timestamp.

- [ ] **Step 4: Launch Dolphin and smoke-test the three scenes**

Manual verification targets:

- `colored_cube_grid` still renders correctly with depth ordering
- `textured_cube_grid` still renders correctly once selected as startup scene for a dedicated run
- `cube_test` still renders correctly when selected as startup scene for a dedicated run

Capture whether Dolphin’s reported render time drops relative to the prior 5-6 ms range for `colored_cube_grid`.

- [ ] **Step 5: Commit**

```bash
rtk git add builder.tests/GameCubePackagedRuntimeSourceTests.cs src/platform/gamecube/GameCubeMeshCache.cpp src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeRuntimeModel.hpp
rtk git commit -m "Verify cached GameCube mesh rendering end to end"
```

## Notes

- The `builder.tests` project has had unrelated pre-existing failures in this repository. Do not hide or “fix” those unless they block this exact work directly. Record when a targeted assertion is valid but the project still fails for unrelated reasons.
- Do not introduce a GameCube-specific cooked mesh asset in this plan.
- Do not keep a silent fallback to the immediate authoring-array walk. Unsupported mesh shapes should fail loudly.
- Preserve the current depth, cull, projection, and packaged-runtime behavior that was already validated before this performance work started.
