# GameCube Cube Test Renderer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render the authored `city` `cube_test` scene on GameCube through the generated-core runtime path, with the cube visibly rotating in Dolphin through the real update and draw loop.

**Architecture:** Keep the generated `RenderManager3D` seam, but replace the placeholder bridge with four local pieces: deterministic scene bootstrap, strict frame extraction, backend-local mesh validation, and a narrow GX raster path. The first backend tier stays intentionally small: one camera, opaque mesh submissions, authored transforms, perspective projection, depth, and an unlit constant-color material path that is sufficient to show the rotating cube.

**Tech Stack:** C++17, generated Helengine core, devkitPro, libogc, GX, Docker, Dolphin

---

## File Structure

- `/mnt/c/dev/helworks/helengine-gc/Makefile`
  Extends the native GameCube build so the new renderer source files are compiled.
- `/mnt/c/dev/helworks/helengine-gc/tools/stage_city_cube_test_content.sh`
  Stages the minimal cooked `city` content bundle required by the first GameCube renderer milestone.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootPhase.hpp`
  Declares the extra scene/bootstrap/render phases used for strict runtime diagnostics.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.hpp`
  Owns host bootstrap, generated-core initialization, and startup scene loading.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
  Wires content root, runtime scene catalog, startup scene load, and steady-state frame presentation.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneBootstrap.hpp`
  Declares the canonical `cube_test` scene id, content-root path, and scene-catalog construction for the GameCube host.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneBootstrap.cpp`
  Validates the staged content bundle and builds the single-scene `RuntimeSceneCatalog`.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRuntimeModel.hpp`
  Extends `RuntimeModel` with the geometry arrays the GameCube backend needs to draw authored model content.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeFramePlan.hpp`
  Owns the reduced per-camera execution payload for the first GameCube renderer tier.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderQueueSnapshotVisitor.hpp`
  Snapshots a generated `IRenderQueue3D` into a backend-local list without GX knowledge.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneRenderBridge.hpp`
  Declares runtime-to-frame-plan extraction for the GameCube backend.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneRenderBridge.cpp`
  Resolves the active camera, snapshots visible drawables, rejects unsupported submissions, and builds the canonical first-tier frame plan.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeMeshCache.hpp`
  Declares backend-local validation and reuse of `GameCubeRuntimeModel` geometry.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeMeshCache.cpp`
  Validates runtime models once and reuses them across frames by runtime id.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRasterRenderer.hpp`
  Declares GX setup and draw execution for the first opaque unlit path.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRasterRenderer.cpp`
  Clears the camera target, loads projection/modelview matrices, enables depth, and emits indexed GX triangle draws.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.hpp`
  Declares the orchestrating 3D backend seam and owns the bridge/cache/raster collaborators.
- `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.cpp`
  Builds runtime materials/models from authored content and delegates frame extraction plus GX execution.
- `/mnt/c/dev/helworks/helengine-gc/README.md`
  Documents staged content, build, and Dolphin launch commands for the rotating `cube_test` milestone.

### Task 1: Stage the `cube_test` content bundle and boot the authored scene

**Files:**
- Create: `/mnt/c/dev/helworks/helengine-gc/tools/stage_city_cube_test_content.sh`
- Modify: `/mnt/c/dev/helworks/helengine-gc/Makefile`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeBootPhase.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneBootstrap.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneBootstrap.cpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
- Test: `/mnt/c/dev/helworks/helengine-gc/tools/stage_city_cube_test_content.sh`

- [ ] **Step 1: Create the deterministic content staging script**

```bash
#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/city/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-cube-test-content/cooked"

mkdir -p \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/engine/models" \
    "${STAGE_ROOT}/engine/materials" \
    "${STAGE_ROOT}/shaders"

cp "${CITY_COOKED_ROOT}/scenes/rendering/cube_test.hasset" "${STAGE_ROOT}/scenes/rendering/cube_test.hasset"
cp "${CITY_COOKED_ROOT}/engine/models/cube.hasset" "${STAGE_ROOT}/engine/models/cube.hasset"
cp "${CITY_COOKED_ROOT}/engine/materials/standard.hasset" "${STAGE_ROOT}/engine/materials/standard.hasset"
cp "${CITY_COOKED_ROOT}/shaders/ForwardStandardShader.dx11.hasset" "${STAGE_ROOT}/shaders/ForwardStandardShader.dx11.hasset"

printf 'Staged GameCube cube-test content under %s\n' "${STAGE_ROOT}"
```

- [ ] **Step 2: Run the staging script and verify the first content bundle exists**

Run: `rtk bash tools/stage_city_cube_test_content.sh`

Expected: PASS and print `Staged GameCube cube-test content under /mnt/c/dev/helworks/helengine-gc/tmp/city-cube-test-content/cooked`.

- [ ] **Step 3: Extend the Makefile source list for the new bootstrap file**

```make
GENERATED_BRIDGE_SOURCES := \
	$(SOURCE_DIR)/platform/gamecube/GameCubeInputManager.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager2D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager3D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneBootstrap.cpp
```

- [ ] **Step 4: Add strict scene-bootstrap metadata**

