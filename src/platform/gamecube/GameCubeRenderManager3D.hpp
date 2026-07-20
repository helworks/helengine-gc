#pragma once

#include <cstdint>

#include "RenderManager3D.hpp"

class ContentManager;
class PlatformMaterialAsset;
class RuntimeMaterial;
class RuntimeModel;
class RendererBackendCapabilityProfile;

namespace helengine::gamecube {
    class GameCubeMeshCache;
    class GameCubeRasterRenderer;
    class GameCubeRenderManager2D;
    class GameCubeRuntimeMaterial;
    class GameCubeRuntimeModel;
    class GameCubeSceneRenderBridge;

    /// Orchestrates authored runtime model creation, frame extraction, and GX execution for the GameCube backend.
    class GameCubeRenderManager3D : public RenderManager3D {
    public:
        /// Creates the GameCube 3D backend and its owned bridge/cache/raster collaborators.
        GameCubeRenderManager3D();

        /// Releases owned GameCube renderer collaborators.
        ~GameCubeRenderManager3D() override;

        /// Rebuilds one legacy raw material asset path through the cooked platform-owned GameCube material contract.
        RuntimeMaterial* BuildMaterialFromRawAsset(ContentManager* assetContentManager, std::string materialAssetPath) override;

        /// Builds the minimal runtime material required for the first cooked-material GameCube draw path.
        RuntimeMaterial* BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset);

        /// Builds the minimal runtime material required for the first cooked-material GameCube draw path from one serialized cooked material asset path.
        RuntimeMaterial* BuildMaterialFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource);

        /// Builds a GameCube runtime model that keeps authored submesh and geometry arrays alive.
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;

        /// Builds a GameCube runtime model from one serialized cooked model asset path.
        RuntimeModel* BuildModelFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource) override;

        /// Releases one GameCube runtime material after the final scene reference is removed.
        void ReleaseMaterial(RuntimeMaterial* material) override;

        /// Releases one GameCube runtime model after the final scene reference is removed.
        void ReleaseModel(RuntimeModel* model) override;

        /// Releases any deferred runtime-material and runtime-model deletions after the scene manager reaches a safe transition boundary.
        void FlushReleasedAssets() override;

        /// Extracts the current frame and renders it through GX.
        void Draw() override;

        /// Registers the 2D overlay render manager that should be captured and rasterized inside the shared 3D draw call.
        void SetOverlayRenderManager2D(GameCubeRenderManager2D* renderManager2D);

        /// Registers the physical presented framebuffer size used for GX viewport and scissor setup.
        void SetPresentedFrameSize(uint16_t width, uint16_t height);

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

        /// Stores the 2D render manager whose captured overlay drawables must be completed before frame-boundary scene commits run.
        GameCubeRenderManager2D* OverlayRenderManager2D;

        /// Tracks whether the backend has already drawn a real scene frame.
        bool HasRenderedSceneValue;

        /// Tracks the physical framebuffer width used by the raster path.
        uint16_t PresentedFrameWidth;

        /// Tracks the physical framebuffer height used by the raster path.
        uint16_t PresentedFrameHeight;

        /// Counts extracted GameCube scene frames for throttled diagnostics.
        uint32_t ExtractedFrameCount;

        /// Runtime materials deferred until the renderer reaches a safe destruction boundary.
        std::vector<RuntimeMaterial*> ReleasedMaterials;

        /// Runtime models deferred until the renderer reaches a safe destruction boundary.
        std::vector<RuntimeModel*> ReleasedModels;

        /// Resolves one packaged content-relative asset path against the absolute cooked material path that referenced it.
        std::string ResolvePackagedContentAssetPath(const std::string& cookedMaterialAssetPath, const std::string& contentRelativePath);

        /// Loads and attaches one cooked diffuse texture when the path-based GameCube cooked-material contract references one.
        void AttachCookedDiffuseTexture(GameCubeRuntimeMaterial* runtimeMaterial, PlatformMaterialAsset* materialAsset, const std::string& cookedMaterialAssetPath, IContentStreamSource* contentStreamSource);

        /// Releases one owned deserialized cooked model payload attached to a GameCube runtime model.
        void ReleaseOwnedSourceModelAsset(GameCubeRuntimeModel* runtimeModel);
    };
}
