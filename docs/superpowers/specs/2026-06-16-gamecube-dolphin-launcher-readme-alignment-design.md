# GameCube Dolphin Launcher and README Alignment Design

## Goal

Align the GameCube host repository with the established Wii developer workflow for editor-owned package builds and Dolphin launch verification, while preserving GameCube-specific artifact naming and behavior.

## Current State

The GameCube repository currently documents Docker-oriented build steps near the top of `README.md` and still tells developers to launch Dolphin manually against the built artifact.

The repository already contains `tools/build_and_launch_editor_gamecube_probe.ps1`, but that script is a heavier probe workflow that builds the editor and builder directly, copies the final image to a probe directory, and does not match the checked-in Wii launcher contract or README structure.

The Wii repository establishes a clearer developer pattern:

- `README.md` starts with a concise host status summary.
- The primary build flow uses the shared `..\helengine\artifacts\build-platform.ps1` wrapper.
- Dolphin launch is documented through a checked-in PowerShell script with an explicit image argument.
- The launcher uses an isolated Dolphin user directory and forces logger visibility.
- Builder tests guard both the script contract and README documentation contract.

## Requirements

### README alignment

Rework the top-level GameCube README so it follows the same structure as the Wii README as closely as possible:

- `Current status`
- `Editor CLI build`
- `Launching in Dolphin`
- `Verification`

Lower-level GameCube-specific details that already exist in the README should remain, but move below the primary day-to-day developer workflow sections.

### Build workflow

Document the shared editor-owned wrapper as the primary GameCube build path:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform gamecube `
  -Output ..\helprojs\city\gamecube-build
```

The README should explain that this runs the shared editor CLI with `--build gamecube` and emits the packaged GameCube image to the requested output directory.

### Dolphin launch workflow

Add a checked-in script at `scripts\launch_gamecube_image_in_dolphin.ps1` with a contract intentionally close to the Wii launcher.

Required contract:

- Require `[Parameter(Mandatory = $true)] [string]$ImagePath`
- Resolve the image path to a full path
- Fail fast when the image path does not exist
- Fail fast when Dolphin, the global Dolphin profile root, `Config\Qt.ini`, or `Config\Logger.ini` is missing
- Force-close any running `Dolphin` processes before launch
- Recreate an isolated Dolphin user directory under the GameCube repo `tmp\` root
- Seed the isolated user directory from the global Dolphin profile using GameCube-relevant directories
- Write an isolated `Config\Logger.ini` that forces console, file, and window logging on
- Write an isolated `Config\Qt.ini` that forces the log window visible
- Start Dolphin with `-u <userdir> -e <image>`
- Print image path, image last write time, Dolphin path, isolated user dir, seeded logger config path, log window enabled state, and spawned process id

### Profile seed behavior

The GC launcher should stay close to the Wii script shape but use GameCube-appropriate profile content.

Seed directories:

- `GC`
- `Backup`
- `ResourcePacks`

Optional additional seed directory:

- `Load`

The implementation should keep the script readable and deterministic. The isolated profile exists to prevent cross-run contamination and to make the logger configuration predictable for repeated verification runs.

### Verification coverage

Add builder test coverage mirroring the Wii launcher contract tests:

- verify the launcher exists
- verify it keeps an explicit `-ImagePath` contract
- verify it force-closes Dolphin
- verify it reads the image timestamp
- verify it references `GC`, `Logger.ini`, and `Qt.ini`
- verify it launches Dolphin with `-u` and `-e`
- verify it prints the spawned process id
- verify it does not hardcode a specific sample image path

Add README coverage that checks the documented launcher workflow appears in `README.md`.

## Files To Change

- `README.md`
- `scripts/launch_gamecube_image_in_dolphin.ps1`
- `builder.tests/GameCubeDolphinLauncherScriptTests.cs`

## Tradeoffs

### Why not reuse the existing probe script?

The existing probe script mixes build orchestration, probe image copying, and launch behavior into one heavier workflow. That is useful for a specific smoke path, but it does not match the simpler Wii contract for “build with the shared wrapper, then launch an explicit image in Dolphin.”

### Why not use ISO naming to match Wii exactly?

The GameCube packaged artifact in this repo is `game.gcm`. Mirroring the Wii wording too literally would make the docs less accurate. The alignment goal should preserve structure and behavior, not incorrect artifact terminology.

## Implementation Notes

- Keep the existing probe script untouched unless a follow-up change explicitly asks to consolidate it.
- Create the new launcher under `scripts/` to match the Wii repository layout.
- Keep the README’s lower-level native build and generated-core sections, but move them below the primary editor and Dolphin workflow.
- Follow repository C# conventions for XML comments and standard member ordering in the new test file.

## Success Criteria

- The GameCube README reads like the Wii README for the primary developer workflow.
- Developers can build a GameCube image through the shared editor wrapper and launch it through a checked-in Dolphin script.
- The launcher uses an isolated Dolphin user directory and predictable logging configuration.
- Automated tests guard the README and launcher contract so the workflow does not drift.