```cpp
#pragma once

#include "runtime/native_string.hpp"

class RuntimeSceneCatalog;

namespace helengine::gamecube {
    /// Declares the authored startup scene and staged content root used by the first GameCube 3D renderer milestone.
    class GameCubeSceneBootstrap {
    public:
        /// Relative repo path that must contain the staged cooked content bundle before Dolphin verification.
        static std::string BundledContentRootPath;

        /// Stable scene id authored by `city` and expected by the generated runtime scene catalog.
        static std::string StartupSceneId;

        /// Returns the staged content root and fails if the bundle has not been prepared.
        static std::string GetValidatedContentRootPath();

        /// Creates the single-scene runtime catalog used by the first GameCube renderer milestone.
        static RuntimeSceneCatalog* CreateSceneCatalog();
    private:
        /// Verifies one required staged content file exists under the bundle root.
        static void ValidateRequiredFile(std::string rootPath, std::string relativePath);
    };
}
```

```cpp
#include "platform/gamecube/GameCubeSceneBootstrap.hpp"

#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/file.hpp"
#include "system/io/path.hpp"

namespace helengine::gamecube {
    std::string GameCubeSceneBootstrap::BundledContentRootPath = "tmp/city-cube-test-content";
    std::string GameCubeSceneBootstrap::StartupSceneId = "scenes/rendering/cube_test.helen";

    std::string GameCubeSceneBootstrap::GetValidatedContentRootPath() {
        const std::string rootPath = Path::GetFullPath(BundledContentRootPath);
        ValidateRequiredFile(rootPath, "cooked/scenes/rendering/cube_test.hasset");
        ValidateRequiredFile(rootPath, "cooked/engine/models/cube.hasset");
        ValidateRequiredFile(rootPath, "cooked/engine/materials/standard.hasset");
        ValidateRequiredFile(rootPath, "cooked/shaders/ForwardStandardShader.dx11.hasset");
        return rootPath;
    }

    RuntimeSceneCatalog* GameCubeSceneBootstrap::CreateSceneCatalog() {
        Array<RuntimeSceneCatalogEntry*>* entries = new Array<RuntimeSceneCatalogEntry*>(1);
        (*entries)[0] = new RuntimeSceneCatalogEntry(
            StartupSceneId,
            "cooked/scenes/rendering/cube_test.hasset");
        return new RuntimeSceneCatalog(entries);
    }

    void GameCubeSceneBootstrap::ValidateRequiredFile(std::string rootPath, std::string relativePath) {
        const std::string fullPath = Path::GetFullPath(Path::Combine(rootPath, relativePath));
        if (!File::Exists(fullPath)) {
            throw new InvalidOperationException(std::string("Required staged GameCube content file is missing: ") + fullPath);
        }
    }
}
```

- [ ] **Step 5: Add scene-bootstrap and scene-load phases to the host diagnostics**

```cpp
enum class GameCubeBootPhase {
    NativeVideo,
    NativeGraphics,
    CoreConstruction,
    CoreOptions,
    BridgeConstruction,
    CoreInitialization,
    SceneBootstrap,
    SceneLoad,
    CoreUpdate,
    CoreDraw,
    Running,
    Failed
};
```

- [ ] **Step 6: Wire the staged content root and startup scene load into `GameCubeApplication.cpp`**

```cpp
#include "platform/gamecube/GameCubeSceneBootstrap.hpp"

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
#include "SceneLoadMode.hpp"
#endif
```

```cpp
            SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x80, 0x00, 0xFF });
            CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
            if (options == nullptr) {
                FailBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                return false;
            }

            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0x40, 0x80, 0xFF });
            options->ContentRootPath = GameCubeSceneBootstrap::GetValidatedContentRootPath();
            options->SceneCatalog = GameCubeSceneBootstrap::CreateSceneCatalog();
            options->UpdateOrderLayers = 4;
            options->RenderOrderLayers3D = 4;
            options->UpdateListInitialCapacity = 64;
            options->RenderList2DInitialCapacity = 64;
            options->RenderList3DInitialCapacity = 64;
```

```cpp
            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0x00, 0x00, 0xFF, 0xFF });
            EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);
            EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, options);

            SetBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0x40, 0x40, 0xFF, 0xFF });
            EngineCore->get_SceneManager()->LoadScene(GameCubeSceneBootstrap::StartupSceneId, SceneLoadMode::Single);

            EngineInitialized = true;
```

- [ ] **Step 7: Rebuild after the bootstrap work**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all`

Expected: PASS and emit `build/helengine_gc.dol`.

- [ ] **Step 8: Launch Dolphin from the repo root to verify the authored scene path is now exercised**

Run: `rtk cmd.exe /c "cd /d C:\dev\helworks\helengine-gc && start \"\" \"C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe\" \"C:\dev\helworks\helengine-gc\build\helengine_gc.dol\""`

Expected: the build boots past native startup, reaches the generated startup scene load, and then fails later in rendering because `GameCubeRenderManager3D` still returns placeholder runtime models and no GX draw path exists yet.

- [ ] **Step 9: Commit the staged-content bootstrap**

```bash
git add Makefile tools/stage_city_cube_test_content.sh src/platform/gamecube/GameCubeBootPhase.hpp src/platform/gamecube/GameCubeSceneBootstrap.hpp src/platform/gamecube/GameCubeSceneBootstrap.cpp src/platform/gamecube/GameCubeApplication.cpp
git commit -m "Stage cube test content and boot authored scene"
```

### Task 2: Replace the placeholder 3D bridge with real runtime model and material loading

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRuntimeModel.hpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.hpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.cpp`
- Test: `/mnt/c/dev/helworks/helengine-gc/Makefile`

