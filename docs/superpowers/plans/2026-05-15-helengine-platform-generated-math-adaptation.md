# Platform Generated Math Adaptation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move platform-specific matrix convention fixes out of the GameCube runtime and into a generic generated-output adaptation pass in `csharpcodegen`, then make `helengine-gc` consume the adapted matrices directly.

**Architecture:** Treat shared generated math as platform-authored output, not immutable engine law. Add a generic post-generation adaptation seam in `csharpcodegen` that can rewrite copied runtime math files for targets whose native graphics APIs use different matrix/vector conventions. Implement the first adapter for GameCube so generated `float4x4` output matches GX/libogc column-vector expectations, then simplify the GameCube renderer to load those matrices directly instead of rebuilding world/view math with `gu`.

**Tech Stack:** C#/.NET 9, Roslyn-based `csharpcodegen`, generated C++ runtime templates, C++17, libogc GX, Docker devkitPPC build via `rtk`, Dolphin manual verification

---

## File Map

- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\model\CPPPlatformProfile.cs`
  - Add generic platform math-adaptation metadata.
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\model\CPPGeneratedMathConventionKind.cs`
  - Define stable output math conventions for generated targets.
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOutputAdapter.cs`
  - Add the generic generated-output post-processing seam.
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGameCubeGeneratedMathAdapter.cs`
  - Implement the first platform-specific runtime-math rewrite over emitted files.
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPCodeConverter.cs`
  - Invoke the generated-output adapter during `WriteOutput(...)`.
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPGeneratedOutputAdapterTests.cs`
  - Cover the generic adapter dispatch behavior.
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPConversionOptionsTests.cs`
  - Assert GameCube default options expose the new math convention.
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPConversionPresetCatalogTests.cs`
  - Assert the GameCube preset resolves the new math convention correctly.
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPFeaturePruningEndToEndTests.cs`
  - Inspect emitted `float4x4.cpp` output for the GameCube preset.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneRenderBridge.cpp`
  - Keep frame-plan math on generated `float4x4`, not `gu` rebuilds.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`
  - Stop rebuilding runtime world/view matrices with `gu`; load adapted generated matrices directly into GX.

### Task 1: Add Generic Generated Math Convention Metadata

**Files:**
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\model\CPPGeneratedMathConventionKind.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\model\CPPPlatformProfile.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPConversionOptionsTests.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPConversionPresetCatalogTests.cs`

- [ ] **Step 1: Add the convention enum**

Create `C:\dev\helworks\csharpcodegen\cs2.cpp\model\CPPGeneratedMathConventionKind.cs`:

```csharp
namespace cs2.cpp {
    /// <summary>
    /// Describes the matrix/vector convention that generated runtime math should follow for one platform output.
    /// </summary>
    public enum CPPGeneratedMathConventionKind {
        EngineRowVector = 0,
        NativeColumnVector = 1
    }
}
```

- [ ] **Step 2: Extend the platform profile**

Update `C:\dev\helworks\csharpcodegen\cs2.cpp\model\CPPPlatformProfile.cs`:

```csharp
        /// <summary>
        /// Gets or sets the generated runtime math convention required by the target platform.
        /// </summary>
        public CPPGeneratedMathConventionKind GeneratedMathConvention { get; set; }
```

Set the defaults explicitly:

```csharp
                GeneratedMathConvention = CPPGeneratedMathConventionKind.EngineRowVector
```

for Windows, and:

```csharp
                GeneratedMathConvention = CPPGeneratedMathConventionKind.NativeColumnVector
```

for GameCube. Keep PlayStation 2 and Nintendo 64 on `EngineRowVector` until they get dedicated adapters later.

- [ ] **Step 3: Write the failing options test**

Add to `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPConversionOptionsTests.cs`:

```csharp
[Fact]
public void CreateGameCubeDefault_UsesNativeColumnVectorMathConvention() {
    CPPConversionOptions options = CPPConversionOptions.CreateGameCubeDefault();

    Assert.Equal(CPPGeneratedMathConventionKind.NativeColumnVector, options.PlatformProfile.GeneratedMathConvention);
}
```

