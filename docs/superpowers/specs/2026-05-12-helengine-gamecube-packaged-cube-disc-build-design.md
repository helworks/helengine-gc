# GameCube Packaged Cube Disc Build Design

## Goal

Advance `helengine-gc` from a direct-`dol` development target into the first real packaged GameCube media milestone:

- build the native GameCube player executable
- stage a deterministic GameCube disc root
- package a bootable GameCube disc image
- boot the packaged `cube_test` scene from disc-backed cooked content instead of the temporary in-memory cube bootstrap

This first packaged-disc slice is intentionally narrow. It only needs to support the current rotating-cube milestone from the `city` project, and it may consume already-cooked `cube_test` output instead of owning end-to-end cooking.

## Current Context

The repository now has three important pieces:

1. a GameCube native player that boots in Dolphin and renders a rotating white cube through GX
2. a generated-core seam that can compile and run `helengine.core` C++ output on GameCube
3. a GameCube builder assembly under `builder/` that currently stages payloads and emits metadata, but does not yet build the native player or package GameCube media

There is also an earlier GameCube builder design in this repository. That spec was intentionally limited to the current editor-facing builder contract: stage payloads into the output root and write a GameCube build manifest. It explicitly did not include native compilation or packaged runtime media.

This new spec is the follow-up maturity slice. It does not replace the earlier builder design. It extends that builder from staged-output metadata into a real packaged GameCube runtime path.

## Scope Boundary

The first packaged-disc milestone is constrained as follows:

- target content: the current `cube_test` scene and its cooked dependencies
- content source: already-cooked output from the `city` project is acceptable
- runtime policy: packaged-content failure is fatal
- fallback policy: no fallback to the in-memory cube installer on packaged disc builds

This slice does not need to solve arbitrary project export on day one. It needs to prove that the GameCube platform can boot a real packaged image and load authored runtime content from disc.

## Chosen Approach

The right shape is a staged disc-layout pipeline plus generated runtime path metadata, aligned with the PS2 platform at the architectural level but GameCube-specific in execution.

The builder should mature in these steps:

1. keep the existing staged-payload builder contract
2. add a GameCube native build step that emits `build/helengine_gc.dol`
3. stage a deterministic GameCube disc root under the requested output root
4. generate GameCube runtime startup-scene and scene-catalog manifest files into the generated-core runtime folder
5. package the staged disc root into a bootable GameCube image

This preserves the existing builder seam while moving the packaged runtime behavior into explicit platform-owned components.

## Artifacts

The first packaged GameCube build should emit three explicit artifacts:

- native executable: `build/helengine_gc.dol`
- inspectable staged disc root: `<output>/disc/`
- bootable GameCube disc image: `<output>/game.gcm`

The disc image format should be treated as a raw GameCube disc image, not a generic ISO9660 shortcut. The packaging implementation may use whichever concrete toolchain is appropriate, but the builder contract should treat the result as a GameCube-native image artifact.

## Builder Evolution

The existing `builder/` project should remain the orchestration entry point. The important change is that `GameCubeBuildWorkspace` stops being only a payload-copy helper and becomes the owner of the packaged GameCube build pipeline.

The builder flow for this milestone should be:

1. validate the build request and required paths
2. stage the selected cooked `cube_test` package payloads into a working staging root
3. build the GameCube native player executable
4. generate GameCube runtime disc-path manifest files into the generated-core runtime folder
5. write the staged GameCube disc layout
6. package the staged disc root into `game.gcm`
7. return a build report whose artifacts describe the staged disc root and packaged image

The earlier staging-only behavior is not discarded. It becomes the first phase of the packaged build rather than the entire build.

## Components

The first packaged-disc milestone should add or mature these units.

### `GameCubeBuildWorkspace`

This becomes the top-level coordinator for one packaged GameCube build. It should own:

- output root paths
- working staging paths
- generated-core runtime-manifest output paths
- staged disc root path
- final `game.gcm` output path
- the ordered execution of staging, native build, manifest generation, disc layout writing, and image packaging

### `IGameCubeNativeBuildExecutor`

This interface should compile the native GameCube player for the supplied workspace and produce `build/helengine_gc.dol`.

The first implementation may continue to use Docker plus the existing `make` flow. The important design rule is that native build invocation stays behind a GameCube-owned abstraction rather than being embedded directly into the generic builder logic.

### `GameCubeDiscLayoutWriter`

This component writes the inspectable GameCube disc root consumed by final image packaging.

