#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#include <ogc/system.h>

#include "Entity.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRasterRenderer.hpp"
#include "ModelAsset.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelSubmeshResolver.hpp"
#include "RuntimeMaterial.hpp"
#include "ShaderAsset.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "platform/gamecube/GameCubeSceneRenderBridge.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube 3D backend and its owned bridge/cache/raster collaborators.
    GameCubeRenderManager3D::GameCubeRenderManager3D()
        : RenderManager3D()
        , CapabilityProfile(new RendererBackendCapabilityProfile(true, false, false, false, 0, 0))
        , SceneRenderBridge(new GameCubeSceneRenderBridge())
        , MeshCache(new GameCubeMeshCache())
        , RasterRenderer(new GameCubeRasterRenderer(MeshCache))
        , HasRenderedSceneValue(false)
        , ExtractedFrameCount(0U) {
    }

    /// Releases owned GameCube renderer collaborators.
    GameCubeRenderManager3D::~GameCubeRenderManager3D() {
        delete RasterRenderer;
        delete MeshCache;
        delete SceneRenderBridge;
        delete CapabilityProfile;
    }

    /// Builds the minimal runtime material required for the first unlit GameCube draw path.
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

    /// Builds a GameCube runtime model that keeps authored submesh and geometry arrays alive.
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

    /// Extracts the current frame and renders it through GX.
    void GameCubeRenderManager3D::Draw() {
        GameCubeFramePlan* framePlan = SceneRenderBridge->BuildFramePlan(CapabilityProfile, MainWindowSize.X, MainWindowSize.Y);
        ExtractedFrameCount++;
        if (ExtractedFrameCount <= 5U || (ExtractedFrameCount % 60U) == 0U) {
            RenderFrameDrawableSubmission* firstSubmission = (*framePlan->DrawableSubmissions)[0];
            Entity* firstEntity = firstSubmission->get_Drawable()->get_Parent();
            const float3 firstEntityPosition = firstEntity->get_Position();
            SYS_Report(
                "[GC] Frame %u extracted. submissions=%d viewport=(%.2f, %.2f, %.2f, %.2f) firstEntity=(%.3f, %.3f, %.3f) submesh=%d\n",
                ExtractedFrameCount,
                framePlan->DrawableSubmissions->get_Count(),
                framePlan->Viewport.X,
                framePlan->Viewport.Y,
                framePlan->Viewport.Z,
                framePlan->Viewport.W,
                firstEntityPosition.X,
                firstEntityPosition.Y,
                firstEntityPosition.Z,
                firstSubmission->get_SubmeshIndex());
        }
        HasRenderedSceneValue = RasterRenderer->DrawFrame(framePlan);
        delete framePlan;
    }

    /// Returns the strict backend capability surface exposed by the first GameCube tier.
    RendererBackendCapabilityProfile* GameCubeRenderManager3D::GetCapabilityProfile() {
        return CapabilityProfile;
    }

    /// Reports whether this backend has already emitted at least one real GX scene frame.
    bool GameCubeRenderManager3D::HasRenderedScene() const {
        return HasRenderedSceneValue;
    }
}