- [ ] **Step 4: Run the options test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter FullyQualifiedName~CPPConversionOptionsTests.CreateGameCubeDefault_UsesNativeColumnVectorMathConvention -v minimal
```

Expected: FAIL because the new profile property does not exist yet or is not assigned.

- [ ] **Step 5: Write the failing preset test**

Add to `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPConversionPresetCatalogTests.cs`:

```csharp
[Fact]
public void Resolve_GameCubeCoreBoot_UsesNativeColumnVectorMathConvention() {
    CPPConversionPreset preset = new CPPConversionPresetCatalog().Resolve("gamecube-core-boot");

    Assert.Equal(CPPGeneratedMathConventionKind.NativeColumnVector, preset.PlatformProfile.GeneratedMathConvention);
}
```

- [ ] **Step 6: Run the preset test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter FullyQualifiedName~CPPConversionPresetCatalogTests.Resolve_GameCubeCoreBoot_UsesNativeColumnVectorMathConvention -v minimal
```

Expected: FAIL for the same reason.

- [ ] **Step 7: Implement the metadata assignments**

Apply the `CPPPlatformProfile` changes from steps 1 and 2, then rerun:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPConversionOptionsTests.CreateGameCubeDefault_UsesNativeColumnVectorMathConvention|FullyQualifiedName~CPPConversionPresetCatalogTests.Resolve_GameCubeCoreBoot_UsesNativeColumnVectorMathConvention" -v minimal
```

Expected: PASS

- [ ] **Step 8: Commit**

```bash
git -C C:/dev/helworks/csharpcodegen add cs2.cpp/model/CPPGeneratedMathConventionKind.cs cs2.cpp/model/CPPPlatformProfile.cs cs2.cpp.tests/CPPConversionOptionsTests.cs cs2.cpp.tests/CPPConversionPresetCatalogTests.cs
git -C C:/dev/helworks/csharpcodegen commit -m "Add generated math convention metadata"
```

### Task 2: Add the Generic Generated-Output Adaptation Seam

**Files:**
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOutputAdapter.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPCodeConverter.cs`
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPGeneratedOutputAdapterTests.cs`

- [ ] **Step 1: Write the failing adapter-dispatch test**

Create `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPGeneratedOutputAdapterTests.cs`:

```csharp
public class CPPGeneratedOutputAdapterTests {
    [Fact]
    public void Apply_ForGameCubeConvention_RewritesFloat4x4RuntimeFile() {
        string root = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        File.WriteAllText(Path.Combine(root, "float4x4.cpp"), "result.M41 = x;\nresult.M42 = y;\nresult.M43 = z;\n");

        CPPConversionOptions options = CPPConversionOptions.CreateGameCubeDefault();

        new CPPGeneratedOutputAdapter().Apply(root, options);

        string output = File.ReadAllText(Path.Combine(root, "float4x4.cpp"));
        Assert.Contains("result.M14 = x;", output);
        Assert.DoesNotContain("result.M41 = x;", output);
    }
}
```

- [ ] **Step 2: Run the dispatch test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter FullyQualifiedName~CPPGeneratedOutputAdapterTests.Apply_ForGameCubeConvention_RewritesFloat4x4RuntimeFile -v minimal
```

Expected: FAIL because `CPPGeneratedOutputAdapter` does not exist yet.

- [ ] **Step 3: Add the generic adapter entry point**

Create `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOutputAdapter.cs`:

```csharp
namespace cs2.cpp {
    /// <summary>
    /// Applies post-generation output rewrites required by one platform math convention.
    /// </summary>
    public sealed class CPPGeneratedOutputAdapter {
        /// <summary>
        /// Applies any required post-processing to one generated output root.
        /// </summary>
        public void Apply(string outputFolder, CPPConversionOptions options) {
            if (string.IsNullOrWhiteSpace(outputFolder)) {
                throw new ArgumentException("Output folder must not be empty.", nameof(outputFolder));
            } else if (options == null) {
                throw new ArgumentNullException(nameof(options));
            }

            if (options.PlatformProfile.GeneratedMathConvention == CPPGeneratedMathConventionKind.NativeColumnVector
                && options.PlatformProfile.Kind == CPPPlatformKind.GameCubeHeadless) {
                new CPPGameCubeGeneratedMathAdapter().Apply(outputFolder);
            }
        }
    }
}
```

