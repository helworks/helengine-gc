# HelEngine GameCube External Packager Integration Design

Date: 2026-05-12

## Goal

Integrate a real external GameCube disc-image packager into `helengine-gc` so the builder can progress from:

- staged cooked content
- packaged-disc-mode native DOL
- extracted `disc/` root

to:

- real `game.gcm` output

without baking packager-specific assumptions into the engine runtime or builder orchestration.

The first acceptable tool target is Windows-hosted. The long-term installation story must remain compatible with a future launcher that installs platform tools and injects deterministic tool paths into build execution.

## Decision

The first external packager backend will target `Wiimms ISO Tools` (`wit`).

Rationale:

- it is a CLI-oriented tool family instead of a GUI-only utility
- it explicitly supports creating GameCube ISO images
- it has a better long-term cross-platform story than Windows-only community GUIs
- it fits the future launcher model more naturally than an ad hoc desktop tool

Alternative tools were considered:

- `pyisotools`
  - viable fallback backend
  - cross-platform
  - adds a Python runtime dependency
  - currently presented as beta tooling
- `GCRebuilder`
  - useful Windows utility
  - not a good primary builder dependency because the long-term launcher story wants deterministic installed tooling and a cleaner CLI boundary

## Architecture

The builder keeps the existing generic packaging boundary:

- `IGameCubeImagePackager`

The first concrete implementation becomes:

- `GameCubeWiimmsIsoToolsImagePackager`

Responsibilities:

- validate configured `wit` executable path
- adapt canonical extracted GameCube disc layout if the external tool requires a different directory structure
- invoke the external packager with deterministic arguments
- produce `<output>/game.gcm`
- surface failures as explicit builder diagnostics

The builder orchestration stays unchanged at the high level:

1. stage cooked artifacts
2. emit GameCube runtime scene manifest
3. build packaged-disc-mode DOL
4. write canonical extracted `disc/` root
5. package `game.gcm`

The only changed part is step 5, which moves from fake image packaging or future custom-image work to a concrete external CLI backend.

## Tool Discovery

Tool discovery must be explicit and launcher-friendly.

First implementation contract:

- packaged GameCube builds read the `wit` executable path from configuration supplied to the builder
- the minimum acceptable configuration source is an environment variable such as `HELENGINE_GAMECUBE_WIT_PATH`
- the builder must not rely on broad machine scanning as the primary contract

Why:

- future launcher ownership is clear
- local and CI behavior stays deterministic
- build failures explain missing setup directly

Primary failure cases:

- tool path missing
  - fail packaged build with a diagnostic that the GameCube image packager is not installed or configured
- tool path configured but file missing
  - fail packaged build with a diagnostic that the configured executable path is invalid
- tool invocation fails
  - fail packaged build with a diagnostic that includes exit-code context and relevant stdout/stderr output

PATH-based probing may be added later as a convenience fallback, but it is not part of the first required contract.

## Canonical Layout vs Tool Layout

The engine-owned canonical extracted layout remains:

- `disc/sys/main.dol`
- `disc/files/...`

That layout is the builder’s stable internal contract and must not be distorted around one packager.

External tool integration must assume that `wit` may require a more standard extracted-disc structure, potentially including system artifacts such as:

- `boot.bin`
- `bi2.bin`
- `apploader.img`
- `fst.bin`

Therefore the design introduces an isolated tool-specific adaptation layer when needed:

- `GameCubeWiimmsDiscLayoutAdapter`

Responsibilities:

- read the canonical extracted layout
- create a temporary tool-consumable staging root if required by `wit`
- leave canonical builder outputs unchanged

Rules:

- if `wit` accepts the canonical layout directly, no adapter transform is needed
- if `wit` requires a different extracted-disc structure, the adapter owns that translation
- no packager-specific path semantics leak into runtime content lookup, `GameCubeSceneBootstrap`, or scene manifest generation

This keeps the builder extensible for later alternate backends such as `pyisotools`.

## Builder Components

### `GameCubeWiimmsIsoToolsOptions`

Purpose:

