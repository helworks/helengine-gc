# GameCube Texture Region Upload Plan

## Failure

The real DemoDisc packaged build now reaches devkitPPC compilation. `GameCubeRenderManager2D` is abstract because the current generated `RenderManager2D` contract requires `UpdateTextureRegionCore`. This hook is also required at runtime: the software path tracer uploads one completed RGBA8 tile into its persistent 320x240 presentation texture.

## Repair

1. Add source-contract tests for the exact generated override signature, renderer-owned texture validation, GX RGB5A3 tiled addressing, bounded per-block-row cache flushes, and texture-cache invalidation.
2. Add `GameCubeRuntimeTexture::UpdateRegionRgba8`. Convert only the requested logical pixels to RGB5A3 and write them into the existing 4x4-tiled native allocation. Preserve all texels outside the rectangle; do not allocate a replacement texture or retain the managed input array.
3. Implement `GameCubeRenderManager2D::UpdateTextureRegionCore` as a narrow ownership/type/disposal check that delegates to the runtime texture.
4. Run the focused source tests, full GameCube builder suite, and the Docker native packaged build. Then point DemoDisc's temporary GameCube catalog at this isolated player worktree and rerun the actual disc build.

## Acceptance

- `GameCubeRenderManager2D` is concrete against current generated core.
- A tile upload updates the existing GX texture memory in place.
- RGBA8 rows honor `sourceRowPitch`.
- Cache flushing is bounded to touched 4x4 block rows and GX texture cache is invalidated.
- No per-upload heap allocation and no full-texture recreation.
- User-owned dirty files in `C:\dev\helworks\helengine-gc` remain untouched.
