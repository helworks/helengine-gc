# GameCube White Shared Lighting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render the packaged rotating cube through the real scene pipeline with shared-engine lit/unlit material selection, using forced white ambient plus directional diffuse lighting on GameCube.

**Architecture:** Keep the current packaged scene, mesh, and cooked platform material path intact. Extend the GameCube frame/runtime model plumbing so the raster renderer receives extracted light submissions and authored normals, then add a first lit branch in the GX renderer that computes forced-white ambient plus directional diffuse color while preserving GPU-driven transform and rasterization.

**Tech Stack:** C#/.NET 9 builder tests, generated C++ runtime, C++17, libogc GX, Docker devkitPPC build via `rtk`, Dolphin manual verification

---

## File Map

- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeFramePlan.hpp`
  - Carry extracted light submissions alongside drawable submissions.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneRenderBridge.cpp`
  - Preserve extracted light submissions in the frame plan instead of discarding them.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeModel.hpp`
  - Carry authored normal arrays for the lit branch.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp`
  - Populate runtime-model normals and preserve shared lit/unlit material mapping.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.hpp`
  - Add explicit lit-branch helpers for light accumulation and vertex color evaluation.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`
  - Implement ambient plus directional diffuse forced-white shading and route lit/unlit submissions correctly.
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`
  - Guard the new light-plumbing and lit-branch source contract.

### Task 1: Plumb Extracted Lights And Normals Into The GameCube Frame

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeFramePlan.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneRenderBridge.cpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeModel.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp`
- Test: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-contract test for light and normal plumbing**

Add to `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`:

```csharp
[Fact]
public void PackagedDiscBootSource_PreservesExtractedLightsAndNormalsForLitRendering() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string framePlanSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeFramePlan.hpp"));
    string sceneRenderBridgeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.cpp"));
    string runtimeModelSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeModel.hpp"));
    string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));

    Assert.Contains("List<RenderFrameLightSubmission*>* LightSubmissions;", framePlanSource, StringComparison.Ordinal);
    Assert.Contains("frame->get_LightSubmissions()", sceneRenderBridgeSource, StringComparison.Ordinal);
    Assert.Contains("Array<float3>* Normals;", runtimeModelSource, StringComparison.Ordinal);
    Assert.Contains("runtimeModel->Normals = data->Normals;", renderManagerSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedDiscBootSource_PreservesExtractedLightsAndNormalsForLitRendering -v minimal
```

Expected: FAIL because `GameCubeFramePlan`, `GameCubeSceneRenderBridge`, and `GameCubeRuntimeModel` do not yet preserve those values.

- [ ] **Step 3: Extend the GameCube frame plan to carry extracted lights**

Update `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeFramePlan.hpp`:

```cpp
#include "RenderFrameLightSubmission.hpp"
```

and change the constructor and members to:

```cpp
        GameCubeFramePlan(
            CameraComponent* camera,
            List<RenderFrameDrawableSubmission*>* drawableSubmissions,
            List<RenderFrameLightSubmission*>* lightSubmissions,
            float4 viewport,
            float4x4 view,
            float4x4 projection,
            float4x4 viewProjection)
            : Camera(camera)
            , DrawableSubmissions(drawableSubmissions)
            , LightSubmissions(lightSubmissions)
            , Viewport(viewport)
            , View(view)
            , Projection(projection)
            , ViewProjection(viewProjection) {
        }

        /// Light submissions extracted from the generated runtime graph for this frame.
        List<RenderFrameLightSubmission*>* LightSubmissions;
```

- [ ] **Step 4: Preserve extracted light submissions when building the frame plan**

Update `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeSceneRenderBridge.cpp`:

```cpp
        List<LightComponent*>* lights = new List<LightComponent*>();

        RenderFrameExtractionService extractor;
        RenderFrameExtractionResult* extraction = extractor.Extract(cameras, drawables, lights, capabilities);
        RenderFrame* frame = (*extraction->get_Frames())[0];
```

and change the return to:

```cpp
        return new GameCubeFramePlan(
            camera,
            frame->get_DrawableSubmissions(),
            frame->get_LightSubmissions(),
            viewport,
            view,
            projection,
            viewProjection);
```

