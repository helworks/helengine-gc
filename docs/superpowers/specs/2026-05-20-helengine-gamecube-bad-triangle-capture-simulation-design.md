# GameCube Bad Triangle Capture Simulation Design

## Goal

Create a temporary local-only GameCube renderer patch that simulates the old pre-matrix-fix broken triangle behavior for video capture. The simulation should run on the latest stable code so scene boot, menu flow, and packaged asset loading remain reliable while the visible 3D result looks intentionally wrong.

## Success Criteria

- The latest GameCube build still boots and reaches the current scene reliably.
- The visible 3D output shows only two white triangles.
- The triangles periodically:
  - disappear and reappear
  - jump in scale or translation
  - use obviously broken matrix composition
- The effect is deterministic enough for repeatable footage capture.
- The change is isolated to temporary local renderer code and can be reverted cleanly after recording.

## Non-Goals

- Reconstructing the exact historical bug bit-for-bit.
- Changing shared engine math, editor cook behavior, or scene content.
- Preserving this mode as a permanent user-facing feature.

## Approach

The simulation will live entirely inside the latest `GameCubeRasterRenderer` path. That keeps the failure scoped to 3D submission and avoids destabilizing:

- packaged scene boot
- menu rendering and navigation
- content loading
- 2D overlay paths

The renderer will gain a temporary local capture mode that intentionally degrades the first visible mesh submission into a controlled bad output:

- force geometry color to white
- emit only two triangles from the first runtime submesh
- apply intentionally wrong model-view composition
- periodically apply exaggerated transform distortion
- periodically suppress one or both triangles for a few frames

## Behavior Design

### Triangle Count

The simulation will clamp the rendered output to two triangles only, taken from the earliest mesh submission in the first frame plan. This matches the footage target better than full cube rendering.

### Color

Both triangles will be forced to white. This matches the remembered “magenta that was actually white” visual result closely enough for capture while avoiding unnecessary texture or lighting complexity.

### Broken Matrix Path

The simulation will bypass the current correct world/view behavior and intentionally use a wrong composition order for the capture triangles. This should create the warped, unstable motion that matches the old footage.

### Timed Distortion

A deterministic frame-based pattern will modulate the capture triangles:

- normal-looking wrong state for most frames
- occasional oversized or offset state
- occasional fully skipped draw

This creates the visible “flashing in and out of existence” and “matrixes messing up” effect without relying on real unstable memory corruption.

## Integration Boundary

The temporary logic should be isolated to GameCube-specific files only:

- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- `src/platform/gamecube/GameCubeRasterRenderer.hpp`

No shared engine files, no editor files, and no asset-side changes should be part of this capture patch.

## Safety and Revertability

The simulation is intentionally temporary and should be easy to remove after recording. The implementation should keep the patch localized and obvious, with no hidden dependency on generated core output or cooked assets.

## Verification

Verification for this temporary capture mode is visual rather than test-driven:

1. build latest GameCube image
2. launch Dolphin
3. confirm two white triangles appear
4. confirm periodic disappearance and transform corruption
5. record footage
6. revert the temporary patch

## Recommended Next Step

Implement the temporary capture-mode renderer patch on the latest code, rebuild, relaunch Dolphin, and validate the visible behavior for recording.
