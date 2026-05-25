# GameCube GX-Native Mesh Cache Design

**Date:** 2026-05-25

**Goal:** Replace the current immediate-mode GameCube 3D mesh submission path with a GX-native cached mesh path that becomes the default for all GameCube 3D meshes, without introducing a GameCube-specific cooked mesh asset format in phase 1.

## Problem

The current GameCube 3D renderer is functionally correct for `cube_test` and `colored_cube_grid`, but it is still using a CPU-heavy immediate submission path:

- `GameCubeRasterRenderer::DrawSubmesh(...)` walks authored position, normal, texture-coordinate, and index arrays every frame.
- Geometry is submitted with `GX_Begin(...)` and per-vertex `GX_Position3f32(...)`, `GX_Color4u8(...)`, and `GX_TexCoord2f32(...)`.
- Lit materials evaluate directional and ambient lighting on the CPU per submitted vertex.

This is acceptable for early bring-up, but it keeps the CPU front-end cost too high and does not reflect how the GameCube backend should scale for general 3D content.

## Scope

Phase 1 covers all GameCube 3D meshes rendered through the current `GameCubeRasterRenderer` path:

- packaged cooked meshes loaded through `BuildModelFromCooked(...)`
- authored runtime models constructed in code, including `cube_test`
- unlit, textured, and lit 3D mesh submissions

Phase 1 does not introduce:

- a GameCube-specific cooked mesh asset format
- a new cross-platform model asset contract
- a full lighting redesign that pushes lighting computation into GX fixed-function light channels

## Design Summary

The existing cross-platform cooked `ModelAsset` remains the source asset format. The GameCube backend will convert that data once into GameCube-owned cached draw data when the mesh is first resolved in `GameCubeMeshCache`.

The cached draw data becomes the default render path for all GameCube 3D meshes. The renderer will stop traversing the authored arrays during every draw and instead bind cached GameCube-native geometry buffers and draw submesh index ranges from that cached representation.

## Recommended Approach

### 1. Keep `ModelAsset` as the source format

`ModelAsset` remains the platform-neutral cooked mesh format. This avoids unnecessary pipeline churn and allows the GameCube backend to evolve without forcing a content rebuild contract change across the engine.

### 2. Convert once into GameCube-owned cached mesh data

When `GameCubeMeshCache::Resolve(...)` encounters a `GameCubeRuntimeModel` for the first time, it will validate the authored arrays and build one cached GameCube representation.

That cached representation should include:

- tightly-owned position data in the layout expected by the GameCube renderer
- optional normal data when the source mesh contains normals
- optional texture-coordinate data when the source mesh contains texture coordinates
- per-submesh index range ownership
- index storage normalized for the GameCube path

The first pass should prefer normalizing to 16-bit index storage where the source data allows it. If a model cannot be represented safely with 16-bit indices, the cache build should fail loudly with a specific exception instead of silently falling back to a slow path.

### 3. Make the cached representation the default path for all GameCube 3D meshes

`GameCubeRasterRenderer::DrawSubmesh(...)` should use cached GameCube geometry by default for every GameCube runtime model. The current immediate array-walk path should stop being the normal render path.

This means:

- submeshes still use their authored material assignments
- submesh `IndexStart` and `IndexCount` still define draw boundaries
- the mesh cache becomes responsible for preparing the data shape expected by the renderer

### 4. Treat lighting as a separate concern from geometry caching

Geometry submission and lighting should be decoupled.

For phase 1:

- unlit meshes should use fully cached geometry data
- textured unlit meshes should use fully cached geometry data
- lit meshes should share the same cached topology and vertex-source ownership
- if the current lighting model still requires per-entity CPU-side color evaluation, only the color data should remain dynamic in pass 1

The important architectural point is that lit rendering should not force the engine back into full immediate-mode geometry submission. If temporary dynamic color upload is still needed, that should be isolated to color handling, not positions, indices, and overall draw topology.

## Data Ownership

`GameCubeRuntimeModel` should evolve from “authored arrays attached to a runtime model” into “authoring source plus cached GameCube draw data owned by the runtime model.”

The runtime model should own:

- the deserialized source `ModelAsset` when created from cooked content
- one cached GameCube mesh representation built from that source
- any cached submesh metadata needed by the renderer

The authoring arrays can remain accessible during the transition if needed, but the design target is that the renderer no longer depends on those arrays during the steady-state draw path.

## Renderer Behavior

The renderer should continue to own:

- projection upload
- model-view matrix upload
- material branch selection
- GX pipeline state selection
- texture binding
- cull mode selection
- z mode selection

The renderer should stop owning:

- per-frame traversal of authored mesh arrays as the default 3D path
- per-frame reconstruction of full triangle submission streams for unlit and textured meshes

## Why Not A Custom GameCube Mesh Format Yet

A GameCube-specific cooked mesh asset format is a valid later optimization, but it is not required for the first meaningful performance step.

Reasons to defer it:

- the highest current cost is in the runtime submission path, not in the cross-platform asset contract
- a backend-local runtime conversion is much lower risk than an asset-pipeline redesign
- this keeps phase 1 focused on renderer throughput, not content pipeline expansion

A GameCube-specific cooked format can be revisited later if profiling shows that load cost or memory layout remains a meaningful bottleneck after the cached draw path lands.

## Validation Strategy

Validation should focus on both correctness and performance shape.

Correctness checks:

- `cube_test` still boots and renders correctly
- `colored_cube_grid` still renders with correct depth ordering
- `textured_cube_grid` still renders textured geometry correctly
- no regressions in packaged cooked model loading

Performance checks:

- compare Dolphin render timing before and after the cached path
- confirm the render thread no longer pays the full per-frame authored-array traversal cost
- inspect whether lit meshes still dominate due to dynamic color handling

## Risks

Primary risks:

- incorrect ownership or lifetime around cached mesh data
- mismatched index normalization or submesh range mapping
- accidentally creating separate paths for authored and cooked models that drift apart
- leaving lit meshes on a materially different slow path that undermines the goal of one default renderer shape

Mitigation:

- keep one default cached-geometry path for all GameCube runtime models
- isolate any unavoidable phase-1 dynamic work to color data only
- fail loudly on unsupported mesh shapes instead of silently falling back

## Future Follow-Up

After phase 1 lands, the next likely follow-up is to profile lit meshes and decide whether to:

- keep CPU-side per-vertex lighting with smaller dynamic color handling
- move more lighting responsibility into GX fixed-function channels
- introduce a GameCube-specific cooked mesh format for faster load and tighter memory layout

## Approval Record

Approved design direction:

- use cached GX-native geometry as the default path for all GameCube 3D meshes
- do not introduce a custom GameCube cooked mesh format in phase 1
