# Helengine GameCube Host

This repository contains the native GameCube host, the GameCube platform builder integration, and the GameCube-specific runtime source audits for Helengine.

## Current status

- The shared editor CLI can build GameCube packages with platform id `gamecube`.
- The packaged-disc output is a raw `game.gcm` image suitable for Dolphin launch verification.
- The repository still keeps lower-level Docker and generated-core workflows for native runtime bring-up.
- Source-audit coverage exists for GameCube packaged runtime generation, disc layout staging, and image packaging.

## Editor CLI build

If your workspace keeps `helengine-gc`, `helengine`, and `helprojs` as sibling directories, use the shared wrapper like this:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform gamecube `
  -Output ..\helprojs\city\gamecube-build
```

That wrapper runs the main editor CLI with `--build gamecube` and writes the generated GameCube package to the output directory you provide.

## Launching in Dolphin

Use the checked-in launcher script:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_gamecube_image_in_dolphin.ps1 `
  -ImagePath .\tmp\packaged-disc-proof-life\game.gcm
```

The launcher requires an explicit `-ImagePath`. Before launch it force-closes any running `Dolphin.exe` processes, creates a persistent dedicated Dolphin user directory under `tmp\` on first run, copies the `GC`, `Backup`, `ResourcePacks`, and `Load` directories from the global Dolphin profile when present during that initial seed, rewrites `Logger.ini` from the global Dolphin profile channel set while forcing `WriteToConsole`, `WriteToFile`, and `WriteToWindow` on for every launch, and rewrites `Qt.ini` on every launch to keep the Dolphin logger window visible.

The launcher prints:

- the image path
- the image last write time
- the Dolphin executable path
- the persistent dedicated Dolphin user directory path
- the seeded `Logger.ini` path
- that the logger window is enabled
- the spawned Dolphin process id

It then starts Dolphin with `-u <userdir> -e <image>`.

The script fails fast when:

- `-ImagePath` is missing
- the image file is missing
- the Dolphin executable is missing
- the logging profile seed files are missing

## Verification

Run the focused GameCube launcher audit:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "GameCubeDolphinLauncherScriptTests"
```

This verifies the current GameCube-specific contract around:

- the explicit Dolphin image launcher workflow
- the README documentation for the launcher workflow

## Builder Output

The GameCube builder assembly lives under `builder/` and is intended to be loaded dynamically by the editor through `user_settings/platforms.json`.

The current builder slice exposes GameCube platform metadata and stages payloads into the output root. It does not yet invoke the native GameCube player build or emit the final `.dol`.

## Docker Native Build

```bash
docker build -t helengine-gc .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

The build emits `build/helengine_gc.dol`.

## Packaged Disc Build

Packaged GameCube image builds now stage a retail-style extracted-disc root and write the final raw `game.gcm` directly from that staged layout.

The normal packaged-disc flow still builds these native artifacts inside the Docker toolchain:

- `build/packaged-disc/apploader.img`
- `build/packaged-disc/gbi.hdr`

Required external input:

- `HELENGINE_GAMECUBE_APPLOADER_PATH`
  Required path to a real GameCube `apploader.img` that the launcher or tool installer provides.

The packaged-disc builder no longer falls back to the repo-built packaged native `apploader.img`, because that artifact is not a real retail-style GameCube disc apploader and causes Dolphin boot failures in packaged images.
The builder also no longer defaults to the cubeboot ISO path for final packaging, because that path emits an El Torito ISO that Dolphin does not boot as a real GameCube disc image.

The builder stages an inspectable extracted-disc root at `disc/`:

- `disc/setup.txt`
- `disc/sys/boot.bin`
- `disc/sys/bi2.bin`
- `disc/sys/apploader.img`
- `disc/sys/main.dol`
- `disc/sys/fst.bin`
- `disc/files/...`

That extracted-disc root is the authoritative packaged-disc source that the builder copies directly into the final raw GameCube image artifact.

## Generated core seam

The native build consumes generated engine output from `HELENGINE_CORE_CPP_ROOT` when a generated core deployment root is provided.

## Generated core build

Generate GameCube-targeted core output:

```bash
rtk dotnet run --project /mnt/c/dev/helworks/csharpcodegen/codegen/codegen.csproj -- --cpp --project /mnt/c/dev/helworks/helengine/engine/helengine.core/helengine.core.csproj --output /mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube --platform gamecube --compiler gcc --endianness big --set generated-math-convention=native-column-vector --set pointer-size-bytes=4 --set "type-remaps=System.Numerics.Vector2=helengine.float2;System.Numerics.Vector3=helengine.float3;System.Numerics.Vector4=helengine.float4;System.Numerics.Quaternion=helengine.float4" --preset native-core-boot
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
