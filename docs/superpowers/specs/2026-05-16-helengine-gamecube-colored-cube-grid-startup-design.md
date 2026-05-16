# GameCube Colored Cube Grid Startup Design

## Goal

Make the packaged GameCube build boot directly into the authored `city` scene `scenes/rendering/colored_cube_grid.helen` instead of `cube-test`, while preserving the currently working packaged-disc boot chain, shared lighting path, corrected GX cull mapping, and stable rotating-cube rendering.

This change is intentionally narrow. The first objective is to prove that the GameCube packaged runtime can load and render the real colored cube grid scene end to end. If the scene boots but the cubes still render white, that becomes a separate material-color problem rather than a scene/bootstrap problem.

## Scope

In scope:

- Switch the packaged startup scene selection from the current single-cube scene to `scenes/rendering/colored_cube_grid.helen`.
- Ensure the packaged scene catalog and generated runtime manifest include the colored cube grid scene in the packaged-disc build flow.
- Preserve the current GameCube shared lighting branch:
  - extracted scene lights
  - authored mesh normals
  - GX vertex-color lighting
  - corrected GX cull reversal
  - no fake minimum light floor
- Verify the packaged `.gcm` boots straight into the colored cube grid in Dolphin.

Out of scope:

- New lighting models
- New render passes
- Shader-style runtime changes
- Menu-driven scene selection
- Fixes for authored material base color, unless the startup-scene switch proves that is the next boundary

## Current Baseline

The current packaged GameCube path is stable for `cube-test`:

- retail `.gcm` boots correctly
- packaged scene/material/model loading works
- shared ambient + directional white lighting works on GPU vertex colors
- GX culling is mapped correctly from shared `MaterialCullMode`
- the old artificial light floor has been removed

So the startup-scene swap must not regress:

- boot reliability
- packaged content resolution
- scene activation
- stable rendering

## Desired Behavior

After the change:

1. the packaged GameCube build boots directly into `scenes/rendering/colored_cube_grid.helen`
2. the colored cube grid scene loads without using the menu
3. Dolphin shows the colored cube grid scene running from the packaged `.gcm`
4. if authored material colors already flow through the current GC material path, cubes should appear colored under the current shared lighting
5. if cubes still appear white, the startup-scene switch is still considered successful, and the next task becomes GC lit material base-color support

## Design

### 1. Startup Scene Selection

The packaged startup-scene source of truth must move from the current `cube-test` selection to the `city` authored scene id `scenes/rendering/colored_cube_grid.helen`.

The GameCube build flow should not special-case this at runtime with ad hoc overrides. The builder/runtime manifest path should select the colored cube grid the same way other packaged scenes are selected today, so the result stays aligned with the generated packaged scene catalog contract.

### 2. Packaged Scene Availability

The packaged-disc asset flow must include the colored cube grid scene asset and its referenced authored material/model assets. This should happen through the existing packaged manifest/staged disc flow, not through one-off manual file injection.

If the scene already exists in the generated/staged packaged content, the work is mainly in startup selection. If it does not, the builder-side packaged scene input must be updated so the colored cube grid is staged and indexed as part of the packaged content root.

### 3. Renderer Stability

The current renderer path stays unchanged for this step:

- no change to the white ambient + directional lighting implementation
- no change to GX projection/world/view handling
- no change to the cull mapping fix

That isolates the verification:

- successful colored grid boot means the scene/bootstrap path is good
- white cubes after boot mean the next problem is material base color

### 4. Failure Interpretation

The result of this step should be interpreted narrowly:

- Boot failure: packaged startup-scene/catalog regression
- Scene load failure: packaged content/reference regression
- Scene renders but cubes are white: material base-color support is the next targeted task
- Scene renders with authored colors: startup-scene switch is complete and we can move on to broader lighting/material polish

## Testing Strategy

### Source-Contract Coverage

Add or update focused GameCube packaged-runtime source tests so the repo guards:

- the packaged startup-scene selection points at `scenes/rendering/colored_cube_grid.helen`
- the packaged scene manifest/runtime source no longer assumes `cube-test` as the startup scene

### End-to-End Verification

Run the established GC verification path:

1. focused builder/runtime source-contract tests
2. native packaged-disc Docker build
3. package `.gcm`
4. copy to the stable Dolphin probe path
5. launch Dolphin

Success criteria for this step:

- `.gcm` boots directly into the colored cube grid scene
- scene remains stable in Dolphin
- visible result is either:
  - authored colored cubes, or
  - white cubes with stable scene boot, which cleanly identifies the next material-color task

## Risks

### Scene Packaging Drift

The `city` authored scene may rely on assets or references not currently included in the GC packaged content flow. If so, the startup-scene switch may expose missing staged assets rather than a renderer problem.

### Material Color Boundary

The current GC lit branch intentionally forces white after lighting accumulation. That means the colored cube grid may still boot successfully while rendering white cubes. That is acceptable for this step because it cleanly proves startup-scene correctness.

### Catalog/Manifest Assumptions

Some existing GC tests or builder assumptions may still be pinned to `cube-test`. Those need to be updated as part of the startup-scene switch rather than worked around at runtime.

## Completion Criteria

This design is complete when:

- the packaged GameCube build boots directly into `scenes/rendering/colored_cube_grid.helen`
- the boot is stable in Dolphin
- the result clearly establishes the next boundary:
  - authored colored cubes already work, or
  - authored scene boot works and material base color is the next targeted task
