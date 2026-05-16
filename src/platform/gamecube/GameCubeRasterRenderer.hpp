#pragma once

#include <cstdint>

#include <gccore.h>

#include "MaterialCullMode.hpp"

class CameraClearSettings;
class Entity;
class RenderFrameDrawableSubmission;
class RuntimeMaterial;
class RuntimeSubmesh;
class float3;
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

        /// Resolves whether one submission should use the lit branch for the current checkpoint.
        bool UsesLitBranch(RenderFrameDrawableSubmission* submission);

        /// Maps the shared material cull-mode contract onto the reversed GX face-culling convention.
        u8 ResolveGxCullMode(MaterialCullMode cullMode);

        /// Accumulates ambient plus directional light into a white diffuse lighting result.
        float3 AccumulateAmbientAndDirectionalLight(GameCubeFramePlan* framePlan, Entity* entity, float3 normal);

        /// Evaluates one final GX vertex color for the first lit branch.
        GXColor EvaluateLitVertexColor(GameCubeFramePlan* framePlan, Entity* entity, float3 normal);

        /// Converts a normalized RGB lighting value into a GX color with full alpha.
        GXColor ConvertLightingColorToGx(float3 color);

        /// Builds one authored world matrix through the generated platform-adapted float4x4 runtime.
        void BuildWorldMatrix(Entity* entity, float4x4& worldMatrix);

        /// Builds one authored model-view matrix through the generated platform-adapted float4x4 runtime.
        void BuildModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix);

        /// Draws one authored runtime submesh through immediate GX triangle submission and the active entity transform.
        void DrawSubmesh(GameCubeFramePlan* framePlan, RenderFrameDrawableSubmission* submission, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity);
    };
}