- [ ] **Step 4: Wire the adapter into the codegen output pipeline**

Update `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPCodeConverter.cs` inside `WriteOutput(...)`:

```csharp
            CopyRuntimeFiles(new DirectoryInfo(rootPath), new DirectoryInfo(outputFolder), replacements);

            writeClasses(outputFolder, BuildUsageReport);
            new CPPGeneratedOutputAdapter().Apply(outputFolder, Options);
            PruneDisabledFeatureRuntimeFiles(outputFolder);
```

This order is important: the adapter must be able to rewrite copied runtime files and any generated files that future platforms may need to patch.

- [ ] **Step 5: Run the adapter test to verify it now fails in the right place**

Run:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter FullyQualifiedName~CPPGeneratedOutputAdapterTests.Apply_ForGameCubeConvention_RewritesFloat4x4RuntimeFile -v minimal
```

Expected: FAIL because `CPPGameCubeGeneratedMathAdapter` does not exist yet.

- [ ] **Step 6: Commit**

```bash
git -C C:/dev/helworks/csharpcodegen add cs2.cpp/CPPGeneratedOutputAdapter.cs cs2.cpp/CPPCodeConverter.cs cs2.cpp.tests/CPPGeneratedOutputAdapterTests.cs
git -C C:/dev/helworks/csharpcodegen commit -m "Add generated output adapter seam"
```

### Task 3: Implement the First GameCube Generated Math Adapter

**Files:**
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGameCubeGeneratedMathAdapter.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPGeneratedOutputAdapterTests.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPFeaturePruningEndToEndTests.cs`

- [ ] **Step 1: Extend the failing adapter test with the real rewrite targets**

Expand `CPPGeneratedOutputAdapterTests.Apply_ForGameCubeConvention_RewritesFloat4x4RuntimeFile` so the seed file contains all critical row-vector signatures:

```csharp
File.WriteAllText(
    Path.Combine(root, "float4x4.cpp"),
    string.Join(
        "\n",
        "result.M41 = x;",
        "result.M42 = y;",
        "result.M43 = z;",
        "float m41 = (((matrix1.M41 * matrix2.M11) + (matrix1.M42 * matrix2.M21)) + (matrix1.M43 * matrix2.M31)) + (matrix1.M44 * matrix2.M41);",
        "ret.M14 = matrix.M41;",
        "ret.M41 = matrix.M14;"
    ));
```

Assert for GameCube-native replacements:

```csharp
Assert.Contains("result.M14 = x;", output);
Assert.Contains("result.M24 = y;", output);
Assert.Contains("result.M34 = z;", output);
Assert.Contains("float m14 = (((matrix1.M11 * matrix2.M14) + (matrix1.M12 * matrix2.M24)) + (matrix1.M13 * matrix2.M34)) + matrix1.M14;", output);
Assert.DoesNotContain("result.M41 = x;", output);
```

- [ ] **Step 2: Implement the GameCube adapter**

Create `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGameCubeGeneratedMathAdapter.cs`:

