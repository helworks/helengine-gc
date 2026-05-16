# Helengine GameCube White Shared Lighting Design

## Goal

Advance the working packaged GameCube rotating-cube checkpoint from flat unshaded rendering to the first shared-engine lighting checkpoint. The cube should continue to render through the real scene, real mesh, real cooked material pipeline, and real extracted light data, but the visible material result should be intentionally forced to white for now so lighting can be validated independently of albedo and texture work.

Success means the packaged `.gcm` still boots and shows the rotating cube, but the cube is now visibly lit by the scene's extracted ambient and directional lights instead of appearing as a flat constant color.

## Scope

This checkpoint includes:

- using the existing shared `RuntimeMaterialLightingModel` contract to decide whether a submission is lit or unlit
- consuming light data from the extracted render frame rather than ad hoc scene lookups
- implementing the first GameCube lit branch as ambient plus directional diffuse only
- forcing lit material albedo to white for this checkpoint
- preserving GPU-driven transform, clipping, and rasterization through GX

This checkpoint explicitly excludes:

- authored albedo or texture sampling
- specular lighting
- shadows
- point or spot light support in the first pass
- any GC-only lighting model fork

## Design

### Material Ownership

`GameCubeRenderManager3D` remains responsible for turning cooked platform material assets into `RuntimeMaterial` instances. The GameCube path must continue to use the shared runtime lighting-model flag rather than introducing a temporary GC-only material mode.

The runtime contract for this step is:

- `Unlit` materials continue to use the current flat path
- lit materials use the first shared lighting branch on GameCube

The renderer may ignore authored base color for lit materials during this checkpoint, but it must not break or replace the existing material ownership model.

### Lighting Ownership

The GameCube renderer must consume lights from the extracted render frame. It must not query arbitrary global scene state during raster submission. This keeps the GameCube path aligned with the engine's render extraction boundary and makes later light feature additions predictable.

For each drawable submission:

1. inspect the submission material lighting model
2. if unlit, keep the existing constant-color path
3. if lit, gather ambient and directional light contribution from the extracted frame
4. evaluate ambient plus directional diffuse against the mesh normal
5. shade a forced white base color with that lighting result
6. submit the geometry through GX

### GPU Boundary

The renderer must remain GPU-driven in the important sense:

- GX still performs transform, clipping, and rasterization
- the CPU may prepare light inputs and color data for the draw
- the CPU must not become responsible for software-transforming the full mesh or software-rasterizing pixels as part of this lighting feature

This keeps the lighting checkpoint within the normal renderer setup path rather than turning it into a software rendering detour.

### First Lighting Model

The first GameCube lit branch is intentionally narrow:

- ambient contribution from the extracted frame
- directional diffuse contribution from the extracted frame
- forced white base color
- no specular
- no shadow attenuation

The directional term should use the real mesh normal data if the current runtime model exposes it cleanly and stably. If normals are not available or are invalid in the current runtime model, implementation must stop and report that as the next real blocker instead of faking normals.

### Failure Behavior

The cube must not disappear simply because the extracted light set is empty or incomplete. The visible fallback should remain debuggable:

- if directional light data is missing, ambient-only is acceptable
- if ambient is also missing, a small visible floor value is acceptable to preserve visibility while diagnosing the light pipeline

This fallback is only for absence of extracted light inputs. It is not permission to silently fake normals or bypass the shared lighting-model contract.

## Files And Responsibilities

- `src/platform/gamecube/GameCubeRenderManager3D.cpp`
  - preserve shared runtime material lighting-model mapping
  - avoid introducing a GC-only lighting mode
- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  - implement the first lit draw branch
  - consume extracted frame light data
  - force white shaded output for lit materials
  - keep GX as the transform/raster path
- generated runtime and extracted frame types
  - used as inputs only if they already expose the needed light and normal data cleanly

## Verification

Validation for this checkpoint should stay small and concrete:

1. regenerate any generated core needed by the active GameCube pipeline
2. rebuild the packaged-disc native GameCube target
3. repackage the `.gcm`
4. launch in Dolphin
5. confirm the rotating cube still appears and is white with visible ambient plus directional lighting instead of flat constant color

The key runtime regression bar is that the prior stable packaged rotating-cube checkpoint must remain stable. This feature must not reintroduce disappearing geometry, matrix instability, or packaged content regressions.