- [ ] **Step 5: Extend the GameCube runtime model with authored normals**

Update `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeModel.hpp`:

```cpp
            , Normals(nullptr)
            , Indices16(nullptr)
```

and add:

```cpp
        /// Authored model normals used by the first lit GameCube path.
        Array<float3>* Normals;
```

- [ ] **Step 6: Populate runtime-model normals during model build**

Update `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp` inside `BuildModelFromRaw(...)`:

```cpp
        runtimeModel->Positions = data->Positions;
        runtimeModel->Normals = data->Normals;
        runtimeModel->Indices16 = indexData->get_Indices16();
```

- [ ] **Step 7: Run the focused source-contract test to verify it passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedDiscBootSource_PreservesExtractedLightsAndNormalsForLitRendering -v minimal
```

Expected: PASS

- [ ] **Step 8: Commit**

```bash
git -C C:/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeFramePlan.hpp src/platform/gamecube/GameCubeSceneRenderBridge.cpp src/platform/gamecube/GameCubeRuntimeModel.hpp src/platform/gamecube/GameCubeRenderManager3D.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
git -C C:/dev/helworks/helengine-gc commit -m "Plumb GameCube lights and normals into frame plan"
```

### Task 2: Add The Shared Lit Branch To The GX Raster Path

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-contract test for the lit branch**

Add to `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`:

```csharp
[Fact]
public void PackagedDiscBootSource_UsesSharedLightingModelForWhiteDirectionalDiffuseRendering() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
    string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

    Assert.Contains("RuntimeMaterialLightingModel", rasterRendererSource, StringComparison.Ordinal);
    Assert.Contains("EvaluateLitVertexColor(", rasterRendererHeaderSource, StringComparison.Ordinal);
    Assert.Contains("AccumulateAmbientAndDirectionalLight(", rasterRendererHeaderSource, StringComparison.Ordinal);
    Assert.Contains("submission->get_Drawable()->get_Material()", rasterRendererSource, StringComparison.Ordinal);
    Assert.Contains("RuntimeMaterialLightingModel::Unlit", rasterRendererSource, StringComparison.Ordinal);
    Assert.Contains("RuntimeMaterialLightingModel::MetalRoughPbr", rasterRendererSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedDiscBootSource_UsesSharedLightingModelForWhiteDirectionalDiffuseRendering -v minimal
```

Expected: FAIL because the current raster path does not inspect the material lighting model or expose lit helpers.

- [ ] **Step 3: Add lit-branch helper declarations**

Update `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.hpp`:

```cpp
class RenderFrameDrawableSubmission;
class RenderFrameLightSubmission;
class RuntimeMaterial;
```

and add:

```cpp
        /// Resolves whether one submission should use the lit branch for the current checkpoint.
        bool UsesLitBranch(RenderFrameDrawableSubmission* submission);

        /// Accumulates ambient plus directional light into a white diffuse lighting result.
        float3 AccumulateAmbientAndDirectionalLight(GameCubeFramePlan* framePlan, Entity* entity, float3 normal);

        /// Evaluates one final GX vertex color for the first lit branch.
        GXColor EvaluateLitVertexColor(GameCubeFramePlan* framePlan, Entity* entity, float3 normal);

        /// Converts a normalized RGB lighting value into a GX color with full alpha.
        GXColor ConvertLightingColorToGx(float3 color);
```

- [ ] **Step 4: Route draw submissions through lit vs unlit material selection**

Update the draw loop in `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`:

```cpp
            RenderFrameDrawableSubmission* submission = (*framePlan->DrawableSubmissions)[submissionIndex];
            GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());
            RuntimeSubmesh* runtimeSubmesh = (*runtimeModel->get_Submeshes())[submission->get_SubmeshIndex()];
            Entity* entity = submission->get_Drawable()->get_Parent();

            DrawSubmesh(framePlan, submission, runtimeModel, runtimeSubmesh, entity);
