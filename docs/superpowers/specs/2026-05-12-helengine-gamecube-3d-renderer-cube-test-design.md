# GameCube 3D Renderer Cube Test Design

## Goal

Design the first real GameCube 3D renderer slice in `helengine-gc` so the platform can render the existing checked-in `city` scene `assets/scenes/rendering/cube_test.helen` through the generated-core scene/runtime path.

The design must aim at a state-of-the-art renderer model while allowing later backends to reduce precision or feature coverage explicitly.

## Current Baseline

The repository already proves the pre-render bring-up milestones:

- `helengine-gc` boots in Dolphin
- generated core is included in the GameCube build
- `Core::Initialize(...)`, `Core::Update()`, and `Core::Draw()` survive frame-to-frame
- `GameCubeRenderManager3D` is still a placeholder bridge that returns a dummy `RuntimeModel`

The reusable scene source already exists outside this repo in:

- `/mnt/c/dev/helprojs/city/assets/scenes/rendering/cube_test.helen`

That scene should be treated as the canonical first rendering input for GameCube rather than inventing a GameCube-only scene path.

## Constraints

- The first spec is local to `helengine-gc`, not engine-wide.
- The first visible scene must load through the generated-core runtime path, not a host-owned bypass.
- The first source scene is `cube_test.helen` from `city`.
- The first visual milestone proves geometry-space correctness, not renderer parity.
- Fallback behavior must be explicit and capability-driven, not hidden behind silent approximation.
- The design must not hard-code a low-end-only GameCube renderer model into the core shape.

## Chosen Approach

Design one canonical high-precision renderer contract conceptually, but implement only a narrow GameCube execution slice first.

That means:

1. keep the render model more expressive than the first GameCube backend implementation
2. extract scene and frame state before reduction
3. reduce unsupported features through explicit capability decisions
4. implement only the minimum path required to render `cube_test.helen` correctly on GameCube

This is preferable to both extremes:

- a full state-of-the-art renderer architecture first, which is too broad for the next executable milestone
- an ad hoc GX-first renderer, which would make later precision tiers and higher-level renderer evolution harder

## Repository Scope

This spec is intentionally owned by `helengine-gc` for now.

It covers:

- local GameCube rendering architecture
- the first GameCube backend execution path
- the first GameCube scene-render verification milestone

It does not require immediate upstream renderer ownership or engine-wide contract adoption. If the design proves out, those contracts can be upstreamed later from a working GameCube implementation instead of from speculation.

## Canonical Renderer Model

The renderer should be designed around one canonical render model that is more precise than the first backend tier.

That canonical model should own:

- camera state
- world/view/projection transforms
- mesh submissions
- material intent
- frame ownership
- pass ordering

The important rule is that GameCube should consume that model through explicit reduction rather than redefine the engine around GX-era limitations.

For the first milestone, the canonical model may describe more than GameCube executes. That is acceptable. The GameCube backend is allowed to decline unsupported features only through an explicit capability surface.

## Local Component Design

The GameCube renderer should not collapse into one monolithic `GameCubeRenderManager3D`.

Instead, the local responsibilities should be split into four focused components.

### `GameCubeSceneRenderBridge`

Owns conversion from generated runtime scene state into a GameCube-local render input.

Responsibilities:

- inspect runtime camera and mesh state
- resolve scene-visible render inputs from generated-core objects
- avoid direct GX calls
- preserve high-precision frame data before backend reduction

### `GameCubeFramePlan`

Owns the reduced execution payload for one GameCube frame.

For the first milestone it only needs:

- one resolved camera
- visible opaque mesh submissions
- resolved world/view/projection matrices
- depth state
- reduced material intent sufficient for the first draw path

This is backend-local, but it must be derived from the canonical render model rather than from scene-specific shortcuts.

### `GameCubeMeshCache`

Owns conversion of runtime model data into GameCube-ready geometry records.

Responsibilities:

- resolve runtime model references into concrete backend geometry
- cache reusable mesh data between frames
- isolate asset/runtime-model concerns from per-frame draw execution

### `GameCubeRasterRenderer`

Owns GX setup and draw execution.

Responsibilities:

