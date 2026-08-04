# GameCube First-Frame Trace Design

## Goal

Produce one optional packaged GameCube diagnostic that locates a first-frame hang without requiring a series of console restarts or loader modifications.

## Scope

The trace runs only after normal generated-core initialization succeeds. It presents a distinct, held diagnostic frame before and after every first-frame boundary:

1. Core initialization completed.
2. 2D render-manager `BeginFrame` completed.
3. Core `Update` completed.
4. Released 2D textures flushed.
5. Released 3D assets flushed.
6. Core `Draw` completed.
7. The host present completed.

If execution hangs, the last shown color identifies the boundary that was reached. If all boundaries complete, the application remains on a final success frame. The probe never touches memory-card APIs and does not change Nintendont.

## Configuration

Add one default-off GameCube builder setting named `enable-first-frame-trace-diagnostic`. The native build receives its resolved value through `HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC`. It is independent of the existing default-off memory-card and Nintendont-handoff diagnostics.

## Runtime behavior

The trace is enabled only when its compile definition is `1`. Its checkpoints call the normal boot-frame presentation path, wait long enough for a human to see the color transition, and retain the final color indefinitely. A disabled build preserves the existing normal-frame loop.

## Verification

Builder tests verify the default-off platform setting, environment forwarding, and native source contract. The packaged image is verified to compile with the trace enabled while both the memory-card journal and handoff-only stop are disabled.