```

and change the signature in both header and source to:

```cpp
void DrawSubmesh(GameCubeFramePlan* framePlan, RenderFrameDrawableSubmission* submission, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity);
```

- [ ] **Step 5: Implement the first lit white ambient-plus-directional branch**

Add to `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`:

```cpp
    bool GameCubeRasterRenderer::UsesLitBranch(RenderFrameDrawableSubmission* submission) {
        if (submission == nullptr) {
            throw new ArgumentNullException("submission");
        }

        RuntimeMaterial* material = submission->get_Drawable()->get_Material();
        if (material == nullptr) {
            throw new InvalidOperationException("GameCube drawable submission requires a runtime material.");
        }

        return material->get_LightingModel() == RuntimeMaterialLightingModel::MetalRoughPbr;
    }
```

```cpp
    float3 GameCubeRasterRenderer::AccumulateAmbientAndDirectionalLight(GameCubeFramePlan* framePlan, Entity* entity, float3 normal) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        float3 accumulated = float3(0.0f, 0.0f, 0.0f);
        float3 normalizedNormal = float3::Normalize(normal);
        for (int32_t lightIndex = 0; lightIndex < framePlan->LightSubmissions->get_Count(); lightIndex++) {
            RenderFrameLightSubmission* submission = (*framePlan->LightSubmissions)[lightIndex];
            LightComponent* light = submission->get_Light();
            float4 color = light->get_Color();
            float3 rgb = float3(color.X, color.Y, color.Z) * light->get_Intensity();

            if (submission->get_LightType() == LightType::Ambient) {
                accumulated = accumulated + rgb;
                continue;
            }

            if (submission->get_LightType() == LightType::Directional) {
                Entity* lightEntity = light->get_Parent();
                if (lightEntity == nullptr) {
                    continue;
                }

                float3 lightDirection = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), lightEntity->get_Orientation());
                lightDirection = float3::Normalize(lightDirection * -1.0f);
                float diffuse = std::max(0.0f, float3::Dot(normalizedNormal, lightDirection));
                accumulated = accumulated + (rgb * diffuse);
            }
        }

        if (accumulated.X <= 0.0f && accumulated.Y <= 0.0f && accumulated.Z <= 0.0f) {
            return float3(0.15f, 0.15f, 0.15f);
        }

        return accumulated;
    }
```

```cpp
    GXColor GameCubeRasterRenderer::EvaluateLitVertexColor(GameCubeFramePlan* framePlan, Entity* entity, float3 normal) {
        const float3 lighting = AccumulateAmbientAndDirectionalLight(framePlan, entity, normal);
        const float3 whiteShaded = float3(
            std::min(1.0f, lighting.X),
            std::min(1.0f, lighting.Y),
            std::min(1.0f, lighting.Z));
        return ConvertLightingColorToGx(whiteShaded);
    }
