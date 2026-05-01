# GameCube Player Generated-Core Bootstrap Design

## Goal

Create the first real `helengine-gc` player milestone that can boot converted `helengine.core` C++ output on GameCube, using the same external generated-core seam as `helengine-windows`, but with a GameCube-appropriate code-generation profile and a GameCube-native player host.

## Constraints

- The current generated C++ output is not truly GameCube-ready because `cs2.cpp` still defaults to a Windows-headless target profile.
- The first milestone should prioritize the player, not the build/deployment builder pipeline.
- The generated core must continue to live outside the platform repo and be consumed through `HELENGINE_CORE_CPP_ROOT`.
- The GameCube host should stay small and pragmatic for the first engine boot.
- The low-level host must remain GameCube native rather than pretending the Windows host abstractions can be reused unchanged.

## Chosen Approach

Use a two-repo implementation slice:

1. update `csharpcodegen/cs2.cpp` so it can emit a real GameCube-oriented generated-core profile
2. update `helengine-gc` so the native player can optionally compile and boot that generated core

This keeps the boundary correct. The platform repo should not work around Windows-profiled generated output, and the generator should not stay Windows-defaulted when targeting GameCube.

## Repository Scope

This milestone spans:

- `/mnt/c/dev/helworks/csharpcodegen`
- `/mnt/c/dev/helworks/helengine-gc`

The spec lives in `helengine-gc` because the user-facing goal is a GameCube player bootstrap, but the generator changes are mandatory preconditions for real boot.

## Generator Design

`cs2.cpp` already contains the concept of multiple compiler and platform profiles. GCC support already exists, and `GameCubeHeadless` already exists as a platform enum value.

What is missing is the concrete GameCube profile plumbing.

The generator changes should:

- add a concrete `CreateGameCubeHeadless()` platform profile
- make that profile describe a non-Windows host target
- mark the target as big-endian
- allow the conversion entry point to select `GCC + GameCubeHeadless`
- keep the existing Windows path working

The goal is not to redesign the whole code generator. It is to unblock a correct first GameCube-targeted generated C++ drop.

## Player Design

The GameCube host should evolve from a `GameCubeBootHost` proof-of-life into a real player bootstrap similar in responsibility to `Win32Application`.

The first GameCube player should:

- own GameCube startup and the main native loop
- initialize video and the minimal rendering state needed for a frame loop
- optionally include generated core when `HELENGINE_CORE_CPP_ROOT` points to valid generated output
- instantiate `Core`
- create native `RenderManager3D`, `RenderManager2D`, and `InputManager` bridge objects
- call `Core::Initialize(...)`
- then run `Core::Update()` and `Core::Draw()` inside the GameCube loop

## Native Bridge Scope

The first milestone does not need a full renderer.

It only needs the minimum native bridge surface required for the generated core to initialize and run without immediately crashing.

That means:

- `GameCubeRenderManager3D` can start as a minimal implementation that satisfies the generated contract and preserves the current clear/present behavior
- `GameCubeRenderManager2D` can start as a minimal no-op or storage-only implementation
- `GameCubeInputManager`, `GameCubeKeyboard`, and `GameCubeMouse` can start with simple default-state behavior if no real controller mapping is needed for first boot

The milestone is “converted engine boots,” not “real gameplay or renderer parity.”

## Build Design

The GameCube build should stay Dockerized and keep the existing explicit devkitPro wiring.

The Makefile should grow from “reserved seam only” to “real optional generated-core inclusion,” analogous to the Windows host:

- validate that `HELENGINE_CORE_CPP_ROOT` contains the expected generated files when generated core is enabled
- compile `helengine_core_unity.cpp` into the GameCube build
- include the generated root on the include path
- preserve the ability to build without generated core for host-only debugging

The first GameCube milestone does not need a builder pipeline, content deployment flow, or packaged startup scene loading.

## Runtime Design

When generated core is present, the GameCube player should:

1. initialize native video / frame presentation
2. create the native bridge objects required by `Core`
3. construct and initialize `Core`
4. enter the main loop
5. call `Update()` and `Draw()` every frame
6. continue presenting a stable frame in Dolphin

The first successful result is not a complex rendered scene. The first success condition is that the converted engine core initializes and runs inside the GameCube host without crashing.

## Verification

Success means:

1. `cs2.cpp` can emit a GameCube-profiled generated-core output instead of Windows-headless output
2. `helengine-gc` can optionally compile that generated output through `HELENGINE_CORE_CPP_ROOT`
3. the GameCube player boots in Dolphin with generated core included
4. the native loop continues running instead of crashing during `Core::Initialize()`, `Update()`, or `Draw()`

## Non-Goals

- builder/deployment pipeline work
- asset packaging
- startup scene loading
- complete GameCube renderer implementation
- full input mapping
- content parity with the Windows host

## Order Of Work

The safest order is:

1. generator profile support in `csharpcodegen`
2. regenerated GameCube-oriented core output
3. GameCube player host refactor and optional generated-core inclusion
4. minimal native bridge implementations required for boot

That ordering keeps the platform host from chasing the wrong generated target profile.
