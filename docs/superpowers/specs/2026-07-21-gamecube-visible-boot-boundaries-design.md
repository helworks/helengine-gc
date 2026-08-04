# GameCube Visible Boot Boundaries Design

## Goal

Make the first physical-hardware GameCube boot test diagnostic. A vWii/Nintendont black screen must identify whether execution reached VI, GX, or a later generated-core boot boundary.

## Scope

This is a diagnostic-only host change in `GameCubeApplication`.

Included:

- present an existing boot-phase colour through VI immediately after video initialization succeeds, before GX is available
- present existing boot-phase colours through GX after GX initialization succeeds and at later boot boundaries
- preserve the current phase colours and failure colours
- retain the normal engine update, draw, and presentation loop unchanged

Excluded:

- disc-image format changes
- Nintendont configuration changes
- scene, renderer, audio, or generated-core behavior changes
- recovery from a failed phase

## Current Problem

`SetBootPhase` updates `ClearColor`, but the colour does not reach the display until `PresentFrame`. `PresentFrame` is reached only after `VIDEO`, `GX`, and generated-core initialization complete. Therefore, a hardware black screen currently combines every failure before the first normal frame into the same symptom.

## Chosen Approach

Add one narrow host-owned boot-frame presenter with two safe paths:

1. Before GX initialization, clear the configured VI framebuffer to the requested colour, select it, flush, and wait for vertical sync.
2. After GX initialization, configure the GX copy clear colour, copy the EFB to the selected framebuffer, select it, flush, and wait for vertical sync.

The presenter is called only at phase boundaries. It does not run from the steady-state frame loop and does not change the render managers or generated core.

## Boundary Sequence

| Reached boundary | Visible result | Diagnostic meaning |
| --- | --- | --- |
| video initialization complete | blue | VI and framebuffer setup completed; investigate GX next |
| GX initialization complete | yellow | GX FIFO and display-copy setup completed; investigate core construction next |
| later existing boot colours | existing phase colour | the last colour identifies the reached generated-core or scene boundary |
| no colour, still black | black | startup did not reach the post-video presentation point, or the image was not entered |

The existing magenta/red failure colours remain the visible result when a failure returns or is caught after GX is available.

## Error Handling

The diagnostic code must not invent fallback state. It must only present after the prerequisite subsystem has reported successful initialization. Existing initialization failures continue to return their existing failure code and stop later boot work.

## Testing

The native host has no standalone C++ unit-test target. Add a focused builder-side source contract test that requires the explicit VI and GX boot-presentation calls at the intended phase boundaries. This test must first fail against the current source, then pass after implementation.

Hardware validation is required after building a packaged image: record the final persistent colour shown by vWii/Nintendont. That colour is the next root-cause boundary; it is not itself treated as a runtime fix.

## Success Criteria

- a pre-GX VI presentation occurs after successful video initialization
- a GX presentation occurs after successful GX initialization
- existing core and scene phases become visible before their work begins
- ordinary per-frame presentation remains unchanged
- the targeted source-contract test and native packaged build succeed