```

```cpp
    GXColor GameCubeRasterRenderer::ConvertLightingColorToGx(float3 color) {
        return GXColor {
            static_cast<u8>(std::clamp(color.X, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(std::clamp(color.Y, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(std::clamp(color.Z, 0.0f, 1.0f) * 255.0f),
            0xFF
        };
    }
```

- [ ] **Step 6: Use authored normals for lit submissions while preserving the unlit path**

Inside `DrawSubmesh(...)` in `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`, replace the hardcoded yellow color emission with:

```cpp
        const bool useLitBranch = UsesLitBranch(submission);

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, runtimeSubmesh->get_IndexCount());
        for (int32_t indexOffset = 0; indexOffset < runtimeSubmesh->get_IndexCount(); indexOffset++) {
            const uint32_t positionIndex = runtimeModel->Uses32BitIndices
                ? (*runtimeModel->Indices32)[runtimeSubmesh->get_IndexStart() + indexOffset]
                : (*runtimeModel->Indices16)[runtimeSubmesh->get_IndexStart() + indexOffset];
            const float3 localPosition = (*runtimeModel->Positions)[positionIndex];
            GX_Position3f32(localPosition.X, localPosition.Y, localPosition.Z);

            if (useLitBranch) {
                if (runtimeModel->Normals == nullptr) {
                    throw new InvalidOperationException("GameCube lit rendering requires authored mesh normals.");
                }

                const float3 localNormal = (*runtimeModel->Normals)[positionIndex];
                GXColor litColor = EvaluateLitVertexColor(framePlan, entity, localNormal);
                GX_Color4u8(litColor.r, litColor.g, litColor.b, litColor.a);
            } else {
                GX_Color4u8(OpaqueMeshColorRed, OpaqueMeshColorGreen, OpaqueMeshColorBlue, OpaqueMeshColorAlpha);
            }
        }
        GX_End();
```

- [ ] **Step 7: Run the focused source-contract test to verify it passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedDiscBootSource_UsesSharedLightingModelForWhiteDirectionalDiffuseRendering -v minimal
```

Expected: PASS

- [ ] **Step 8: Commit**

```bash
git -C C:/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
git -C C:/dev/helworks/helengine-gc commit -m "Add first shared white lighting branch for GameCube"
```

### Task 3: Verify The Packaged White Lighting Checkpoint End-To-End

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRasterRenderer.cpp`
- Test: packaged runtime through Docker build and Dolphin

- [ ] **Step 1: Keep the shared runtime material contract explicit**

Confirm `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager3D.cpp` still maps cooked materials like this:

```cpp
        runtimeMaterial->set_LightingModel(materialAsset->Lit
            ? RuntimeMaterialLightingModel::MetalRoughPbr
            : RuntimeMaterialLightingModel::Unlit);
```

Do not replace this with a GC-specific enum or probe-only lighting mode.

- [ ] **Step 2: Run the focused builder test slice**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePlatformAssetBuilderTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal
```

Expected: PASS

- [ ] **Step 3: Regenerate the active GameCube core if needed**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\csharpcodegen\codegen\codegen.csproj -- --cpp --project C:\dev\helworks\helengine\engine\helengine.core\helengine.core.csproj --output C:\dev\helworks\helengine-gc\tmp\generated-core-gamecube --platform gamecube --compiler gcc --endianness big --preset gamecube-core-boot
```

Expected: `C++ conversion completed.`

- [ ] **Step 4: Refresh cooked platform materials and scene manifest**

Run:

```powershell
rtk dotnet run --project .\tmp\builder-retail-check\builder-retail-check.csproj -- normalize-and-recook-material
rtk dotnet run --project .\tmp\builder-retail-check\builder-retail-check.csproj -- prepare-manifest
```

Expected: `standard.hasset` is recooked as `helengine.PlatformMaterialAsset` and `runtime\gamecube_runtime_scene_manifest.hpp` is restored.

- [ ] **Step 5: Rebuild the native packaged-disc target**

Run:

```powershell
rtk docker run --rm -v C:/dev/helworks/helengine-gc:/workspace -w /workspace helengine-gc sh -lc 'export HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube; export HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc; export HELENGINE_GAMECUBE_MINIMAL_SAMPLE=0; make clean all packaged-disc-assets'
```

Expected: build completes without `error:` lines and emits `build/helengine_gc.dol`.

- [ ] **Step 6: Repackage the `.gcm`**

Run:

```powershell
rtk dotnet run --project .\tmp\builder-retail-check\builder-retail-check.csproj -- package
```

Expected: reports `ISO/GC` and writes `C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm`

- [ ] **Step 7: Launch Dolphin with the rebuilt image**

Run:

```powershell
$source = 'C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm'
$targetDir = 'C:\dev\helworks\emus\gc-probe'
$target = Join-Path $targetDir 'game.gcm'
New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
Copy-Item -LiteralPath $source -Destination $target -Force
Start-Process -FilePath 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe' -WorkingDirectory 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64' -ArgumentList @('-e', $target)
```

Expected: Dolphin launches the packaged scene.

- [ ] **Step 8: Manually verify the visible result**

Check:

- the cube still rotates
- the cube is intentionally white
- the white shading changes with ambient plus directional light instead of staying flat
- no return of disappearing geometry, scale explosions, or matrix flicker

- [ ] **Step 9: Commit**

```bash
git -C C:/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeRenderManager3D.cpp src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeFramePlan.hpp src/platform/gamecube/GameCubeSceneRenderBridge.cpp src/platform/gamecube/GameCubeRuntimeModel.hpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
git -C C:/dev/helworks/helengine-gc commit -m "Render GameCube cube with first shared white lighting"
```
