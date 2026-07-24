# Helengine GameCube Host

This repository contains the GameCube platform host and builder integration for Helengine.

## Build

```powershell
dotnet run --project ..\helengine\tools\build-waiter\helengine.buildwaiter.csproj -- `
  --output ..\helprojs\city\gamecube-build `
  --require game.gcm `
  -- powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\scripts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform gamecube `
  -Output ..\helprojs\city\gamecube-build
```

The Build Waiter returns successfully only after `game.gcm` is fresh and non-empty.

## Run In Emulator

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_in_emulator.ps1 `
  -ArtifactPath ..\helprojs\city\gamecube-build\game.gcm
```

Set `HELENGINE_GAMECUBE_DOLPHIN_PATH` to override the default sibling `..\emus\...` Dolphin executable path, and `HELENGINE_GAMECUBE_DOLPHIN_PROFILE_ROOT` to override the global Dolphin profile root if needed.

## More Docs

- [Docker Build Notes](docs/Docker.md)
- [Platform Notes](docs/PlatformNotes.md)
