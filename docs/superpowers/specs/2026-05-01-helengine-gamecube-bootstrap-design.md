# Helengine GameCube Host Bootstrap Design

## Summary

This design defines the first native GameCube host milestone for Helengine.

The goal is to create a reproducible Docker-based build that emits a Dolphin-loadable `.dol` and displays a solid green screen. The repository should also be prepared for later `cs2.cpp` generated-core integration, but that integration is not part of this first milestone.

The scope stays intentionally small:

- one devkitPro-based Docker image
- one native build script
- one thin GameCube host bootstrap
- one `.dol` output
- one verification path through Dolphin

## Goals

- Build the GameCube host entirely inside Docker.
- Emit `build/helengine_gc.dol` as the first boot artifact.
- Keep the native entrypoint thin and put platform logic behind a host class boundary.
- Reserve a generated-core input seam now so later `helengine.core` C++ handoff does not require a repo redesign.
- Match the early-stage scaffolding style already used by `helengine-ps2`.

## Non-Goals

- Integrating generated `helengine.core` output in this milestone.
- Building a disc image such as `.iso` or `.gcm`.
- Designing the full deployment-root builder/orchestrator flow.
- Implementing real rendering abstraction, input, audio, or asset loading.
- Supporting anything beyond a stable green-screen boot test in Dolphin.

## Design Principles

### Keep the first milestone testable

The first GameCube host should prove only the toolchain, native boot path, and emulator verification loop.

If the repository tries to solve packaging, generated-core binding, and runtime systems immediately, the first milestone will become harder to verify and harder to debug.

### Keep the host boundary clean

`main.cpp` should remain a tiny launcher. Platform initialization, framebuffer or GX setup, and frame presentation belong in a dedicated `GameCubeBootHost` type.

This keeps future runtime binding work isolated from the executable entrypoint.

### Prepare the generated-core seam without forcing it

The build should expose a generated-core root variable now, but phase 1 should not require generated code to exist.

That gives the repository a stable path for later `cs2.cpp` handoff while avoiding fake placeholder integration.

### Let Docker own the toolchain

The host machine should not need a local devkitPro installation for the first milestone.

The Docker image should contain the GameCube build dependencies so the build remains reproducible and host drift is minimized.

## Repository Shape

The first milestone should create only these top-level artifacts:

- `Dockerfile`
- `Makefile`
- `README.md`
- `src/main.cpp`
- `src/platform/gamecube/GameCubeBootHost.hpp`
- `src/platform/gamecube/GameCubeBootHost.cpp`

This structure mirrors the small PS2 bootstrap and leaves room for later expansion without premature layering.

## Architecture

### Docker image

The Docker image is responsible for provisioning a Linux-based devkitPro GameCube build environment.

It should include:

- the GameCube-targeted `devkitPPC` toolchain
- `libogc`
- the standard build utilities needed to run `make`

The image should drop the user into `/workspace` so the repository can be mounted directly and built with one command.

### Native build script

The build script should own:

- compiler and linker configuration
- source file list
- output location under `build/`
- a reserved generated-core root variable for future use

The initial artifact should be:

- `build/helengine_gc.dol`

The generated-core variable should exist as a forward-compatible seam, but the build should not attempt to compile generated output in this phase.

### Native entrypoint

`src/main.cpp` should stay minimal:

1. create `GameCubeBootHost`
2. call `Run()`
3. return its exit code

This keeps the entrypoint stable even when later runtime binding work expands.

### GameCube host class

`GameCubeBootHost` should own the phase-1 native boot flow:

- GameCube video initialization
- framebuffer or GX setup
- frame clear and present logic
- the main loop

Its responsibility is only to prove the platform boot path and stable frame presentation.

## Build Flow

The first milestone build flow should be:

1. `docker build -t helengine-gc .`
2. `docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make`
3. the container compiles the native GameCube host
4. the build emits `build/helengine_gc.dol`
5. Dolphin loads the `.dol` directly for verification

This flow keeps the repository honest about its current stage and avoids pretending that broader orchestration already exists.

## Generated-Core Readiness

This repository should be ready for later external generated C++ input in the same general style as `helengine-windows`, but only at the boundary level for now.

Phase 1 readiness should include:

- a named build variable that can later point to generated C++ output
- a source layout that keeps platform bootstrap code separate from future runtime binding code
- documentation that makes it clear generated-core integration is planned but not yet wired

Phase 1 should not include:

- stub generated files
- fake handoff manifests
- conditional compilation paths that do nothing useful

The seam should be real, but it should remain inactive until real generated output exists.

## Runtime Behavior

When the host starts successfully, it should enter an infinite present loop and display a solid green frame.

That frame is the only required visual behavior for this milestone. No text, UI, assets, or engine runtime boot is needed yet.

## Error Handling

The host should fail directly when required initialization cannot be completed.

Expected rules:

- if GameCube video initialization fails, `Run()` returns a non-zero exit code
- if framebuffer or GX setup fails, `Run()` returns a non-zero exit code
- the build should not create fallback rendering paths
- the code should not silently construct default runtime state

This matches the broader Helengine rule to fix or surface invalid initialization rather than hiding it with best-effort behavior.

## Verification

The first verification pass is intentionally narrow.

Build verification:

- the Docker image builds successfully
- `make` completes inside the container
- `build/helengine_gc.dol` is emitted

Runtime verification:

- Dolphin loads `build/helengine_gc.dol`
- the application does not immediately crash or reset
- the visible result is a solid green screen

This is enough proof for the first playable-platform bootstrap.

## Documentation

`README.md` should document:

- the Docker image build command
- the Docker container run command
- the expected `.dol` output path
- the Dolphin verification expectation of a solid green frame

The documentation should stay focused on the current milestone and avoid speculative future builder usage.

## Risks And Mitigations

### Risk: toolchain setup is host-dependent

Mitigation:

- keep the GameCube toolchain inside Docker
- document only the Docker-based build path

### Risk: the first scaffold grows into premature builder infrastructure

Mitigation:

- restrict phase 1 to a `.dol` build and green-screen verification
- reserve only one generated-core seam instead of implementing a fake integration path

### Risk: platform code spreads into the entrypoint

Mitigation:

- keep `main.cpp` minimal
- place GameCube-specific setup and frame logic in `GameCubeBootHost`

## Success Criteria

This design is successful when:

- the repository builds the GameCube host inside Docker
- the build emits `build/helengine_gc.dol`
- Dolphin can load the `.dol`
- the screen stays solid green
- the repository remains ready for later generated-core integration without having overbuilt phase 1
