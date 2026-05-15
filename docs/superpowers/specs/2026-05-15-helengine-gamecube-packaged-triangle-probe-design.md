# Helengine GameCube Packaged Triangle Probe Design

## Goal

Add one narrow debug step for the packaged GameCube runtime: draw a hardcoded GX triangle while keeping the current retail `.gcm` boot path, disc mounting, generated-core scene activation, and frame extraction alive.

Success criteria:
- The packaged `.gcm` still boots and stops cleanly in Dolphin.
- The runtime still loads the packaged startup scene and reaches first update and first draw.
- The final visible frame shows one hardcoded triangle instead of only the fallback clear color.
- The change is isolated enough that we can remove it cleanly after proving raster state.

## Scope

This step is intentionally smaller than "make the cube render."

Included:
- Packaged-disc boot path
- Generated-core scene startup
- Existing frame extraction and camera selection
- A renderer-local triangle probe inside the GameCube raster renderer

Excluded:
- Scene asset changes
- Mesh asset changes
- Material/shader support
- Broad renderer refactors
- New unit tests

## Recommended Approach

Use a renderer-local triangle probe in `GameCubeRasterRenderer`.

Design:
- Keep `GameCubeApplication`, scene loading, and `GameCubeSceneRenderBridge` behavior intact.
- Keep frame extraction alive so we know the runtime scene is still producing one drawable submission.
- In `GameCubeRasterRenderer::DrawFrame`, replace authored mesh submission with one hardcoded GX triangle after the normal viewport, projection, and view setup.
- Keep the triangle in clip-space-friendly camera space so we are only proving GX submission, matrix direction, and final framebuffer ownership.

Why this approach:
- It isolates raster output from mesh/index/material uncertainty.
- It keeps the packaged runtime path active, so success still proves `.gcm` boot plus scene startup plus draw ownership.
- It gives a clean next branch:
  - triangle works -> reintroduce authored mesh submission in small steps
  - triangle fails -> stay inside GX state/projection/view/presentation only

## Component Changes

### `GameCubeRasterRenderer`

This file owns the entire probe.

Planned change:
- Add one temporary hardcoded triangle draw path that emits exactly three vertices with direct color.
- Do not depend on runtime mesh data for this step.
- Leave existing diagnostics that report frame extraction/raster boundaries in place unless they block readability.

Expected behavior:
- If the triangle appears, the current packaged runtime has a valid end-to-end draw path and the remaining failure is in authored mesh submission or transform interpretation.
- If the triangle still does not appear, the failure remains inside GX state, matrices, or presentation.

### `GameCubeApplication`

No functional behavior changes are planned here for this step.

Reason:
- Boot, mount, scene activation, update, and draw ownership are already confirmed.
- Changing application flow would widen the search space again.

### Generated Core

No generated-core behavior changes are planned for this step.

Reason:
- Generated-core deserialization and scene activation are already past the failing boundary.

## Data Flow

The active packaged runtime flow remains:

1. Retail `.gcm` boots through the clean-room retail apploader.
2. `GameCubeApplication` mounts DVD and initializes the engine.
3. Packaged scene asset loads and activates.
4. `GameCubeSceneRenderBridge` extracts a frame and hands one submission to the renderer.
5. `GameCubeRasterRenderer` ignores authored submesh drawing for this probe and emits one hardcoded triangle instead.
6. `PresentFrame` displays the result without post-render debug color overrides.

## Error Handling

This step should preserve existing failure behavior.

Rules:
- Do not add best-effort fallbacks.
- If frame extraction fails, keep throwing as today.
- If the triangle path requires invalid state, fail where the state is detected instead of silently switching back to clear-only output.

## Verification

Manual verification only for this step.

Required check:
- Launch the packaged `.gcm` in Dolphin from the stable copied path.
- Confirm whether one visible triangle appears.
- Confirm Dolphin can still stop emulation cleanly.

Interpretation:
- Visible triangle: packaged runtime draw path is good enough; next step is reintroducing authored geometry in small slices.
- No visible triangle: continue debugging only inside `GameCubeRasterRenderer` and presentation state.
