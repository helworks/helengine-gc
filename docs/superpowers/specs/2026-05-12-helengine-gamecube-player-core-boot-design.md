# GameCube Player Core Boot Stability Design

## Goal

Advance the GameCube player to the first strict runtime milestone: a generated-core GameCube build that boots in Dolphin, completes `Core::Initialize(...)`, and continues running `Core::Update()` and `Core::Draw()` without crashing.

## Current Baseline

The repository already contains the earlier bootstrap slices:

- a GameCube host under `src/platform/gamecube/`
- optional generated-core build wiring through `HELENGINE_CORE_CPP_ROOT`
- bridge placeholders for `GameCubeRenderManager3D`, `GameCubeRenderManager2D`, `GameCubeInputManager`, `GameCubeKeyboard`, and `GameCubeMouse`
- an earlier builder bootstrap under `builder/`

The code generator repository also already contains a `GameCubeHeadless` platform profile and preset plumbing. This changes the shape of the next milestone. The work is no longer “invent GameCube support from scratch.” The work is to make the existing host and generated-core seam stable enough to survive real runtime execution in Dolphin.

## Constraints

- The first milestone is `player-first`, not builder-first.
- The slice is complete only when Dolphin runtime validation succeeds.
- The GameCube host must stay native to GameCube and libogc.
- The work may span `/mnt/c/dev/helworks/helengine-gc` and `/mnt/c/dev/helworks/csharpcodegen`.
- Generated core must remain external to the platform repo and be consumed through `HELENGINE_CORE_CPP_ROOT`.
- This slice must not drift into startup-scene loading, content packaging, or renderer parity work.

## Chosen Approach

Use a host-first diagnostic slice with targeted generator follow-up only when the failure source is proven.

That means:

1. tighten the GameCube host boot pipeline so native initialization, generated-core construction, `Core::Initialize(...)`, `Update()`, and `Draw()` are isolated into explicit phases
2. make build-time generated-core validation strict and deterministic
3. make runtime failure boundaries visible instead of hiding them behind broad fallback behavior
4. patch `csharpcodegen` only if the observed failure comes from generated output assumptions rather than from the GameCube host

This is the safest way to reach a real Dolphin-stable boot without mixing multiple speculative fixes at once.

## Repository Scope

This milestone primarily touches:

- `/mnt/c/dev/helworks/helengine-gc`

It may also require targeted follow-up changes in:

- `/mnt/c/dev/helworks/csharpcodegen`

The boundary is simple:

- `helengine-gc` owns native GameCube startup, frame loop, bridge objects, and generated-core inclusion
- `csharpcodegen` owns the shape and platform assumptions of the generated C++ output

## Architecture

The runtime pipeline should be made explicit and deterministic.

The GameCube build should produce one native executable path with optional generated-core inclusion:

1. native host init
2. generated-core compilation and linkage
3. engine bootstrap
4. steady-state update and draw loop
5. visible failure isolation when a phase breaks

The host should not try to recover from engine bootstrap failure with broad “best effort” continuation. For this milestone, preserving the exact crash boundary is more important than keeping the app limping forward.

## Build Design

The current `Makefile` already contains the optional `HELENGINE_CORE_CPP_ROOT` seam and validates the presence of `helcpp_config.hpp` and `helengine_core_unity.cpp`.

For this milestone, the build contract should be:

- if `HELENGINE_CORE_CPP_ROOT` is not set, the GameCube host still builds as a host-only bootstrap
- if `HELENGINE_CORE_CPP_ROOT` is set, the build must fail immediately when required generated files are missing
- when generated core is enabled, the build must compile the generated unity source and the native GameCube bridge sources together
- the build should not silently downshift from “generated-core build” to “host-only build”

This keeps the verification surface honest. A successful generated-core build must mean that the final `.dol` really contains the generated core.

## Host Design

`GameCubeApplication` should become the authoritative owner of the boot phases.

The host responsibilities for this slice are:

- initialize `VIDEO` and framebuffer state
- initialize `GX` enough for a stable present loop
- construct the minimal generated-core bridge objects
- construct `Core`
- populate required `CoreInitializationOptions`
- call `Core::Initialize(...)`
- run `Core::Update()` and `Core::Draw()` every frame after successful initialization
- preserve a deterministic diagnostic state when any phase fails