```csharp
namespace cs2.cpp {
    /// <summary>
    /// Rewrites emitted runtime math files so GameCube output matches GX/libogc column-vector matrix conventions.
    /// </summary>
    public sealed class CPPGameCubeGeneratedMathAdapter {
        public void Apply(string outputFolder) {
            RewriteFloat4x4(Path.Combine(outputFolder, "float4x4.cpp"));
        }

        static void RewriteFloat4x4(string sourcePath) {
            string source = File.ReadAllText(sourcePath);
            source = source.Replace("result.M41 = x;", "result.M14 = x;");
            source = source.Replace("result.M42 = y;", "result.M24 = y;");
            source = source.Replace("result.M43 = z;", "result.M34 = z;");
            source = source.Replace("result.M41 = position.X;", "result.M14 = position.X;");
            source = source.Replace("result.M42 = position.Y;", "result.M24 = position.Y;");
            source = source.Replace("result.M43 = position.Z;", "result.M34 = position.Z;");
            source = source.Replace("ret.M14 = matrix.M41;", "ret.M14 = matrix.M14;");
            source = source.Replace("ret.M24 = matrix.M42;", "ret.M24 = matrix.M24;");
            source = source.Replace("ret.M34 = matrix.M43;", "ret.M34 = matrix.M34;");
            source = source.Replace("ret.M41 = matrix.M14;", "ret.M41 = matrix.M41;");
            source = source.Replace("ret.M42 = matrix.M24;", "ret.M42 = matrix.M42;");
            source = source.Replace("ret.M43 = matrix.M34;", "ret.M43 = matrix.M43;");

            // Replace the row-vector multiply block with column-vector multiply semantics.
            source = RewriteMultiplyBlock(source);

            File.WriteAllText(sourcePath, source);
        }
    }
}
```

The implementation detail is important: keep the adapter file-based and generated-output-local. Do not mutate the source `.net.cpp` templates in place, and do not move this logic into `helengine-gc`.

- [ ] **Step 3: Run the focused adapter test**

Run:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter FullyQualifiedName~CPPGeneratedOutputAdapterTests.Apply_ForGameCubeConvention_RewritesFloat4x4RuntimeFile -v minimal
```

Expected: PASS

- [ ] **Step 4: Add the end-to-end emitted-file regression**

Add to `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPFeaturePruningEndToEndTests.cs`:

```csharp
[Fact]
public void WriteOutput_WhenPresetIsGameCubeCoreBoot_RewritesFloat4x4ForNativeColumnVectorMath() {
    string source = "public class Sample { }";
    string outputPath = RunConversionWithPreset(source, "gamecube-core-boot");

    string float4x4Source = File.ReadAllText(Path.Combine(outputPath, "float4x4.cpp"));
    Assert.Contains("result.M14 = x;", float4x4Source);
    Assert.Contains("result.M24 = y;", float4x4Source);
    Assert.Contains("result.M34 = z;", float4x4Source);
    Assert.DoesNotContain("result.M41 = x;", float4x4Source);
}
```

- [ ] **Step 5: Run the end-to-end GameCube output test**

Run:

```powershell
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter FullyQualifiedName~CPPFeaturePruningEndToEndTests.WriteOutput_WhenPresetIsGameCubeCoreBoot_RewritesFloat4x4ForNativeColumnVectorMath -v minimal
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git -C C:/dev/helworks/csharpcodegen add cs2.cpp/CPPGameCubeGeneratedMathAdapter.cs cs2.cpp.tests/CPPGeneratedOutputAdapterTests.cs cs2.cpp.tests/CPPFeaturePruningEndToEndTests.cs
git -C C:/dev/helworks/csharpcodegen commit -m "Add GameCube generated math adapter"
```

### Task 4: Switch GameCube Rendering to Adapted Generated Matrices

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneRenderBridge.cpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`

