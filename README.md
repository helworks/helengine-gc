# Helengine GameCube Host

This repository contains the native GameCube host scaffold for Helengine.

## Builder Output

The GameCube builder assembly lives under `builder/` and is intended to be loaded dynamically by the editor through `user_settings/platforms.json`.

The current builder slice exposes GameCube platform metadata and stages payloads into the output root. It does not yet invoke the native GameCube player build or emit the final `.dol`.

## Current milestone

- Docker-only build using devkitPro GameCube tooling
- Native `.dol` output for direct loading in Dolphin
- First boot check with a solid green screen

## Build

```bash
docker build -t helengine-gc .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

The build emits `build/helengine_gc.dol`.

## Generated core seam

The native build consumes generated engine output from `HELENGINE_CORE_CPP_ROOT` when a generated core deployment root is provided.

## Generated core build

Generate GameCube-targeted core output:

```bash
rtk dotnet run --project /mnt/c/dev/helworks/csharpcodegen/codegen/codegen.csproj -- --cpp --project /mnt/c/dev/helworks/helengine/engine/helengine.core/helengine.core.csproj --output /mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube --platform gamecube --compiler gcc --endianness big --preset gamecube-core-boot
```

Build the GameCube player with generated core enabled:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all
```

The build emits `build/helengine_gc.dol`.

## Boot check

Load `build/helengine_gc.dol` in Dolphin. The expected result for this milestone is a solid green frame with no immediate crash or reset loop.

## Dolphin verification

Load `build/helengine_gc.dol` in Dolphin.

Expected result for this milestone:

- the application reaches the steady-state green phase
- `Core::Initialize(...)` completes
- the process does not immediately crash during `Update()` or `Draw()`