The host should stay small. It does not need to load real content, implement a real scene pipeline, or perform sophisticated renderer work yet.

## Native Bridge Scope

The bridge layer should remain minimal but valid.

This milestone only requires enough behavior for the generated core to initialize and survive its steady-state loop:

- `GameCubeRenderManager3D` may remain mostly placeholder, but it must satisfy the generated core contract without handing back invalid native state
- `GameCubeRenderManager2D` may remain a no-op or storage-only implementation if that is sufficient for generated-core boot
- `GameCubeInputManager`, `GameCubeKeyboard`, and `GameCubeMouse` may remain simple default-state implementations unless the generated core requires more for initialization

If a bridge class is missing a required invariant, fix that invariant directly. Do not mask the problem with generalized exception swallowing or fake fallback objects.

## Runtime Design

The runtime path should be split into hard phases with no silent blending between them.

### Phase 1: Native Host Init

Initialize `VIDEO`, choose the preferred render mode, allocate the framebuffer, allocate the GX FIFO, and configure minimal GX state required for repeated presentation.

### Phase 2: Generated-Core Wiring

When `HELENGINE_CORE_CPP_ROOT` is enabled, the GameCube build must include generated core and native bridge sources as one coherent executable.

### Phase 3: Engine Bootstrap

Construct `Core`, obtain `CoreInitializationOptions`, create native bridge objects, and call `Core::Initialize(...)`.

### Phase 4: Steady-State Loop

After successful initialization, call `Core::Update()`, `Core::Draw()`, and present every frame.

### Phase 5: Failure Isolation

If boot fails, stop progression at the failing phase and preserve a deterministic visible state that distinguishes construction failure, initialization failure, or steady-state failure.

## Error Handling

Error handling should be strict and diagnostic-oriented.

- Missing generated-core files should be build failures, not warnings.
- Required engine state should not be replaced with invented defaults.
- Broad catch-all behavior should be narrowed where that helps expose the real failure phase.
- The runtime should not continue into later phases after a proven engine bootstrap failure.
- Any host-side workaround must fix a real GameCube invariant violation, not hide an unknown bug.

## Generator Follow-Up Rules

`csharpcodegen` changes are allowed, but only when the failure source is clearly on the generated-code side.

Examples of generator-owned problems:

- wrong platform preprocessor assumptions for GameCube
- incorrect endianness assumptions in emitted support code
- Windows-host-only generated behavior leaking into the GameCube target
- emitted code that violates the existing GameCube bridge contract in a platform-specific way

Examples of host-owned problems:

- bad native initialization order
- invalid or incomplete bridge object setup
- GameCube-specific runtime assumptions not satisfied before `Core::Initialize(...)`
- present-loop or GX setup problems

This boundary keeps the platform repo from accumulating workarounds for generator bugs.

## Verification

Success for this milestone means all of the following are true:

1. `csharpcodegen` can emit GameCube-profiled generated-core output
2. `helengine-gc` builds cleanly with `HELENGINE_CORE_CPP_ROOT` pointed at that generated output
3. the resulting `.dol` boots in Dolphin
4. the GameCube host completes `Core::Initialize(...)`
5. the native loop continues through repeated `Core::Update()` and `Core::Draw()` without crashing

The verification target is runtime, not just compile or link success.

## Non-Goals

- builder orchestration work
- packaged startup-scene loading
- asset deployment flow
- real renderer parity with Windows
- full controller mapping
- generalized content pipeline changes
- broad refactors across unrelated platform hosts

## Order Of Work

The safest order for implementation is:

1. inspect and tighten the existing GameCube host boot phases
2. verify generated-core build inclusion and failure behavior
3. run generated-core builds and isolate the real Dolphin failure phase
4. patch `helengine-gc` at the host or bridge seam if the failure is platform-host owned
5. patch `csharpcodegen` only if the failure is proven to come from generated output assumptions
6. rerun Dolphin verification until the loop survives `Initialize`, `Update`, and `Draw`

That ordering keeps the debugging boundary sharp and avoids speculative dual-repo churn.
