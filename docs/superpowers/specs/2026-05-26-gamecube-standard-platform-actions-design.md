# GameCube Standard Platform Actions Design

## Goal

Make GameCube menu interaction use the existing standard platform input contract so the demo-disc main menu responds to `A` as `Accept` and `B` as `Return`.

## Current State

The `city` menu code already uses `StandardPlatformAction.Accept` and `StandardPlatformAction.Return` through the shared engine input layer.

The GameCube backend already maps raw controller buttons consistently:

- `PAD_BUTTON_A -> InputGamepadButton.South`
- `PAD_BUTTON_B -> InputGamepadButton.East`

The failure is in platform configuration, not in runtime menu logic:

- `C:\dev\helprojs\city\settings\platform.gamecube.json` currently leaves `input.standardActions.accept` and `input.standardActions.return` as `null`
- the shared editor profile seeding currently auto-defaults `ds` and `ps2`, but not `gamecube`

As a result, the packaged GameCube manifest does not carry standard action bindings for the menu code to consume.

## Design

Use the existing standard platform input system without adding any GameCube-only fallback behavior.

### Project Binding

Update the authored GameCube platform settings in `city` so the platform explicitly binds:

- `Accept -> InputGamepadButton.South`
- `Return -> InputGamepadButton.East`

Under the current GameCube controller mapping this means:

- `A = Accept`
- `B = Return`

### Editor Default Seeding

Update the shared editor profile settings seeding so GameCube defaults missing standard actions the same way future profiles are created for `ds` and `ps2`.

GameCube should seed:

- `Accept -> South`
- `Return -> East`

This keeps the persistent project settings and future profile creation behavior aligned.

## Rejected Approaches

### Runtime Fallback In The GameCube Backend

Do not add a GameCube-specific hidden fallback that triggers menu actions directly from raw button polling when standard actions are missing.

That would:

- bypass the existing cross-platform input contract
- create behavior that differs from other platforms
- hide broken project settings instead of fixing them

### Menu-Code Changes

Do not rewrite `MenuComponent` or `DemoDiscReturnToMenuComponent` for GameCube.

Those sources already use `StandardPlatformAction.Accept` and `StandardPlatformAction.Return`, which is the correct abstraction boundary.

## Verification

Verify with a normal GameCube main-menu export and Dolphin run.

Expected behavior:

- `A` activates the selected menu item
- `B` triggers return behavior on scenes that use `DemoDiscReturnToMenuComponent`
- no GameCube-specific menu code path is required

## Scope

In scope:

- `city/settings/platform.gamecube.json`
- shared editor seeding for GameCube standard actions
- GameCube main-menu rebuild and manual runtime verification

Out of scope:

- changing raw GameCube button mapping
- adding runtime input fallbacks
- changing DS or PS2 semantics
