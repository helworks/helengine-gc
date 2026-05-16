#pragma once

#include <cstdint>

#include "RenderManager3D.hpp"

class MaterialAsset;
class PlatformMaterialAsset;
class RuntimeMaterial;
class RuntimeModel;
class ShaderAsset;
class RendererBackendCapabilityProfile;
class MaterialLayout;

namespace helengine::gamecube {
    class GameCubeMeshCache;
    class GameCubeRasterRenderer;
    class GameCubeRenderManager2D;
    class GameCubeRuntimeMaterial;
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

        /// Builds the minimal runtime material required for the first cooked-material GameCube draw path.
        RuntimeMaterial* BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) override;

        /// Builds a GameCube runtime model that keeps authored submesh and geometry arrays alive.
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;

        /// Extracts the current frame and renders it through GX.
        void Draw() override;

        /// Draws the captured 2D overlay for the current frame through the shared GX raster path.
        void Draw2D(GameCubeRenderManager2D* renderManager2D, uint16_t frameWidth, uint16_t frameHeight);

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

        /// Counts extracted GameCube scene frames for throttled diagnostics.
        uint32_t ExtractedFrameCount;

        /// Builds the minimal shared-engine material layout needed for one diffuse texture binding.
        MaterialLayout* CreateCookedMaterialLayout();
    };
}
