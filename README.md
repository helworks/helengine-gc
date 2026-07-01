# Helengine GameCube Host

This repository contains the GameCube platform host and builder integration for Helengine.

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform gamecube `
  -Output ..\helprojs\city\gamecube-build
```

## Run In Emulator

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_in_emulator.ps1 `
  -ArtifactPath ..\helprojs\city\gamecube-build\game.gcm
```

Set `HELENGINE_GAMECUBE_DOLPHIN_PATH` to override the default sibling `..\emus\...` Dolphin executable path, and `HELENGINE_GAMECUBE_DOLPHIN_PROFILE_ROOT` to override the global Dolphin profile root if needed.

## More Docs

- [Docker Build Notes](docs/Docker.md)
- [Platform Notes](docs/PlatformNotes.md)