- [ ] **Step 1: Extend `GameCubeRuntimeModel` so authored model geometry survives runtime loading**

```cpp
#pragma once

#include "RuntimeModel.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"

namespace helengine::gamecube {
    /// Stores the authored model geometry arrays needed by the first GameCube GX backend tier.
    class GameCubeRuntimeModel : public RuntimeModel {
    public:
        /// Creates an empty GameCube runtime model before authored arrays are attached.
        GameCubeRuntimeModel()
            : RuntimeModel()
            , Positions(nullptr)
            , Indices16(nullptr)
            , Indices32(nullptr)
            , Uses32BitIndices(false) {
        }

        /// Authored model positions used by the first unlit triangle path.
        Array<float3>* Positions;

        /// Authored 16-bit indices when the source mesh uses them.
        Array<uint16_t>* Indices16;

        /// Authored 32-bit indices when the source mesh uses them.
        Array<uint32_t>* Indices32;

        /// Tracks whether the runtime mesh must read from `Indices32`.
        bool Uses32BitIndices;
    };
}
```

- [ ] **Step 2: Expand the render-manager surface just enough to build authored runtime materials**

```cpp
#pragma once

#include "RenderManager3D.hpp"

class MaterialAsset;
class RuntimeMaterial;
class RuntimeModel;
class ShaderAsset;
class ModelAsset;

namespace helengine::gamecube {
    /// Orchestrates authored runtime model creation, frame extraction, and GX execution for the GameCube backend.
    class GameCubeRenderManager3D : public RenderManager3D {
    public:
        /// Creates the GameCube 3D backend bridge.
        GameCubeRenderManager3D();

        /// Builds the minimal runtime material required for the first unlit GameCube draw path.
        RuntimeMaterial* BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) override;

        /// Builds a GameCube runtime model that keeps authored submesh and geometry arrays alive.
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;
    };
}
```

- [ ] **Step 3: Rebuild immediately after the header change to verify the implementation has to catch up**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all`

Expected: FAIL because `GameCubeRenderManager3D.cpp` does not yet implement `BuildMaterialFromRaw(...)`.

- [ ] **Step 4: Implement real runtime material/model creation**

```cpp
#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#include "ModelAsset.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelSubmeshResolver.hpp"
#include "RuntimeMaterial.hpp"
#include "ShaderAsset.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    GameCubeRenderManager3D::GameCubeRenderManager3D()
        : RenderManager3D() {
    }

    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        } else if (shaderAsset == nullptr) {
            throw new ArgumentNullException("shaderAsset");
        }

        RuntimeMaterial* runtimeMaterial = new RuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        runtimeMaterial->set_CastsShadows(materialAsset->CastsShadows);
        runtimeMaterial->set_ReceivesShadows(materialAsset->ReceivesShadows);
        return runtimeMaterial;
    }

    RuntimeModel* GameCubeRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        ModelAssetIndexData* indexData = ModelAssetIndexData::Resolve(data);
        GameCubeRuntimeModel* runtimeModel = new GameCubeRuntimeModel();
        runtimeModel->set_Id(data->get_Id());
        runtimeModel->SetBounds(data->BoundsMin, data->BoundsMax);
        runtimeModel->SetSubmeshes(ModelSubmeshResolver::BuildRuntimeSubmeshes(data));
        runtimeModel->Positions = data->Positions;
        runtimeModel->Indices16 = indexData->get_Indices16();
        runtimeModel->Indices32 = indexData->get_Indices32();
        runtimeModel->Uses32BitIndices = indexData->get_Uses32BitIndices();
        return runtimeModel;
    }
}
```

- [ ] **Step 5: Rebuild and verify scene load now reaches authored model/material construction**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all`

Expected: PASS and emit `build/helengine_gc.dol`.

- [ ] **Step 6: Commit the real runtime model/material bridge**

```bash
git add src/platform/gamecube/GameCubeRuntimeModel.hpp src/platform/gamecube/GameCubeRenderManager3D.hpp src/platform/gamecube/GameCubeRenderManager3D.cpp
git commit -m "Load authored GameCube runtime models and materials"
```

### Task 3: Build the strict frame-plan extraction path

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/Makefile`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeFramePlan.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderQueueSnapshotVisitor.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneRenderBridge.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeSceneRenderBridge.cpp`
- Test: `/mnt/c/dev/helworks/helengine-gc/Makefile`

- [ ] **Step 1: Add `GameCubeSceneRenderBridge.cpp` to the build**

```make
GENERATED_BRIDGE_SOURCES := \
	$(SOURCE_DIR)/platform/gamecube/GameCubeInputManager.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager2D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager3D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneBootstrap.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneRenderBridge.cpp
```

- [ ] **Step 2: Add the backend-local frame-plan type**

