# Tilt Trial Walnut Player Sphere Design

## Goal

Resume the paused `Tilt Trial` ball visual work by giving the real `Tilt Trial` `PlayerSphere` a new texture-backed medium polished walnut finish on GameCube. The look should read as varnished wood, but it must arrive through the normal authored content, cooking, and runtime material flow instead of through a GameCube-only override.

## Scope

This design covers:

- one new authored material asset for the `Tilt Trial` `PlayerSphere`
- one new authored diffuse texture asset for the walnut finish
- scene or scene-generation wiring so `PlayerSphere` references that new material
- builder cooking of that material into the existing GameCube cooked material contract
- GameCube runtime consumption of the cooked textured material through the existing path
- focused validation that the walnut sphere appears in `Tilt Trial`

This design does not cover:

- wood treatment for stage pieces, pads, trim, or UI
- alternate wood variants or material customization controls
- temporary native test-scene overrides for the ball look
- broader `Tilt Trial` gameplay expansion beyond the minimum hookup needed for the real sphere material path

## Chosen Approach

Use the standard authored-content path end to end.

The `Tilt Trial` `PlayerSphere` should stop depending on a generic sphere material reference and instead point at a new `Tilt Trial`-specific walnut material asset. That material should reference a new diffuse wood texture and should cook into the existing GameCube `PlatformMaterialAsset` format with a populated `TextureRelativePath`. GameCube should then render the ball through the current textured material pipeline with no sphere-specific native special cases.

This is preferred over a native override because it keeps the look portable across runtimes, validates the real GameCube texture-backed material path, and avoids leaving cleanup-only native debt behind after `Tilt Trial` is fully live.

## Visual Direction

The sphere should target a medium polished walnut finish:

- warm medium-brown body color
- visible wood grain, not flat brown shading
- polished varnish feel without pushing the finish so glossy that the grain becomes unreadable at gameplay distance
- readable contrast on GameCube against the current course materials and sky clear color

Only the `PlayerSphere` receives this treatment in the first pass. Nearby `Tilt Trial` elements should remain unchanged.

## Asset Ownership

The walnut look must be owned by authored City content rather than by native GameCube code.

Required authored assets:

- one new `Tilt Trial` sphere material asset
- one new walnut diffuse texture asset referenced by that material

The material naming should make its ownership obvious and keep it scoped to `Tilt Trial` or the player sphere instead of suggesting a generic engine-wide wood preset.

## Scene Hookup

The real `Tilt Trial` scene path should reference the new walnut sphere material directly on `PlayerSphere`.

If the current `Tilt Trial` implementation still exists only in design or partial authoring state, the minimum resumed implementation should be to finish enough of the real scene-generation or authored-scene path so that:

- `PlayerSphere` exists in the real `Tilt Trial` scene
- `PlayerSphere` references the new walnut material
- the GameCube packaged build reaches that scene without a fake slope-scene substitution

The walnut look should not be introduced by hardcoded checks against entity names, scene ids, or runtime test overrides in native code.

## Build And Runtime Flow

The builder should keep using the current GameCube material cooking pipeline:

1. The authored walnut material is read by the builder.
2. The material cooker emits a `PlatformMaterialAsset`.
3. The cooked material includes the walnut texture through `TextureRelativePath`.
4. The packaged build includes the cooked texture asset.

At runtime, GameCube should keep using the existing textured material flow:

1. `GameCubeRenderManager3D` deserializes the cooked material.
2. The runtime material receives the cooked base color and `TextureRelativePath`.
3. The diffuse texture is resolved and loaded into a runtime texture object.
4. `GameCubeRasterRenderer` binds that runtime texture and renders the sphere through the textured branch.

No new native branch should exist for `Tilt Trial`, `PlayerSphere`, or walnut materials specifically.

## Validation

Run the smallest validation that proves the real path works:

- a source-level check that the live `Tilt Trial` scene authoring or generator references the new walnut sphere material
- a builder-focused check that the cooked GameCube material payload includes a non-empty `TextureRelativePath`
- one GameCube smoke run that launches directly into the real `Tilt Trial` scene and confirms the sphere renders with the walnut texture

If the smoke run depends on temporary launch configuration, keep that change local to verification and restore the default startup path afterward.

## Risks And Constraints

The main constraint is that the current repository snapshot appears to contain `Tilt Trial` design and plan documents but not a completed live runtime path in GameCube code yet. Because of that, the resumed walnut-sphere work may require finishing enough of the real `Tilt Trial` hookup first so the authored sphere material can actually reach GameCube.

That constraint should not be solved with a temporary GameCube-only material override. The resumed work should stay aligned with the real `Tilt Trial` scene path even if that means the material task also needs a small amount of scene hookup work before the visual result can be seen.
