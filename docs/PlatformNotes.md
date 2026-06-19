# GameCube Platform Notes

## Builder Output

The GameCube builder assembly lives under `builder/` and is intended to be loaded dynamically by the editor through `user_settings/platforms.json`.

The current builder slice exposes GameCube platform metadata and stages payloads into the output root. It does not yet invoke the native GameCube player build or emit the final `.dol`.

## Packaged Disc Build

Packaged GameCube image builds stage a retail-style extracted-disc root and write the final raw `game.gcm` directly from that staged layout.

Required external input:

- `HELENGINE_GAMECUBE_APPLOADER_PATH`
  Required path to a real GameCube `apploader.img` that the launcher or tool installer provides.

## Generated Core Seam

The native build consumes generated engine output from `HELENGINE_CORE_CPP_ROOT` when a generated core deployment root is provided.

## Cube Test Content

Stage the minimal authored `city` content bundle required by the first GameCube 3D renderer milestone:

```bash
rtk bash tools/stage_city_cube_test_content.sh
```

## Dolphin Verification

Load `build/helengine_gc.dol` in Dolphin. The expected result for this milestone is that the authored `cube_test` scene loads through the generated runtime path and the cube is visibly rendered and rotating through the GameCube GX path.