```cpp
#pragma once

#include "CameraComponent.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"

namespace helengine::gamecube {
    /// Owns the first-tier reduced execution payload for one GameCube camera frame.
    class GameCubeFramePlan {
    public:
        /// Creates a frame plan with resolved camera state, viewport, and drawable submissions.
        GameCubeFramePlan(
            CameraComponent* camera,
            List<RenderFrameDrawableSubmission*>* drawableSubmissions,
            float4 viewport,
            float4x4 view,
            float4x4 projection,
            float4x4 viewProjection)
            : Camera(camera)
            , DrawableSubmissions(drawableSubmissions)
            , Viewport(viewport)
            , View(view)
            , Projection(projection)
            , ViewProjection(viewProjection) {
        }

        /// Camera that owns the render queues and authored clear settings for this frame.
        CameraComponent* Camera;

        /// Opaque drawable submissions extracted from the generated runtime graph.
        List<RenderFrameDrawableSubmission*>* DrawableSubmissions;

        /// Pixel-space viewport resolved from the authored runtime viewport.
        float4 Viewport;

        /// View matrix derived from the authored runtime camera transform.
        float4x4 View;

        /// Perspective projection derived from the authored runtime camera settings.
        float4x4 Projection;

        /// Cached view-projection matrix reused by the raster renderer.
        float4x4 ViewProjection;
    };
}
```

- [ ] **Step 3: Add the render-queue snapshot visitor**

```cpp
#pragma once

#include "IRenderVisitor3D.hpp"
#include "runtime/native_list.hpp"

namespace helengine::gamecube {
    /// Copies generated 3D render-queue items into a backend-local list without introducing GX knowledge.
    class GameCubeRenderQueueSnapshotVisitor : public IRenderVisitor3D {
    public:
        /// Creates an empty snapshot visitor before the queue walk begins.
        GameCubeRenderQueueSnapshotVisitor()
            : Items(new List<IDrawable3D*>()) {
        }

        /// Ordered queue contents captured from the generated camera queue.
        List<IDrawable3D*>* Items;

        /// Appends one queue item to the backend-local snapshot list.
        void Visit(IDrawable3D* drawable) override {
            Items->Add(drawable);
        }
    };
}
```

- [ ] **Step 4: Declare the strict runtime-to-frame-plan bridge**

```cpp
#pragma once

#include "runtime/native_list.hpp"

class CameraComponent;
class IDrawable3D;
class RendererBackendCapabilityProfile;
class float4;
class float4x4;

namespace helengine::gamecube {
    class GameCubeFramePlan;

    /// Resolves generated runtime state into the first backend-local GameCube frame plan.
    class GameCubeSceneRenderBridge {
    public:
        /// Builds one strict frame plan for the active camera and visible opaque drawables.
        GameCubeFramePlan* BuildFramePlan(RendererBackendCapabilityProfile* capabilities, int32_t targetWidth, int32_t targetHeight);
    private:
        /// Resolves the first enabled runtime camera the GameCube backend is willing to render.
        CameraComponent* ResolveActiveCamera();

        /// Copies the ordered visible 3D queue for one camera into a backend-local list.
        List<IDrawable3D*>* SnapshotVisibleDrawables(CameraComponent* camera);

        /// Builds the authored view matrix from the generated entity transform.
        float4x4 BuildViewMatrix(CameraComponent* camera);
    };
}
```

- [ ] **Step 5: Implement strict camera resolution, queue snapshot, and capability reduction**

```cpp
#include "platform/gamecube/GameCubeSceneRenderBridge.hpp"

#include "CameraComponent.hpp"
#include "CameraProjectionUtils.hpp"
#include "CameraViewportResolver.hpp"
#include "Core.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "LightComponent.hpp"
#include "ObjectManager.hpp"
#include "RenderFrame.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "RenderFrameExtractionService.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeRenderQueueSnapshotVisitor.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    GameCubeFramePlan* GameCubeSceneRenderBridge::BuildFramePlan(RendererBackendCapabilityProfile* capabilities, int32_t targetWidth, int32_t targetHeight) {
        if (capabilities == nullptr) {
            throw new ArgumentNullException("capabilities");
        } else if (targetWidth < 1) {
            throw new ArgumentOutOfRangeException("targetWidth");
        } else if (targetHeight < 1) {
            throw new ArgumentOutOfRangeException("targetHeight");
        }

        CameraComponent* camera = ResolveActiveCamera();
        List<IDrawable3D*>* drawables = SnapshotVisibleDrawables(camera);
        List<CameraComponent*>* cameras = new List<CameraComponent*>(1);
        cameras->Add(camera);
        List<LightComponent*>* lights = new List<LightComponent*>();

        RenderFrameExtractionService extractor;
        RenderFrameExtractionResult* extraction = extractor.Extract(cameras, drawables, lights, capabilities);
        RenderFrame* frame = (*extraction->Frames)[0];
        if (frame->get_HasTransparentDrawables()) {
            throw new NotSupportedException("Transparent 3D submissions are not supported in the first GameCube renderer tier.");
        }
        if (frame->get_DrawableSubmissions()->get_Count() == 0) {
            throw new InvalidOperationException("The startup GameCube scene produced no visible 3D drawable submissions.");
        }

        float4 viewport = CameraViewportResolver::ResolveViewport(camera->get_Viewport(), targetWidth, targetHeight);
        float4x4 view = BuildViewMatrix(camera);
        float4x4 projection = CameraProjectionUtils::CreatePerspectiveProjection(camera, static_cast<float>(3.14159265358979323846 / 4.0), viewport.Z / viewport.W);
        float4x4 viewProjection;
        float4x4::Multiply(view, projection, viewProjection);
        return new GameCubeFramePlan(camera, frame->get_DrawableSubmissions(), viewport, view, projection, viewProjection);
    }

    CameraComponent* GameCubeSceneRenderBridge::ResolveActiveCamera() {
        List<ICamera*>* cameras = Core::get_Instance()->get_ObjectManager()->get_Cameras();
        for (int32_t index = 0; index < cameras->get_Count(); index++) {
            CameraComponent* camera = he_cpp_try_cast<CameraComponent>((*cameras)[index]);
            if (camera == nullptr || camera->get_Parent() == nullptr || !camera->get_Parent()->get_IsHierarchyEnabled()) {
                continue;
            }
            return camera;
        }
        throw new InvalidOperationException("No active runtime camera is available for the GameCube frame plan.");
    }

    List<IDrawable3D*>* GameCubeSceneRenderBridge::SnapshotVisibleDrawables(CameraComponent* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }
        GameCubeRenderQueueSnapshotVisitor visitor;
        camera->get_RenderQueue3D()->VisitOrdered(&visitor);
        return visitor.Items;
    }

    float4x4 GameCubeSceneRenderBridge::BuildViewMatrix(CameraComponent* camera) {
        float3 cameraPosition = camera->get_Parent()->get_Position();
        float4 cameraOrientation = camera->get_Parent()->get_Orientation();
        float3 cameraForward = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), cameraOrientation);
        float3 cameraUp = float4::RotateVector(float3(0.0f, 1.0f, 0.0f), cameraOrientation);
        float3 cameraTarget = cameraPosition + cameraForward;
        float4x4 view;
        float4x4::CreateLookAt(cameraPosition, cameraTarget, cameraUp, view);
        return view;
    }
}
```

