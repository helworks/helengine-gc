# GameCube GX Fixed-Function Directional Lighting Design

**Date:** 2026-05-25

**Goal:** Reduce the remaining CPU cost of the GameCube lit 3D path by replacing CPU-side per-vertex lighting evaluation with GX fixed-function lighting for the simple shared-engine case of ambient plus one directional light, while keeping the new cached indexed mesh path as the default geometry path.

## Problem

The current GameCube 3D renderer now uses cached GX-native geometry for all meshes, which removed the worst immediate-mode geometry overhead. The remaining hot path for lit meshes is still CPU-heavy:

- `GameCubeRasterRenderer` evaluates lit vertex colors on the CPU.
- lit draws still touch every submitted vertex to compute ambient plus directional contribution
- the geometry submission path is improved, but the lighting branch still spends CPU time that GX can handle directly

This keeps `colored_cube_grid` and `cube_test` correct, but it leaves a meaningful amount of CPU work in the steady-state draw path.

## Scope

This phase covers the narrow lighting shape already exercised by the current packaged GameCube scenes:

- ambient contribution from the extracted render frame
- one directional light from the extracted render frame
- lit opaque 3D meshes rendered through the cached indexed GameCube mesh path
- normals supplied to GX through cached indexed normal arrays

This phase explicitly excludes:

- multiple directional lights
- point lights
- spot lights
- specular lighting
- shadows
- a new cooked asset format
- changes to the unlit or 2D paths

## Design Summary

The existing cached indexed mesh path remains the default geometry path. Lit opaque meshes stop computing per-vertex colors on the CPU and instead bind indexed normal data and configure GX color channel lighting for one ambient term plus one directional light.

The renderer remains responsible for translating extracted shared-engine light data into GX state. The scene extraction boundary does not change. The cooked model and material contracts do not change.

## Recommended Approach

### 1. Keep one cached mesh path for all 3D geometry

The current cached mesh design is correct and stays in place:

- positions remain cached in GameCube-owned packed arrays
- texture coordinates remain cached in GameCube-owned packed arrays
- indices remain cached in GameCube-owned normalized 16-bit arrays

The lighting optimization must not split the renderer back into separate geometry paths for lit and unlit meshes.

### 2. Extend cached mesh ownership to include indexed normals

The lit GX path needs stable normal input. `GameCubeCachedMeshData` should therefore own a packed normal array suitable for `GX_SetArray(...)` and `GX_VA_NRM`.

Requirements:

- normal ownership must live beside the cached position and texture-coordinate arrays
- normal packing must use POD storage instead of generated engine math types
- normal data must be cache-flushed once after build, just like the other cached GX arrays
- a lit mesh without normals must fail loudly instead of silently falling back to fake lighting

### 3. Use GX color channel lighting for the first lit branch

For lit opaque meshes, the renderer should configure GX to compute the lit vertex color instead of writing a CPU-evaluated color per vertex.

That means:

- bind normals as indexed GX vertex input
- enable the color channel lighting path for lit draws
- load one `GXLightObj` for the active directional light
- configure ambient color and material color so GX evaluates the final lit vertex color

The first GX lighting branch should remain intentionally narrow:

- ambient term
- one directional diffuse term
- no specular
- no attenuation model beyond what is required for a directional light

### 4. Treat light selection as a renderer policy, not a scene-contract change

The extracted render frame already carries the light submissions. This feature should not change extraction.

Renderer policy for this phase:

- use the first directional light submission in the extracted frame
- accumulate ambient contribution from the extracted frame as before
- if no directional light exists, ambient-only lighting is valid
- if no ambient light exists, zero ambient is valid

This keeps the implementation deterministic and small while matching the current scene expectations.

### 5. Preserve current material and texture behavior

This phase is only about moving lighting work from CPU to GX.

It must preserve:

- current material lighting-model decisions
- current texture binding behavior for textured lit materials
- current z, cull, alpha, and blend behavior for opaque meshes
- current unlit branch behavior

