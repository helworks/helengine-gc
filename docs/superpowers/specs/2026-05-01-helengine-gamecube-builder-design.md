# GameCube Builder Design

## Goal

Add the first `helengine.gamecube.builder` assembly so `helengine-gc` participates in the current dynamically loaded platform-builder system used by `windows` and `ps2`.

This first slice should match the current builder contract that already exists, not jump ahead into native GameCube player compilation or final `.dol` production.

## Current System

The current platform-loading system resolves builders dynamically through `helengine/user_settings/platforms.json`.

Each installed platform currently provides:

- a `builderAssemblyPath`
- a `playerSourceRootPath`
- an `IPlatformAssetBuilder` implementation
- a `PlatformBuilderDescriptor`
- a typed `PlatformDefinition`
- a `BuildAsync(...)` implementation that stages payloads into `OutputRoot`
- a platform-specific manifest written into `WorkingRoot`

The important current constraint is that neither the Windows builder nor the PS2 builder invokes the native player build yet. Both builders are still at the staged-payload level.

## Chosen Approach

Add a GameCube builder that matches the current Windows/PS2 pattern exactly:

1. dynamic builder assembly under `builder/`
2. metadata descriptor and typed platform definition
3. staged payload copy into `request.OutputRoot`
4. `gamecube-build-manifest.json` written into `request.WorkingRoot`
5. tests that verify metadata and payload staging behavior

This keeps GameCube aligned with the real platform-builder seam the editor already knows how to load.

## Repository Scope

This milestone is centered in:

- `/mnt/c/dev/helworks/helengine-gc`

It will also require a later follow-up change in:

- `/mnt/c/dev/helworks/helengine/user_settings/platforms.json`

That manifest update is part of installation/wiring, not the core builder implementation itself.

## Builder Assembly Shape

`helengine-gc` should gain a `builder/` project and a `builder.tests/` project, matching the structure already used by `helengine-windows` and `helengine-ps2`.

The builder project should include:

- `GameCubePlatformAssetBuilder.cs`
- `GameCubePlatformDefinitionFactory.cs`
- `Program.cs`
- project/solution files matching the existing platform-builder layout

The builder type should:

- implement `IPlatformAssetBuilder`
- expose `Descriptor`
- expose `Definition`
- delegate `BuildAsync(...)` into a focused staging implementation

The entrypoint should support:

- `--describe`
- `--smoke-test`

matching the current dynamic-builder command-line shape.

## Descriptor Design

The GameCube builder descriptor should identify the assembly as:

- builder id: `helengine.gamecube.builder`
- target platform id: `gamecube`

It should declare:

- engine compatibility range consistent with the current platform builders
- manifest compatibility range consistent with the current platform builders
- supported runtime backend ids containing `gamecube`
- supported build profiles containing the first GameCube build profile id

The goal is not novelty. The goal is to make the GameCube builder discoverable and loadable by the same platform-resolution code path already used for other platforms.

## Platform Definition Design

The GameCube builder should expose a typed `PlatformDefinition` so the editor can reason about build profiles, graphics profiles, and required asset classes.

The first definition should stay intentionally small:

- one build profile for the default GameCube player build
- one graphics profile representing the current GX/GameCube rendering backend
- asset requirements aligned with the kinds of staged files we already know how to package today

At this stage the definition is editor metadata, not a promise that the builder already performs a full native console build.

## Build Behavior

`BuildAsync(...)` should follow the existing Windows/PS2 behavior:

1. validate `request`, `progressReporter`, and `diagnosticReporter`
2. ensure `request.OutputRoot` and `request.WorkingRoot` exist
3. iterate staged scene payloads from the manifest
4. iterate staged loose asset payloads from the manifest
5. copy payloads into the output root using their resolved source identities
6. report progress after each item
7. emit diagnostics when inputs are missing or source files are absent
8. write a GameCube-specific build manifest into the working root
9. return a `PlatformBuildReport`

This first builder should not:

- invoke Docker
- call `make`
- regenerate the C++ core
- produce a final `.dol`
- try to synthesize a packaged runtime image

Those steps belong to a later builder maturity slice.

## Build Manifest

The GameCube builder should emit:

- `gamecube-build-manifest.json`

inside `request.WorkingRoot`.

The manifest should mirror the current staging manifests closely enough to stay familiar:

- project id
- project version
- required engine version
- output root
- copied scene entries
- copied loose-asset entries

The GameCube-specific name matters because it keeps platform build traces easy to inspect when multiple builders exist side by side.

## Testing

`builder.tests/` should verify:

1. descriptor metadata is correct
2. platform definition metadata is correct
3. `BuildAsync(...)` copies staged scene and asset payloads into the output root
4. `gamecube-build-manifest.json` is written to the working root
5. build diagnostics remain empty for a successful staging run

The tests should stay at the same maturity level as the current Windows/PS2 builder tests. This is not the place to invent a broader integration harness.

## Non-Goals

- invoking the GameCube player build
- generating a final `.dol`
- Docker orchestration from the builder
- generated-core regeneration
- startup-scene rewriting logic beyond staged payload copy
- cross-platform refactoring of the existing builders

## Follow-Up Work

Once this builder exists and passes tests, the next separate slices can be:

1. register the GameCube builder in `helengine/user_settings/platforms.json`
2. teach the builder to understand GameCube-specific packaged outputs
3. eventually invoke the native GameCube player build and stage the final `.dol`

Those are explicitly later milestones, not part of this first builder bootstrap.
