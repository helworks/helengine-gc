# Helengine GameCube Host

This repository contains the native GameCube host scaffold for Helengine.

## Builder Output

The GameCube builder assembly lives under `builder/` and is intended to be loaded dynamically by the editor through `user_settings/platforms.json`.

The current builder slice exposes GameCube platform metadata and stages payloads into the output root. It does not yet invoke the native GameCube player build or emit the final `.dol`.

## Current milestone

- Docker-only build using devkitPro GameCube tooling
- Native `.dol` output for direct loading in Dolphin
- Generated-core boot with authored `cube_test` scene loading
- First GameCube GX 3D path for the rotating cube scene

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

## Cube Test Content

Stage the minimal authored `city` content bundle required by the first GameCube 3D renderer milestone:

```bash
rtk bash tools/stage_city_cube_test_content.sh
```

This creates:

- `tmp/city-cube-test-content/cooked/scenes/rendering/cube_test.hasset`
- `tmp/city-cube-test-content/cooked/engine/models/cube.hasset`
- `tmp/city-cube-test-content/cooked/engine/materials/standard.hasset`
- `tmp/city-cube-test-content/cooked/shaders/ForwardStandardShader.dx11.hasset`

## Rotating Cube Verification

Build the GameCube player with the generated core enabled:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all
```

Launch Dolphin from the repo root so the staged content root resolves correctly:

```bash
rtk cmd.exe /c "cd /d C:\dev\helworks\helengine-gc && start \"\" \"C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe\" \"C:\dev\helworks\helengine-gc\build\helengine_gc.dol\""
```

Expected result:

- the authored `cube_test` scene loads through the generated runtime path
- the camera framing comes from the scene asset
- the cube mesh comes from the authored runtime model reference
- the cube is visibly rendered through the GX 3D path
- the cube rotates continuously through the generated update loop

## Dolphin verification

Load `build/helengine_gc.dol` in Dolphin.

Expected result for this milestone:

- the authored `cube_test` scene loads through the generated runtime path
- `Core::Initialize(...)`, `Update()`, and `Draw()` continue running
- the cube is visibly rendered and rotating through the GameCube GX path
