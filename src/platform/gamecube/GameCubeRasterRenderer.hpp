#pragma once

#include <cstdint>

#include <gccore.h>

class CameraClearSettings;
class CameraComponent;
class Entity;
class RuntimeSubmesh;
class float3;
class float4;

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

        /// Counts rasterized frames for throttled diagnostics.
        uint32_t RasterizedFrameCount;

        /// Configures the GX state used by the first unlit triangle path.
        void ConfigurePipeline();

        /// Converts the authored runtime clear settings into the presented GX clear color.
        GXColor ResolveClearColor(CameraClearSettings clearSettings);

        /// Converts the authored runtime clear depth into GX packed depth.
        uint32_t ResolveClearDepth(CameraClearSettings clearSettings);

        /// Builds one native GX view matrix from the active runtime camera.
        void BuildViewMatrix(CameraComponent* camera, Mtx& viewMatrix);

        /// Builds one native GX world matrix from the authored entity transform.
        void BuildWorldMatrix(Entity* entity, Mtx& worldMatrix);

        /// Builds one native GX projection matrix from the active runtime camera and viewport.
        void BuildProjectionMatrix(GameCubeFramePlan* framePlan, Mtx44& projectionMatrix);

        /// Draws one authored runtime submesh through immediate GX triangle submission.
        void DrawSubmesh(GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh);
    };
}