- [ ] **Step 6: Rebuild and verify the bridge compiles cleanly before GX draw work begins**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all`

Expected: PASS and emit `build/helengine_gc.dol`.

- [ ] **Step 7: Commit the frame-plan extraction slice**

```bash
git add Makefile src/platform/gamecube/GameCubeFramePlan.hpp src/platform/gamecube/GameCubeRenderQueueSnapshotVisitor.hpp src/platform/gamecube/GameCubeSceneRenderBridge.hpp src/platform/gamecube/GameCubeSceneRenderBridge.cpp
git commit -m "Extract strict GameCube frame plans from runtime scene state"
```

### Task 4: Implement the narrow GX raster path

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/Makefile`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeMeshCache.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeMeshCache.cpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Create: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.hpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager3D.cpp`
- Modify: `/mnt/c/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp`
- Test: `/mnt/c/dev/helworks/helengine-gc/Makefile`

- [ ] **Step 1: Extend the Makefile for the mesh-cache and raster-renderer sources**

```make
GENERATED_BRIDGE_SOURCES := \
	$(SOURCE_DIR)/platform/gamecube/GameCubeInputManager.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager2D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager3D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneBootstrap.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneRenderBridge.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeMeshCache.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRasterRenderer.cpp
```

- [ ] **Step 2: Add the backend-local mesh cache**

```cpp
#pragma once

#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"

class RuntimeModel;

namespace helengine::gamecube {
    class GameCubeRuntimeModel;

    /// Validates authored runtime models once and reuses the backend-local geometry view across frames.
    class GameCubeMeshCache {
    public:
        /// Creates an empty runtime-model cache keyed by authored runtime id.
        GameCubeMeshCache();

        /// Resolves a runtime model into a validated GameCube runtime model or fails loudly.
        GameCubeRuntimeModel* Resolve(RuntimeModel* runtimeModel);
    private:
        /// Cache of runtime models already validated for GX drawing.
        Dictionary<std::string, GameCubeRuntimeModel*>* CachedModelsById;
    };
}
```

```cpp
#include "platform/gamecube/GameCubeMeshCache.hpp"

#include "RuntimeModel.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/string_comparer.hpp"

namespace helengine::gamecube {
    GameCubeMeshCache::GameCubeMeshCache()
        : CachedModelsById(new Dictionary<std::string, GameCubeRuntimeModel*>(StringComparer::OrdinalIgnoreCase)) {
    }

    GameCubeRuntimeModel* GameCubeMeshCache::Resolve(RuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        }

        GameCubeRuntimeModel* cachedRuntimeModel;
        if (CachedModelsById->TryGetValue(runtimeModel->get_Id(), cachedRuntimeModel)) {
            return cachedRuntimeModel;
        }

        GameCubeRuntimeModel* typedRuntimeModel = he_cpp_try_cast<GameCubeRuntimeModel>(runtimeModel);
        if (typedRuntimeModel == nullptr) {
            throw new InvalidOperationException("GameCube 3D rendering requires GameCubeRuntimeModel instances.");
        } else if (typedRuntimeModel->Positions == nullptr || typedRuntimeModel->Positions->Length == 0) {
            throw new InvalidOperationException("GameCube runtime models must contain authored position data.");
        } else if (!typedRuntimeModel->Uses32BitIndices && (typedRuntimeModel->Indices16 == nullptr || typedRuntimeModel->Indices16->Length == 0)) {
            throw new InvalidOperationException("GameCube runtime models using 16-bit indices must provide authored index data.");
        } else if (typedRuntimeModel->Uses32BitIndices && (typedRuntimeModel->Indices32 == nullptr || typedRuntimeModel->Indices32->Length == 0)) {
            throw new InvalidOperationException("GameCube runtime models using 32-bit indices must provide authored index data.");
        }

