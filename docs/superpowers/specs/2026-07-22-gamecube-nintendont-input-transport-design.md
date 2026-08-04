# GameCube Nintendont Input Transport

## Goal

Allow packaged GameCube builds to continue past controller initialization in Nintendont while preserving the existing libogc controller path on physical GameCube hardware.

## Problem

`GameCubeInputManager` calls libogc `PAD_Init()`. On the tested vWii/Nintendont launch path the call does not return. Nintendont's log shows that it did not recognize or patch the libogc SI/PAD routines, unlike retail Nintendo-SDK-shaped controller routines.

## Design

`GameCubeInputManager` owns two input transports selected during platform-input initialization:

- The real-GameCube transport uses `PAD_Init`, `PAD_ScanPads`, and the existing libogc reads.
- The Nintendont transport never initializes libogc serial-interface input. It marks Nintendont's `SIInited` flag ready, invokes Nintendont's preloaded `PadStub` at `0x93000000` to refresh the four `PADStatus` records in its virtual-pad buffer, and maps the first record into the existing `InputFrameState` contract.

The mode decision first reads the PowerPC data-BAT registers without dereferencing Nintendont memory. Original GameCube Gekko (PVR `0x00083214`) has only the lower four data-BAT pairs; Wii-family processors expose eight, and Nintendont maps MEM2 through a high pair on vWii. The decision therefore reads four pairs on Gekko and eight on every other supported processor. It accepts the Nintendont transport only when the documented `SIInited` address at `0x93003060` resolves through a valid supervisor mapping into Wii MEM2's physical `0x10000000`-`0x13FFFFFF` range. `SIInited` cannot be a presence test because stock Nintendont initializes it to zero and sets it only through its retail `SIInit` patch; the application writes the same ready value after the safe mapping decision. The Nintendont buffer is therefore not touched on physical GameCube hardware.

## Data Flow

1. `GameCubeApplication` calls `GameCubeInputManager::InitializePlatformInput` before constructing the input backend.
2. The input manager detects Nintendont and selects the virtual-pad transport, or initializes libogc PAD for physical hardware.
3. `CaptureFrame` obtains the controller state from the selected transport and creates the existing `InputFrameState` without changing engine-facing input APIs.

## Errors and Compatibility

Nintendont detection must be strict: an absent or invalid marker selects the physical GameCube libogc transport. A Nintendont virtual-pad record with an error state is reported as disconnected rather than accessing serial-interface APIs.

## Validation

- Source-contract tests require the explicit transport selection and forbid `PAD_Init` when the Nintendont transport is selected.
- The normal GameCube builder test suite remains unchanged outside the targeted tests.
- A packaged GCM is tested in the user’s Nintendont setup. It must proceed beyond the current green `PAD_Init` checkpoint and preserve controller input for the running scene.
