#pragma once

#include <cstdint>

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

        /// Draws one extracted camera frame through GX and reports whether this frame claimed scene presentation ownership.
        bool DrawFrame(GameCubeFramePlan* framePlan);
    private:
        /// Shared runtime-model cache used by the draw path.
        GameCubeMeshCache* MeshCache;

        /// Configures the GX state used by the first opaque mesh path.
        void ConfigurePipeline();

        /// Converts the authored runtime clear settings into the presented GX clear color.
        GXColor ResolveClearColor(CameraClearSettings clearSettings);

        /// Converts the authored runtime clear depth into GX packed depth.
        uint32_t ResolveClearDepth(CameraClearSettings clearSettings);

        /// Copies one generated affine matrix directly into a GX position matrix without runtime reinterpretation.
        void CopyAffineMatrixToGx(const float4x4& source, Mtx& destination);

        /// Copies one generated projection matrix directly into a GX projection matrix without runtime reinterpretation.
        void CopyProjectionMatrixToGx(const float4x4& source, Mtx44& destination);

        /// Builds one authored world matrix through the generated platform-adapted float4x4 runtime.
        void BuildWorldMatrix(Entity* entity, float4x4& worldMatrix);

        /// Builds one authored model-view matrix through the generated platform-adapted float4x4 runtime.
        void BuildModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix);

        /// Draws one authored runtime submesh through immediate GX triangle submission and the active entity transform.
        void DrawSubmesh(GameCubeFramePlan* framePlan, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity);
    };
}