        CachedModelsById->Add(runtimeModel->get_Id(), typedRuntimeModel);
        return typedRuntimeModel;
    }
}
```

- [ ] **Step 3: Add the GX raster renderer declaration**

```cpp
#pragma once

#include <gccore.h>

class CameraClearSettings;
class Entity;
class RuntimeSubmesh;
class float4x4;

namespace helengine::gamecube {
    class GameCubeFramePlan;
    class GameCubeMeshCache;
    class GameCubeRuntimeModel;

    /// Owns the first narrow GX execution path for authored opaque 3D meshes.
    class GameCubeRasterRenderer {
    public:
        /// Creates the raster renderer with a shared runtime-model cache.
        explicit GameCubeRasterRenderer(GameCubeMeshCache* meshCache);

        /// Draws one extracted camera frame through GX.
        void DrawFrame(GameCubeFramePlan* framePlan);
    private:
        /// Shared runtime-model cache used by the draw path.
        GameCubeMeshCache* MeshCache;

        /// Configures the GX state used by the first unlit triangle path.
        void ConfigurePipeline();

        /// Converts the authored runtime clear settings into the presented GX clear color.
        GXColor ResolveClearColor(CameraClearSettings clearSettings);

        /// Converts the authored runtime clear depth into GX packed depth.
        uint32_t ResolveClearDepth(CameraClearSettings clearSettings);

        /// Builds the world matrix for one authored entity.
        float4x4 BuildWorldMatrix(Entity* entity);

        /// Draws one authored runtime submesh through immediate GX triangle submission.
        void DrawSubmesh(GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh);
    };
}
```

- [ ] **Step 4: Expand `GameCubeRenderManager3D` to own the bridge, cache, raster path, and capability profile**

```cpp
#pragma once

#include "RenderManager3D.hpp"

class MaterialAsset;
class RuntimeMaterial;
class RuntimeModel;
class ShaderAsset;
class ModelAsset;
class RendererBackendCapabilityProfile;

namespace helengine::gamecube {
    class GameCubeMeshCache;
    class GameCubeRasterRenderer;
    class GameCubeSceneRenderBridge;

    /// Orchestrates authored runtime model creation, frame extraction, and GX execution for the GameCube backend.
    class GameCubeRenderManager3D : public RenderManager3D {
    public:
        /// Creates the GameCube 3D backend and its owned bridge/cache/raster collaborators.
        GameCubeRenderManager3D();

        /// Releases owned GameCube renderer collaborators.
        ~GameCubeRenderManager3D() override;

        /// Builds the minimal runtime material required for the first unlit GameCube draw path.
        RuntimeMaterial* BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) override;

        /// Builds a GameCube runtime model that keeps authored submesh and geometry arrays alive.
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;

        /// Extracts the current frame and renders it through GX.
        void Draw() override;

        /// Returns the strict backend capability surface exposed by the first GameCube tier.
        RendererBackendCapabilityProfile* GetCapabilityProfile() override;

        /// Reports whether this backend has already emitted at least one real GX scene frame.
        bool HasRenderedScene() const;
    private:
        /// Shared backend capability object reused across frame extraction calls.
        RendererBackendCapabilityProfile* CapabilityProfile;

        /// Converts generated runtime state into a GameCube-local frame plan.
        GameCubeSceneRenderBridge* SceneRenderBridge;

        /// Validates and reuses authored mesh geometry across frames.
        GameCubeMeshCache* MeshCache;

        /// Owns the narrow GX raster path for the first renderer milestone.
        GameCubeRasterRenderer* RasterRenderer;

        /// Tracks whether the backend has already drawn a real scene frame.
        bool HasRenderedSceneValue;
    };
}
```

- [ ] **Step 5: Implement the first opaque unlit depth-enabled GX path and final render-manager orchestration**

```cpp
#include "platform/gamecube/GameCubeRasterRenderer.hpp"