- configure viewport, projection, culling, and depth state
- bind cached geometry
- issue draw calls from the frame plan
- remain ignorant of scene loading and runtime-object traversal

### `GameCubeRenderManager3D`

Should remain the generated-core seam, but only as an orchestrator.

It should:

- delegate runtime extraction to `GameCubeSceneRenderBridge`
- delegate geometry lifetime to `GameCubeMeshCache`
- delegate draw execution to `GameCubeRasterRenderer`

This preserves the existing generated-core integration point while keeping the internals evolvable.

## Data Flow

The first render path should be one-way and explicit:

1. generated runtime loads `cube_test.helen`
2. `GameCubeSceneRenderBridge` resolves the active camera and visible mesh components
3. the bridge emits canonical frame data with full-precision transforms and camera values
4. a capability filter reduces that data into a `GameCubeFramePlan`
5. `GameCubeMeshCache` resolves the cube model into backend geometry
6. `GameCubeRasterRenderer` configures GX and draws the frame

The fallback boundary is after canonical frame extraction, not before it.

That rule matters because later higher-precision rendering features should be reducible per backend without corrupting authoring, runtime scene loading, or frame extraction.

## First Backend Tier

The first GameCube execution tier should be deliberately narrow.

Supported in the first tier:

- one camera
- opaque mesh submissions
- transform propagation
- projection and viewport setup
- depth-enabled 3D rasterization
- unlit material intent sufficient to render visible geometry

Explicitly out of scope for the first tier:

- lighting correctness
- material parity
- shadows
- post-processing
- batching sophistication
- advanced multi-pass behavior

If `cube_test.helen` requests unsupported visible 3D behavior in the first path, the frame-plan build should fail loudly rather than silently degrade.

## First Scene Milestone

The first visible rendering milestone should use:

- source scene: `/mnt/c/dev/helprojs/city/assets/scenes/rendering/cube_test.helen`

The milestone proves only:

- scene loading is real
- camera resolution is real
- mesh/model resolution is real
- transform propagation is real
- projection is real
- rasterization is real
- depth handling is real

It does not yet prove:

- runtime rotation
- lighting
- material fidelity
- multiple-object state handling

The cube model must come from the authored runtime/model reference path. A hard-coded host primitive is not acceptable for success.

The camera must come from the scene path as authored runtime data. A hard-coded camera is not acceptable for success.

## Error Handling

Failure boundaries should remain strict and observable:

- if `cube_test.helen` cannot load through the generated runtime path, fail before rendering
- if no active camera can be resolved, fail in frame-plan construction
- if the cube model reference cannot resolve into runtime model data, fail in mesh-cache population
- if unsupported visible features enter the first backend tier, fail during capability reduction

The renderer should not invent fallback scene content, fallback camera state, or fallback mesh geometry when required authored values are missing.

## Verification

The first render milestone is complete only when all of the following are true:

1. the GameCube build emits `build/helengine_gc.dol`
2. Dolphin boots the build through the generated-core runtime path
3. `cube_test.helen` is the actual rendered content source
4. the authored scene camera drives the visible framing
5. the authored cube model renders through the real model/runtime path
6. the cube appears through a depth-enabled 3D draw path

This verification is intentionally geometry-first. Visual richness is not part of this milestone.

## Follow-On Order

After the first `cube_test` render milestone, the next slices should stay on the same architecture:

1. add runtime rotation proof through the generated update path
2. add material intent binding verification
3. add lighting behavior
4. add broader precision and feature tiers later

This order keeps geometry-space correctness isolated before more visually complex systems are layered on top.

## Non-Goals

- engine-level renderer ownership in this first spec
- renderer parity with Windows
- lighting correctness in the first milestone
- material/shader precision parity
- batching or optimization passes
- multi-scene renderer coverage
- startup-menu integration

## Exit Criteria

This design is successful when implementation based on it can produce a GameCube-local renderer path that:

- remains architecturally higher precision than its first backend tier
- uses explicit capability reduction rather than hidden approximation
- renders `cube_test.helen` through the real generated-core runtime path
- proves camera, mesh, transform, projection, and depth correctness before richer rendering work begins
