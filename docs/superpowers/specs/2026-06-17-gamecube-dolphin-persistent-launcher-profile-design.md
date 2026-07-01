# GameCube Dolphin Persistent Launcher Profile

## Goal

Allow the GameCube Dolphin launcher to keep a dedicated persistent Dolphin user profile so controller mapping and other per-profile settings survive across launches, while still forcing the log window and logger outputs on each run.

## Current Behavior

`scripts/launch_gamecube_image_in_dolphin.ps1` currently deletes `tmp\dolphin-launcher-user` on every launch, recreates it, copies a subset of the global Dolphin profile into it, then rewrites `Config\Logger.ini` and `Config\Qt.ini`.

That behavior makes the launcher reproducible, but it prevents controller configuration under `GC` from persisting.

## Desired Behavior

The launcher should keep using `tmp\dolphin-launcher-user` as its dedicated profile, but it must stop deleting and recreating that directory on every launch.

On first run, when the launcher profile does not exist yet, the script should:

- create `tmp\dolphin-launcher-user`
- create `tmp\dolphin-launcher-user\Config`
- copy `GC`, `Backup`, `ResourcePacks`, and `Load` from the global Dolphin profile when present

On later runs, when the launcher profile already exists, the script should:

- keep the existing directory and contents
- avoid recopying the persistent directories

On every run, regardless of whether the profile already exists, the script should:

- force-close existing `Dolphin.exe` processes before launch
- seed `Config\Logger.ini` from the global profile and force `WriteToConsole`, `WriteToFile`, and `WriteToWindow`
- seed `Config\Qt.ini` from the global profile and force the logging window visible
- launch Dolphin with `-u <launcher-profile>` and `-e <image>`

## Rationale

This keeps the profile isolated from the main Dolphin profile while preserving stable controller configuration for the GameCube launcher.

The logging overrides remain explicit so debug visibility does not depend on stale local profile state.

## Files

- `scripts/launch_gamecube_image_in_dolphin.ps1`
- `builder.tests/GameCubeDolphinLauncherScriptTests.cs`
- `README.md`

## Validation

- Update the script contract test to assert the launcher no longer deletes the dedicated user directory on every run.
- Update the README text so it documents a persistent dedicated launcher profile instead of a recreated isolated profile.
- Run the launcher script against a built image and confirm it still prints the dedicated `USER_DIR` path and launches Dolphin successfully.
