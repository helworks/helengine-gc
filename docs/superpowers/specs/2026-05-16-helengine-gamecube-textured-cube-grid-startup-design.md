# Helengine GameCube Textured Cube Grid Startup Design

## Goal

Advance the current packaged GameCube checkpoint from the working `colored_cube_grid` scene to the authored `textured_cube_grid` scene from the `city` project. The GameCube packaged build should boot directly into `scenes/rendering/textured_cube_grid.helen` and render the existing rotating cube grid through the same packaged-disc, generated-core, shared-lighting, and platform-cooked material pipeline that is already working for colored materials.

Success means the packaged `.gcm` no longer boots into `colored_cube_grid`. Instead, it boots directly into `textured_cube_grid`, the scene loads without runtime exceptions, and the cubes render with their authored textured materials on the GPU.

## Scope

This checkpoint includes:

- switching the packaged GameCube startup scene from `colored_cube_grid` to `textured_cube_grid`
- staging the authored textured-grid scene asset and all of its dependent cooked material and texture assets into the packaged disc root
- preserving the current PS2/DS-style `PlatformMaterialAsset` contract for GameCube
- extending the current GameCube renderer from lit authored base color to lit textured material sampling
- keeping geometry transform, projection, clipping, and rasterization GPU-driven through GX

This checkpoint explicitly excludes:

- changing the shared lighting model again
- changing scene menu flow or scene-catalog behavior beyond the direct startup scene switch
- adding new texture authoring tools or modifying the `city` scene generator
- specular, normal mapping, or shadow-texture features
- fallback software rendering or CPU-side pixel shading

## Design

### Startup Scene Ownership

The temporary retail-check packaging harness should now treat `scenes/rendering/textured_cube_grid.helen` as the direct startup/export scene for the GameCube probe image, replacing the current `colored_cube_grid` startup target.

The packaged manifest and runtime startup-scene manifest should both point at:

- scene id: `scenes/rendering/textured_cube_grid.helen`
- cooked scene asset: `cooked/scenes/rendering/textured_cube_grid.hasset`

The startup switch must not bypass the existing packaged-scene manifest path. It should use the same manifest-driven startup contract already stabilized for the colored-grid scene.

### Packaged Content Ownership

The staged disc root must include every cooked dependency required by `textured_cube_grid`:

- the cooked scene asset
- the shared cube model
- the sixteen textured cube materials
- the cooked texture assets referenced by those materials
- any shared font or UI assets still required by the scene

This should follow the same explicit staging pattern used for `colored_cube_grid`, not an ad hoc best-effort file crawl. If the scene references sixteen distinct materials and textures, the packaged manifest and staging script should enumerate them deliberately so missing assets remain diagnosable.

### Material Contract

GameCube must stay on the current shared cooked-material contract:

- materials are staged as `PlatformMaterialAsset`
- the generated runtime resolves them through `BuildMaterialFromCooked(...)`
- the GameCube-owned runtime material preserves authored base color and any texture-relative path needed for the first textured branch

The GameCube path must not fall back to desktop shader-package loading and must not reintroduce `ForwardStandardShader.dx11.hasset` reads.

### Texture Rendering Boundary

The textured-grid checkpoint should preserve the same renderer structure that already works for lit colored materials:

1. extract drawables and lights from the generated frame
2. resolve cooked material state on the CPU
3. bind GX texture state for the authored diffuse texture when present
4. multiply sampled texture color by the existing authored base-color and shared lighting result
5. let GX handle transform, clipping, and rasterization

The important constraint is that texture support must remain part of the normal renderer path. This is not a license to replace the lit branch with a temporary software-texture probe.

### Incremental Behavior

The first textured-grid success bar should stay narrow:

- the scene boots directly into `textured_cube_grid`
- textures appear on the cubes
- the cubes remain visibly lit under the current shared ambient-plus-directional path
- the current stability bar remains intact

If some cubes render textured and others do not, that is acceptable as an intermediate diagnostic, but the implementation target remains all authored textured materials in the staged scene.

### Failure Behavior

Texture-path failures must remain explicit and diagnosable:

- missing packaged texture assets should fail with the real staged path
- invalid material-to-texture resolution should report the authored cooked path involved
- the renderer should not silently fall back to solid white if texture resolution fails

The purpose of this checkpoint is to validate the authored textured-material pipeline, not to mask broken texture binding behind existing lit-color behavior.

## Files And Responsibilities

- `tmp/builder-retail-check/Program.cs`
  - switch startup scene ids and packaged manifest entries to `textured_cube_grid`
  - enumerate staged textured materials and textures explicitly
- `tools/stage_city_textured_cube_grid_content.sh`
  - stage the cooked scene, materials, textures, model, and required shared assets from the `city` cooked output
- `src/platform/gamecube/GameCubeRenderManager3D.cpp`
  - preserve cooked material ownership and carry texture-relative-path state when needed
- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  - bind the authored diffuse texture in the lit branch and combine it with the current lighting result
- `builder.tests/*`
  - extend source and packaging contract coverage for the textured-grid startup path

## Verification

Validation for this checkpoint should stay concrete:

1. run the focused GameCube builder/source contract tests for the textured-grid startup path
2. restage cooked `city` textured-grid content into the packaged disc root
3. rebuild the native packaged-disc GameCube target
4. repackage the `.gcm`
5. launch in Dolphin
6. confirm that startup now targets `textured_cube_grid`
7. confirm that the cube grid shows authored textured materials rather than the old colored-only scene

The regression bar remains strict:

- packaged `.gcm` boot must remain stable
- scene load must remain stable
- shared lighting must remain stable
- the prior matrix/projection/material regressions must not return
