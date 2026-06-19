# GameCube Docker Build

Use the local Docker image when you need to build the GameCube host directly.

```bash
docker build -t helengine-gc .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

The build emits `build/helengine_gc.dol`.

Generate GameCube-targeted core output:

```bash
rtk dotnet run --project /mnt/c/dev/helworks/csharpcodegen/codegen/codegen.csproj -- --cpp --project /mnt/c/dev/helworks/helengine/engine/helengine.core/helengine.core.csproj --output /mnt/c/dev/helworks/helengine-gc/tmp/generated-core-gamecube --platform gamecube --compiler gcc --endianness big --set generated-math-convention=native-column-vector --set pointer-size-bytes=4 --set "type-remaps=System.Numerics.Vector2=helengine.float2;System.Numerics.Vector3=helengine.float3;System.Numerics.Vector4=helengine.float4;System.Numerics.Quaternion=helengine.float4" --preset native-core-boot
```

Build the GameCube player with generated core enabled:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all
```