- carry the configured external packager executable path
- optionally carry extra deterministic flags if needed later

Initial fields:

- `ExecutablePath`

### `GameCubeWiimmsIsoToolsImagePackager`

Purpose:

- implement `IGameCubeImagePackager`
- run `wit`
- package the extracted disc root into `game.gcm`

Behavior:

- validate options
- prepare any tool-specific staging root
- execute external process
- verify `game.gcm` exists after successful completion

### `IGameCubeProcessRunner`

Purpose:

- isolate external process execution for unit testing

Behavior:

- run command
- capture exit code, stdout, stderr

This avoids tying packaging tests to real process execution.

### `GameCubeWiimmsDiscLayoutAdapter`

Purpose:

- transform canonical extracted layout into whatever `wit` actually needs, if necessary

This component only exists if real integration proves the canonical layout is insufficient.

## Runtime Impact

No runtime behavior changes are needed beyond the packaged-disc boot work already designed and partially implemented.

Runtime responsibilities remain:

- packaged-disc DOL boots with `HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT=1`
- `GameCubeApplication` uses `GameCubeSceneBootstrap::GetPackagedContentRootPath()`
- `GameCubeApplication` uses `GameCubeSceneBootstrap::CreatePackagedSceneCatalog()`
- `GameCubeApplication` loads `GameCubeSceneBootstrap::GetPackagedStartupSceneId()`

The external packager integration only changes how the packaged disc image is produced, not how the runtime resolves content once booted.

## Diagnostics

Packaged-build diagnostics must be explicit and actionable.

Required diagnostic cases:

- missing `wit` configuration
- invalid `wit` executable path
- external packager non-zero exit
- output image missing after reported success
- tool-layout adaptation failure

Diagnostics should name:

- configured tool path
- staging root path
- destination image path
- exit code when relevant

When stdout/stderr is captured, the builder should include the relevant failure excerpt in diagnostics instead of silently swallowing it.

## Testing

Source-level verification:

- unit tests for missing-tool diagnostics
- unit tests for invalid-path diagnostics
- unit tests for successful packaging orchestration through a fake process runner
- unit tests for tool-layout adaptation if an adapter is needed
- source tests that the builder reads the explicit configuration contract rather than hard-coded machine scanning

Integration verification:

- packaged builder emits:
  - `disc/`
  - `game.gcm`
- packaged-disc-mode DOL still compiles
- Dolphin boots the produced `game.gcm`
- runtime reaches the rotating cube scene through packaged content, not the in-memory installer

## Constraints and Non-Goals

In scope:

- first real external GameCube packager integration
- launcher-friendly deterministic tool discovery contract
- builder diagnostics and orchestration

Out of scope:

- writing a custom GameCube image writer in this slice
- making packaged-disc builds launcher-managed today
- adding multiple external packager backends in the same implementation slice
- changing the runtime scene/content model again

## Implementation Shape

Expected file additions and changes:

- `builder/GameCubeWiimmsIsoToolsOptions.cs`
- `builder/GameCubeWiimmsIsoToolsImagePackager.cs`
- `builder/IGameCubeProcessRunner.cs`
- `builder/GameCubeProcessRunner.cs`
- `builder/GameCubeBuildWorkspace.cs`
- `builder/Program.cs`
- `builder.tests/GameCubeWiimmsIsoToolsImagePackagerTests.cs`
- `builder.tests/Builders/FakeGameCubeProcessRunner.cs`

Conditional addition if required by real `wit` behavior:

- `builder/GameCubeWiimmsDiscLayoutAdapter.cs`
- `builder.tests/GameCubeWiimmsDiscLayoutAdapterTests.cs`

## Acceptance Criteria

The slice is complete when all of the following are true:

1. the builder can be pointed at an installed `wit` executable through explicit configuration
2. packaged builds fail clearly when the tool is missing or invalid
3. packaged builds emit a real `game.gcm`
4. the packaged-disc native DOL and runtime manifest flow still compile
5. Dolphin can boot the produced `game.gcm`
6. the rotating cube scene renders through packaged startup
