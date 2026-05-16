# GameCube Main Menu Navigation Design

## Goal

Enable packaged GameCube builds to navigate the authored demo-disc main menu with the controller, using the shared engine menu runtime instead of a GameCube-specific menu implementation.

## Current State

The packaged GameCube build already:

- Boots into `Scenes/DemoDiscMainMenu.helen`
- Loads the authored menu scene successfully
- Produces and renders 2D menu drawables on screen
- Uses the shared generated `MenuComponent` runtime for menu state and rendering

The remaining gap is input. `GameCubeInputManager::CaptureFrame()` currently returns an empty `InputFrameState` with `GamepadCount = 0`, so the shared menu runtime never receives navigation or confirm/back events.

## Existing Shared Contract

The generated shared `MenuComponent` already owns the desired behavior:

- `DPadUp` moves selection upward
- `DPadDown` moves selection downward
- `South` confirms the selected item
- `East` navigates back
- `Select` also navigates back

Because this logic already exists in shared runtime code, GameCube should not add a platform-specific menu controller. The correct fix is to make the platform input backend populate the shared `InputGamepadState` contract.

## Recommended Approach

Implement native controller polling in `GameCubeInputManager` and map libogc pad input onto the shared `InputFrameState` / `InputGamepadState` model.

This keeps responsibilities clean:

- `GameCubeInputManager` captures hardware state
- shared `InputSystem` computes pressed/released transitions
- shared `MenuComponent` performs selection, confirm, and back behavior

## Input Mapping

The first-pass menu navigation mapping will be:

- GameCube `D-pad Up` -> `InputGamepadButton::DPadUp`
- GameCube `D-pad Down` -> `InputGamepadButton::DPadDown`
- GameCube `D-pad Left` -> `InputGamepadButton::DPadLeft`
- GameCube `D-pad Right` -> `InputGamepadButton::DPadRight`
- GameCube `A` -> `InputGamepadButton::South`
- GameCube `B` -> `InputGamepadButton::East`
- GameCube `Start` -> `InputGamepadButton::Start`

The backend should also mark the pad as connected and provide one populated gamepad in the frame when controller port 0 is available.

Analog stick values may be filled as well if easy to expose from libogc, but they are not required for this success bar because the shared menu path currently keys off D-pad and button presses.

## Scope

Included in this slice:

- Native GameCube controller polling through libogc
- Shared input-frame population for one gamepad
- Main-menu navigation through shared runtime
- Verification that `A` confirms and `B` navigates back

Not included in this slice:

- Pointer-driven menu interaction
- Analog-stick-driven menu movement
- Multi-controller gameplay/input routing
- Broader gameplay input parity beyond the main menu

## Files Expected To Change

- `src/platform/gamecube/GameCubeInputManager.hpp`
- `src/platform/gamecube/GameCubeInputManager.cpp`
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

Additional source-contract tests may be updated if they already audit the GC packaged main-menu path.

## Verification Strategy

Verification should prove both the code contract and the actual runtime behavior:

1. Source-contract tests should confirm that the GameCube input backend now:
   - polls libogc pad state
   - allocates one `InputGamepadState`
   - maps `A/B/D-pad/Start` into shared `InputGamepadButton` values
2. A packaged-disc native rebuild should succeed.
3. The rebuilt `.gcm` should boot into the main menu in Dolphin.
4. Manual runtime verification should confirm:
   - D-pad changes the selected menu item
   - `A` confirms the selected item
   - `B` returns/back-navigates

## Risks

The main risk is over-implementing around the shared input system. The menu runtime already expects standard `InputGamepadState` transitions, so GameCube should avoid inventing extra GC-specific navigation behavior or synthetic menu events. If navigation fails after the backend is populated, the next investigation point should be input capture correctness and shared transition state, not a new menu layer.