Its responsibilities are:

- copy the boot executable into the disc root
- copy the staged cooked `cube_test` scene and dependency payloads into deterministic GameCube physical paths
- build the logical-to-physical path map used by runtime manifest generation

Unlike the PS2 path, this writer should not produce PS2-style rooted `cdrom0:` paths or ISO `;1` suffixed paths. It should emit GameCube-specific physical paths that match the chosen runtime contract.

### `GameCubeDiscPathResolver`

This component resolves logical cooked paths into deterministic GameCube physical disc-relative paths.

The first slice does not need aggressive aliasing unless GameCube filename constraints force it, but it does need:

- deterministic casing rules
- deterministic directory layout
- stable resolution from logical cooked paths to physical disc-relative paths
- tests that prevent accidental path drift

This should be a dedicated component instead of burying path rules inside the layout writer.

### `GameCubeRuntimeAssetPathManifestWriter`

This component generates the GameCube runtime startup-scene and scene-catalog manifest files into the generated-core runtime folder.

The generated files should tell the runtime:

- which startup scene to load
- which authored scene ids exist in the packaged catalog
- which GameCube disc path each runtime scene maps to

This is the formal bridge between the builder’s physical disc layout and the generated runtime’s scene-loading contract.

### `IGameCubeImagePackager`

This interface turns the staged GameCube disc root into `game.gcm`.

The first implementation may use one concrete tool or script, but the packaging mechanism should stay isolated behind this interface so the build pipeline does not depend on one specific tool invocation shape.

## Runtime Contract

Packaged GameCube disc builds must boot through generated runtime metadata only.

For this milestone:

- `GameCubeApplication` should stop installing the in-memory `cube_test` scene on the packaged-disc path
- the startup scene must come from generated GameCube runtime manifest data
- runtime scene resolution must come from the generated GameCube scene catalog
- scene and asset loading must use GameCube disc-rooted physical paths, not host filesystem assumptions

The current `GameCubeCubeTestSceneInstaller` may remain temporarily in the repository for local diagnostics, but it must not be the normal boot path for packaged-disc builds.

## Failure Policy

Packaged-content failures are fatal in this milestone.

That means:

- if the startup-scene manifest is missing, boot fails
- if the startup scene is not present in the GameCube scene catalog, boot fails
- if the scene file is missing from the packaged disc layout, boot fails
- if required cooked dependencies are missing, boot fails
- the runtime must not silently fall back to the self-contained in-memory cube path

This is important because the purpose of the milestone is to prove the packaged disc pipeline itself, not merely preserve an image on screen.

## First Content Slice

The first packaged content slice should include only the assets required by the current `cube_test` scene.

That includes:

- the cooked startup scene for `cube_test`
- the cooked cube model
- the cooked material data required by that scene
- any other direct cooked runtime dependencies required for the scene to load and render successfully on GameCube

This should be treated as a closed, known package, not as an arbitrary project export system.

## Verification

Verification for this milestone should stay narrow and evidence-driven.

Required source-level verification:

1. tests for `GameCubeDiscPathResolver`
2. tests for `GameCubeDiscLayoutWriter`
3. tests for `GameCubeRuntimeAssetPathManifestWriter`
4. tests for the packaged-build workspace orchestration at a smoke-test level

Required build/runtime verification:

1. a build that emits `build/helengine_gc.dol`
2. a build that emits an inspectable staged disc root
3. a build that emits `game.gcm`
4. a Dolphin boot check showing the packaged image reaches the rotating cube scene through disc-backed loading

The success signal is not merely “the image boots.” The success signal is “the packaged image boots and the rotating cube scene is loaded from packaged runtime content.”

## Non-Goals

This milestone does not need to solve:

- generic arbitrary-project GameCube export
- end-to-end cooking from the editor as part of the same slice
- a final production-quality GameCube content-aliasing scheme for every future asset class
- removal of every remaining direct-`dol` development path
- a generalized multi-scene packaged title flow

Those are follow-up slices after the first packaged cube-test image is proven.

## Follow-Up Work

Once this milestone is complete, the next logical follow-ups are:

1. broaden the packaged build from `cube_test` to more authored scenes
2. replace “already-cooked input” with end-to-end cooking/export integration
3. harden GameCube disc-path rules as more asset classes and deeper directory shapes appear
4. decide whether direct-`dol` development and packaged-disc builds should remain separate profiles or converge behind one builder flow