The only intended visual difference is that the lit result now comes from GX instead of the CPU path. The goal is equivalent scene output with lower CPU cost.

## Data Ownership

`GameCubeRuntimeModel` continues to own the cached GameCube mesh representation. That cached representation now includes:

- packed positions
- packed normals when the source mesh has normals
- packed texture coordinates when the source mesh has texture coordinates
- normalized 16-bit indices
- cached submesh metadata

The renderer must consume only that cached representation for the steady-state 3D path.

## Renderer Behavior

For unlit meshes:

- keep the current cached indexed path
- keep vertex color submission behavior unchanged

For lit meshes:

- bind cached positions
- bind cached normals
- bind cached texture coordinates when needed
- configure the GX color channel for one directional light plus ambient
- stop calling the CPU-side lit-color evaluation path for every vertex

The renderer remains responsible for:

- projection upload
- model-view matrix upload
- GX light object setup from extracted frame data
- texture binding
- material branch selection
- opaque raster state selection

## Failure Behavior

This phase should fail loudly on unsupported runtime data instead of introducing fallback behavior that hides bugs.

The renderer should throw for:

- lit meshes that do not provide cached normals
- cached lit meshes whose normal array length does not match the position array length
- a lit material path that reaches GX lighting setup without a valid cached mesh payload

The renderer should not throw for:

- frames with no directional light submissions
- frames with only ambient light

Those are valid scene states and should render with the available light inputs.

## Files And Responsibilities

- `src/platform/gamecube/GameCubeCachedMeshData.hpp`
  - add packed normal storage and cached normal presence metadata
- `src/platform/gamecube/GameCubeMeshCache.cpp`
  - build packed normal arrays from source mesh normals
  - validate lit-mesh normal availability
  - flush packed normal arrays after cache build
- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  - bind cached normal arrays
  - configure lit GX vertex descriptors and color channel state
  - replace CPU per-vertex lighting submission with GX fixed-function light setup
- `src/platform/gamecube/GameCubeRasterRenderer.hpp`
  - declare any new GX-lighting helpers needed by the renderer
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  - extend source-shape assertions for cached normal ownership and lit indexed draw configuration if feasible within the existing test-project constraints

## Validation Strategy

Validation should stay concrete and scene-driven:

1. rebuild the GameCube builder
2. rebuild `cube_test`
3. rebuild `colored_cube_grid`
4. rebuild `textured_cube_grid`
5. launch in Dolphin
6. confirm all three scenes still render correctly
7. compare lit-scene render time against the current CPU-lit baseline

Success means:

- `cube_test` still shows the white lit cube
- `colored_cube_grid` still shows correct colored lit cubes with correct depth ordering
- `textured_cube_grid` still shows textured cubes with the expected lit result
- Dolphin render time for the lit scenes drops meaningfully from the current 5-6 ms range, even if it does not yet reach a final optimized target

## Risks

Primary risks:

- using the wrong light direction sign when mapping the extracted directional light into `GXLightObj`
- incorrect GX channel configuration leading to black, inverted, or flat lighting
- mismatched normal packing or array binding causing visible corruption
- preserving textured lit behavior while changing only the lighting path

Mitigation:

- keep the scope to one directional light plus ambient
- keep one geometry path and change only the lighting branch
- validate on the known-good packaged scenes instead of synthetic probes

## Future Follow-Up

If this phase lands cleanly, the next likely follow-ups are:

- support more than one directional light
- add point-light and spot-light policies
- decide whether additional material terms belong in GX or should stay CPU-side
- revisit whether a GameCube-specific cooked mesh format is needed after the lighting CPU cost is removed

## Approval Record

Approved design direction:

- use GX fixed-function lighting for the simple GameCube shared-engine case
- start with ambient plus one directional light
- accept a larger renderer change if it removes the current CPU-side lit-vertex work