#include "CameraClearSettings.hpp"
#include "Entity.hpp"
#include "MeshComponent.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RuntimeSubmesh.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    GameCubeRasterRenderer::GameCubeRasterRenderer(GameCubeMeshCache* meshCache)
        : MeshCache(meshCache) {
        if (MeshCache == nullptr) {
            throw new ArgumentNullException("meshCache");
        }
    }

    void GameCubeRasterRenderer::DrawFrame(GameCubeFramePlan* framePlan) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        }

        ConfigurePipeline();
        GX_SetViewport(framePlan->Viewport.X, framePlan->Viewport.Y, framePlan->Viewport.Z, framePlan->Viewport.W, 0.0f, 1.0f);
        GX_SetScissor(
            static_cast<u32>(framePlan->Viewport.X),
            static_cast<u32>(framePlan->Viewport.Y),
            static_cast<u32>(framePlan->Viewport.Z),
            static_cast<u32>(framePlan->Viewport.W));
        GX_SetCopyClear(ResolveClearColor(framePlan->Camera->get_ClearSettings()), ResolveClearDepth(framePlan->Camera->get_ClearSettings()));

        Mtx44 projection;
        const float4x4& sourceProjection = framePlan->Projection;
        projection[0][0] = sourceProjection.M11; projection[0][1] = sourceProjection.M12; projection[0][2] = sourceProjection.M13; projection[0][3] = sourceProjection.M14;
        projection[1][0] = sourceProjection.M21; projection[1][1] = sourceProjection.M22; projection[1][2] = sourceProjection.M23; projection[1][3] = sourceProjection.M24;
        projection[2][0] = sourceProjection.M31; projection[2][1] = sourceProjection.M32; projection[2][2] = sourceProjection.M33; projection[2][3] = sourceProjection.M34;
        projection[3][0] = sourceProjection.M41; projection[3][1] = sourceProjection.M42; projection[3][2] = sourceProjection.M43; projection[3][3] = sourceProjection.M44;
        GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

        for (int32_t submissionIndex = 0; submissionIndex < framePlan->DrawableSubmissions->get_Count(); submissionIndex++) {
            RenderFrameDrawableSubmission* submission = (*framePlan->DrawableSubmissions)[submissionIndex];
            GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());
            RuntimeSubmesh* runtimeSubmesh = (*runtimeModel->get_Submeshes())[submission->get_SubmeshIndex()];
            float4x4 world = BuildWorldMatrix(submission->get_Drawable()->get_Parent());
            float4x4 worldView;
            float4x4::Multiply(world, framePlan->View, worldView);

            Mtx modelView;
            modelView[0][0] = worldView.M11; modelView[0][1] = worldView.M12; modelView[0][2] = worldView.M13; modelView[0][3] = worldView.M14;
            modelView[1][0] = worldView.M21; modelView[1][1] = worldView.M22; modelView[1][2] = worldView.M23; modelView[1][3] = worldView.M24;
            modelView[2][0] = worldView.M31; modelView[2][1] = worldView.M32; modelView[2][2] = worldView.M33; modelView[2][3] = worldView.M34;
            GX_LoadPosMtxImm(modelView, GX_PNMTX0);
            GX_SetCurrentMtx(GX_PNMTX0);
            DrawSubmesh(runtimeModel, runtimeSubmesh);
        }
    }

    void GameCubeRasterRenderer::ConfigurePipeline() {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetNumChans(1);
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
        GX_SetNumTexGens(0);
        GX_SetNumTevStages(1);
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GX_SetCullMode(GX_CULL_BACK);
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_FALSE);
    }

    GXColor GameCubeRasterRenderer::ResolveClearColor(CameraClearSettings clearSettings) {
        if (!clearSettings.get_ClearColorEnabled()) {
            return GXColor { 0x00, 0x00, 0x00, 0xFF };
        }
        float4 color = clearSettings.get_ClearColor();
        return GXColor {
            static_cast<u8>(color.X * 255.0f),
            static_cast<u8>(color.Y * 255.0f),
            static_cast<u8>(color.Z * 255.0f),
            static_cast<u8>(color.W * 255.0f)
        };
    }

    uint32_t GameCubeRasterRenderer::ResolveClearDepth(CameraClearSettings clearSettings) {
        if (!clearSettings.get_ClearDepthEnabled()) {
            return 0x00FFFFFF;
        }
        return static_cast<uint32_t>(clearSettings.get_ClearDepth() * 16777215.0f);
    }

    float4x4 GameCubeRasterRenderer::BuildWorldMatrix(Entity* entity) {
        if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }
        float4x4 rotation;
        float4 orientation = entity->get_Orientation();
        float4x4::CreateFromQuaternion(orientation, rotation);
        float4x4 scale;
        float3 worldScale = entity->get_Scale();
        float4x4::CreateScale(worldScale.X, worldScale.Y, worldScale.Z, scale);
        float4x4 rotationScale;
        float4x4::Multiply(rotation, scale, rotationScale);
        float4x4 translation;
        float3 worldPosition = entity->get_Position();
        float4x4::CreateTranslation(worldPosition, translation);
        float4x4 world;
        float4x4::Multiply(rotationScale, translation, world);
        return world;
    }

    void GameCubeRasterRenderer::DrawSubmesh(GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeSubmesh == nullptr) {
            throw new ArgumentNullException("runtimeSubmesh");
        }

        const int32_t firstIndex = runtimeSubmesh->get_IndexStart();
        const int32_t indexCount = runtimeSubmesh->get_IndexCount();
        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, indexCount);
        for (int32_t localIndex = 0; localIndex < indexCount; localIndex++) {
            const int32_t sourceIndex = firstIndex + localIndex;
            const uint32_t positionIndex = runtimeModel->Uses32BitIndices
                ? (*runtimeModel->Indices32)[sourceIndex]
                : (*runtimeModel->Indices16)[sourceIndex];
            const float3 position = (*runtimeModel->Positions)[positionIndex];
            GX_Position3f32(position.X, position.Y, position.Z);
            GX_Color4u8(255, 255, 255, 255);
        }
        GX_End();
    }
}
```

```cpp
#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "ModelAsset.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelSubmeshResolver.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "RuntimeMaterial.hpp"
#include "ShaderAsset.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRasterRenderer.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "platform/gamecube/GameCubeSceneRenderBridge.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    GameCubeRenderManager3D::GameCubeRenderManager3D()
        : RenderManager3D()
        , CapabilityProfile(new RendererBackendCapabilityProfile(true, false, false, false, 0, 0))
        , SceneRenderBridge(new GameCubeSceneRenderBridge())
        , MeshCache(new GameCubeMeshCache())
        , RasterRenderer(new GameCubeRasterRenderer(MeshCache))
        , HasRenderedSceneValue(false) {
    }

    GameCubeRenderManager3D::~GameCubeRenderManager3D() {
        delete RasterRenderer;
        delete MeshCache;
        delete SceneRenderBridge;
        delete CapabilityProfile;
    }

    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        } else if (shaderAsset == nullptr) {
            throw new ArgumentNullException("shaderAsset");
        }

        RuntimeMaterial* runtimeMaterial = new RuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        runtimeMaterial->set_CastsShadows(materialAsset->CastsShadows);
        runtimeMaterial->set_ReceivesShadows(materialAsset->ReceivesShadows);
        return runtimeMaterial;
    }

    RuntimeModel* GameCubeRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        ModelAssetIndexData* indexData = ModelAssetIndexData::Resolve(data);
        GameCubeRuntimeModel* runtimeModel = new GameCubeRuntimeModel();
        runtimeModel->set_Id(data->get_Id());
        runtimeModel->SetBounds(data->BoundsMin, data->BoundsMax);
        runtimeModel->SetSubmeshes(ModelSubmeshResolver::BuildRuntimeSubmeshes(data));
        runtimeModel->Positions = data->Positions;
        runtimeModel->Indices16 = indexData->get_Indices16();
        runtimeModel->Indices32 = indexData->get_Indices32();
        runtimeModel->Uses32BitIndices = indexData->get_Uses32BitIndices();
        return runtimeModel;
    }

    void GameCubeRenderManager3D::Draw() {
        GameCubeFramePlan* framePlan = SceneRenderBridge->BuildFramePlan(
            CapabilityProfile,
            MainWindowSize.X,
            MainWindowSize.Y);
        RasterRenderer->DrawFrame(framePlan);
        HasRenderedSceneValue = true;
        delete framePlan;
    }

    RendererBackendCapabilityProfile* GameCubeRenderManager3D::GetCapabilityProfile() {
        return CapabilityProfile;
    }

    bool GameCubeRenderManager3D::HasRenderedScene() const {
        return HasRenderedSceneValue;
    }
}
```

- [ ] **Step 6: Stop the host diagnostic clear path from overwriting real rendered frames**

```cpp
void GameCubeApplication::PresentFrame() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!EngineInitialized || EngineRenderManager3D == nullptr || !EngineRenderManager3D->HasRenderedScene()) {
            const GXColor visibleColor = ResolvePresentedClearColor();
            GX_SetCopyClear(visibleColor, 0x00FFFFFF);
        }