- [ ] **Step 1: Regenerate the GameCube core with the adapted preset**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\csharpcodegen\codegen\codegen.csproj -- --cpp --project C:\dev\helworks\helengine\engine\helengine.core\helengine.core.csproj --output C:\dev\helworks\helengine-gc\tmp\generated-core-gamecube --platform gamecube --compiler gcc --endianness big --preset gamecube-core-boot
```

Expected:
- `tmp\generated-core-gamecube\float4x4.cpp` is regenerated
- The emitted file contains `M14/M24/M34` translation assignments

- [ ] **Step 2: Stop rebuilding camera matrices with `gu` in the renderer**

Update `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp` so it consumes `framePlan->View` and `framePlan->Projection` instead of rebuilding view/projection with `guLookAt` and `guPerspective`:

```cpp
static void CopyGeneratedMatrixToGx(const float4x4& source, Mtx& destination) {
    destination[0][0] = source.M11;
    destination[0][1] = source.M12;
    destination[0][2] = source.M13;
    destination[0][3] = source.M14;
    destination[1][0] = source.M21;
    destination[1][1] = source.M22;
    destination[1][2] = source.M23;
    destination[1][3] = source.M24;
    destination[2][0] = source.M31;
    destination[2][1] = source.M32;
    destination[2][2] = source.M33;
    destination[2][3] = source.M34;
}
```

Use this as a raw layout copy only. Do not transpose, rebuild, or reinterpret the matrix on GameCube.

- [ ] **Step 3: Build entity world matrices with generated `float4x4`**

Replace the `gu` world path in `GameCubeRasterRenderer.cpp` with generated-math composition:

```cpp
float4x4 scale;
float4x4::CreateScale(entityScale.X, entityScale.Y, entityScale.Z, scale);

float4 normalizedOrientation = entity->get_Orientation();
normalizedOrientation.Normalize();
float4x4 rotation;
float4x4::CreateFromQuaternion(normalizedOrientation, rotation);

float4x4 translation;
float4x4::CreateTranslation(entityPosition, translation);

float4x4 world;
float4x4 scaledRotation;
float4x4::Multiply(scale, rotation, scaledRotation);
float4x4::Multiply(scaledRotation, translation, world);

float4x4 modelView;
float4x4::Multiply(world, framePlan->View, modelView);
```

Then load `modelView` through the raw `CopyGeneratedMatrixToGx(...)` helper and submit local mesh vertices directly.

- [ ] **Step 4: Remove the mixed-convention probe path**

Delete the temporary `guWorld0`, `guView0`, and `altGuView0` compare logic once the adapted generated path is in place. Keep one short startup log only if needed:

```cpp
SYS_Report("[GC] Using generated platform-adapted float4x4 matrices for GX submission.\n");
```

- [ ] **Step 5: Rebuild the GameCube binary and repack the retail image**

Run:

```powershell
rtk docker run --rm -v C:/dev/helworks/helengine-gc:/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube -e HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc -e HELENGINE_GAMECUBE_MINIMAL_SAMPLE=1 helengine-gc make clean packaged-disc-assets all
rtk dotnet run --project tmp\builder-retail-check\builder-retail-check.csproj -v minimal
Copy-Item 'C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm' 'C:\dev\helworks\emus\gc-probe\game.gcm' -Force
```

Expected:
- native build succeeds
- builder reports `ISO/GC`
- copied probe image is fresh

- [ ] **Step 6: Launch Dolphin and verify the adapted path**

Run:

```powershell
Get-Process -Name 'Dolphin*' -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Process -FilePath 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe' -WorkingDirectory 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64' -ArgumentList @('-e','C:\dev\helworks\emus\gc-probe\game.gcm')
```

Expected:
- the rotating cube remains rectangular under rotation
- no trapezoid expansion or depth-axis drift
- no runtime `gu` matrix compare probes are needed anymore

- [ ] **Step 7: Commit**

```bash
git -C C:/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeSceneRenderBridge.cpp src/platform/gamecube/GameCubeRasterRenderer.cpp
git -C C:/dev/helworks/helengine-gc commit -m "Use adapted generated matrices for GC GX submission"
```

## Self-Review

- Spec coverage: the plan moves platform math adaptation into `csharpcodegen`, keeps it generic through `CPPPlatformProfile.GeneratedMathConvention`, and uses GameCube as the first concrete adapter while preserving room for later PS2/N64 adapters.
- Placeholder scan: no `TODO`/`TBD` markers remain; every task names concrete files, commands, and expected outcomes.
- Type consistency: the plan uses one consistent seam name (`CPPGeneratedOutputAdapter`) and one consistent metadata name (`CPPGeneratedMathConventionKind`).
