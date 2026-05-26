# GameCube Main Menu Normal Boot Design

## Goal

Boot the GameCube export into the authored `DemoDiscMainMenu` scene through the normal project-selected startup path and confirm it reaches the first packaged boot frame.

## Scope

This pass is boot-only.

Included:
- switch the GameCube startup scene selection in the city project to `DemoDiscMainMenu`
- rebuild a packaged GameCube ISO through the normal headless project build path
- launch Dolphin on the rebuilt ISO
- collect the first visible runtime result or the first precise boot failure log

Excluded:
- menu navigation
- return-to-menu flow
- scene transitions
- menu content fixes beyond what is required to reach first boot frame
- GameCube-specific bootstrap overrides

## Current Context

The normal GameCube startup path currently uses the GameCube platform entry in `C:\dev\helprojs\city\user_settings\build_config.json`, which is still pointed at `textured_cube_grid`.

The engine also contains older GameCube-specific bootstrap support for `Scenes/DemoDiscMainMenu.helen`, but this pass should not rely on that override because the goal is to validate the normal packaged startup contract used by the project.

## Design

### Startup Selection

Change only the GameCube platform selection in `C:\dev\helprojs\city\user_settings\build_config.json`:

- `selectedSceneIds` becomes `["DemoDiscMainMenu"]`
- `sceneOrders` becomes a single entry for `DemoDiscMainMenu`

No other platform configuration should change.

### Build Output

Build to a dedicated smoke directory:

- `C:\dev\helprojs\output\gamecube-mainmenu-smoke`

This preserves the existing textured-cubes artifact for comparison and avoids clobbering the previous rendering validation output.

### Validation Path

Use the normal headless editor build path:

- project: `C:\dev\helprojs\city\project.heproj`
- platform: `gamecube`
- output: `C:\dev\helprojs\output\gamecube-mainmenu-smoke`

Then launch Dolphin against the rebuilt `game.gcm`.

### Success Criteria

This pass succeeds when one of these happens:

1. the packaged GameCube build boots into the main menu and reaches a visible first frame, or
2. the build fails later in startup but exposes a precise runtime boot log that identifies the next real subsystem blocker

### Failure Handling

If boot fails, the next action should be guided by the first concrete `OSREPORT` failure rather than by broad menu-system changes.

## Risks

- the authored main menu may depend on a packaged asset or generated runtime path that has not been exercised by the rendering showcase scenes
- the normal startup scene id `DemoDiscMainMenu` may resolve differently than the older hardwired `Scenes/DemoDiscMainMenu.helen` bootstrap alias
- 2D menu content may expose a separate runtime issue after startup is switched away from the rendering scenes

## Recommendation

Use the normal project-selected startup path only. Do not enable or expand GameCube bootstrap overrides in this pass unless the normal path proves structurally broken.