#else
        const GXColor visibleColor = ResolvePresentedClearColor();
        GX_SetCopyClear(visibleColor, 0x00FFFFFF);
#endif
        GX_CopyDisp(FrameBuffer, GX_TRUE);
        GX_DrawDone();
        GX_Flush();
        VIDEO_SetNextFramebuffer(FrameBuffer);
        VIDEO_Flush();
        VIDEO_WaitVSync();
}
```

- [ ] **Step 7: Rebuild and verify the `.dol` is ready for the first real draw-path check**

Run: `rtk docker run --rm -v "$PWD":/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube helengine-gc make clean all`

Expected: PASS and emit `build/helengine_gc.dol`.

- [ ] **Step 8: Launch Dolphin and verify the authored cube is visibly rendered and rotating**

Run: `rtk cmd.exe /c "cd /d C:\dev\helworks\helengine-gc && start \"\" \"C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe\" \"C:\dev\helworks\helengine-gc\build\helengine_gc.dol\""`

Expected: Dolphin shows the `cube_test` camera framing, a visible 3D cube rendered through GX, and continuous rotation driven by `DirectionalShadowTowerSpinComponent` through the real generated update path.

- [ ] **Step 9: Commit the first GX raster path**

```bash
git add Makefile src/platform/gamecube/GameCubeMeshCache.hpp src/platform/gamecube/GameCubeMeshCache.cpp src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeRenderManager3D.hpp src/platform/gamecube/GameCubeRenderManager3D.cpp src/platform/gamecube/GameCubeApplication.cpp
git commit -m "Render cube test scene through GameCube GX path"
```

### Task 5: Document the reproducible rotating-cube verification flow

**Files:**
- Modify: `/mnt/c/dev/helworks/helengine-gc/README.md`
- Test: `/mnt/c/dev/helworks/helengine-gc/README.md`

- [ ] **Step 1: Add the staged-content and Dolphin launch flow to the README**

```markdown
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
```

- [ ] **Step 2: Review the README snippet in place**

Run: `sed -n '1,220p' README.md`

Expected: the README shows the staging command, build command, Dolphin launch command, and the rotating-cube success criteria.

- [ ] **Step 3: Commit the verification docs**

```bash
git add README.md
git commit -m "Document GameCube rotating cube verification"
```
