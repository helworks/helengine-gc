# GameCube Nintendont Input Transport

## Goal

Allow packaged GameCube builds to continue past controller initialization in Nintendont while preserving the existing libogc controller path on physical GameCube hardware.

## Problem

`GameCubeInputManager` calls libogc `PAD_Init()`. On the tested vWii/Nintendont launch path this waits indefinitely on the serial interface. Retail GameCube executables avoid this because Nintendont recognizes their SDK pad routines and injects its own reader while loading the DOL. The libogc routine is not one of the recognized code paths.

## Design

`GameCubeInputManager` owns two input transports selected during platform-input initialization:

- The real-GameCube transport uses `PAD_Init`, `PAD_ScanPads`, and the existing libogc reads.
- The Nintendont transport never initializes the serial interface. It reads the four `PADStatus` records maintained by Nintendont at its virtual-pad buffer, invalidates the cache before each read, and maps the first record into the existing `InputFrameState` contract.

The mode decision is based on a Nintendont-specific marker that is safe to inspect before the virtual pad buffer is accessed. The Nintendont buffer is not touched on physical GameCube hardware.

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
