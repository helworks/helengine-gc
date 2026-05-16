#pragma once

#include <cstdint>
#include <vector>

#include <gccore.h>

#include "MaterialCullMode.hpp"
#include "RoundedRectCorners.hpp"

class CameraClearSettings;
class Entity;
class RenderFrameDrawableSubmission;
class RuntimeMaterial;
class RuntimeTexture;
class RuntimeSubmesh;
class byte4;
class float2;
class float3;
class float4;
class float4x4;

namespace helengine::gamecube {
    class GameCubeFramePlan;
    class GameCubeMeshCache;
    class GameCubeRenderManager2D;
    class GameCubeRuntimeMaterial;
    class GameCubeRuntimeModel;
    class GameCubeRuntimeTexture;
    struct GameCubeRoundedRectDrawCommand;
    struct GameCubeSpriteDrawCommand;
    struct GameCubeTextDrawCommand;

    /// Owns the first narrow GX execution path for authored opaque 3D meshes.
    class GameCubeRasterRenderer {
    public:
        /// Creates the raster renderer with a shared runtime-model cache.
        explicit GameCubeRasterRenderer(GameCubeMeshCache* meshCache);

        /// Draws one extracted camera frame through GX and reports whether this frame claimed scene presentation ownership.
        bool DrawFrame(GameCubeFramePlan* framePlan);

        /// Draws the captured 2D overlay drawables for the current frame through GX.
        void Render2D(const GameCubeRenderManager2D& renderManager2D, uint16_t frameWidth, uint16_t frameHeight);
    private:
        /// Shared runtime-model cache used by the draw path.
        GameCubeMeshCache* MeshCache;

        /// Configures the GX state used by the current opaque mesh path.
        void ConfigurePipeline(bool useTexturedBranch);

        /// Converts the authored runtime clear settings into the presented GX clear color.
        GXColor ResolveClearColor(CameraClearSettings clearSettings);

        /// Converts the authored runtime clear depth into GX packed depth.
        uint32_t ResolveClearDepth(CameraClearSettings clearSettings);

        /// Configures the GX state used by the current 2D overlay path.
        void Configure2DPipeline(bool useTexturedBranch);

        /// Copies one generated affine matrix directly into a GX position matrix without runtime reinterpretation.
        void CopyAffineMatrixToGx(const float4x4& source, Mtx& destination);

        /// Copies one generated projection matrix directly into a GX projection matrix without runtime reinterpretation.
        void CopyProjectionMatrixToGx(const float4x4& source, Mtx44& destination);

        /// Resolves whether one submission should use the lit branch for the current checkpoint.
        bool UsesLitBranch(RenderFrameDrawableSubmission* submission);

        /// Resolves one GameCube-native runtime texture from the current material graph when present.
        GameCubeRuntimeTexture* ResolveBoundTexture(GameCubeRuntimeMaterial* material);

        /// Maps the shared material cull-mode contract onto the reversed GX face-culling convention.
        u8 ResolveGxCullMode(MaterialCullMode cullMode);

        /// Accumulates ambient plus directional light into a white diffuse lighting result.
        float3 AccumulateAmbientAndDirectionalLight(GameCubeFramePlan* framePlan, Entity* entity, float3 normal);

        /// Evaluates one final GX vertex color for the first lit branch.
        GXColor EvaluateLitVertexColor(GameCubeFramePlan* framePlan, Entity* entity, GameCubeRuntimeMaterial* material, float3 normal);

        /// Converts a normalized RGB lighting value into a GX color with full alpha.
        GXColor ConvertLightingColorToGx(float3 color);

        /// Converts one shared-engine byte color into a GX color.
        GXColor ConvertByteColorToGx(const byte4& color);

        /// Builds one authored world matrix through the generated platform-adapted float4x4 runtime.
        void BuildWorldMatrix(Entity* entity, float4x4& worldMatrix);

        /// Builds one authored model-view matrix through the generated platform-adapted float4x4 runtime.
        void BuildModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix);

        /// Draws one authored runtime submesh through immediate GX triangle submission and the active entity transform.
        void DrawSubmesh(GameCubeFramePlan* framePlan, RenderFrameDrawableSubmission* submission, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity);

        /// Draws one captured rounded rectangle through the current 2D GX path.
        void RenderRoundedRect2D(const GameCubeRoundedRectDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);

        /// Draws one captured sprite through the current 2D GX path.
        void RenderSprite2D(const GameCubeSpriteDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);

        /// Draws one captured text drawable through the current 2D GX path.
        void RenderText2D(const GameCubeTextDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);

        /// Emits one untextured screen-space quad in pixel coordinates.
        void DrawSolidQuad2D(float x, float y, float width, float height, GXColor color);

        /// Emits one rounded screen-space convex polygon in pixel coordinates.
        void DrawRoundedPolygon2D(float x, float y, float width, float height, float radius, RoundedRectCorners corners, GXColor color);

        /// Builds one clockwise rounded-rectangle outline in screen-space coordinates.
        void BuildRoundedRectOutline(float x, float y, float width, float height, float radius, RoundedRectCorners corners, std::vector<float2>& outline);

        /// Appends one clockwise corner arc into the outline builder.
        void AppendCornerArc(std::vector<float2>& outline, float centerX, float centerY, float radius, float startAngle, float endAngle, bool skipFirstPoint);

        /// Resolves whether the authored corner mask includes one specific corner.
        bool HasRoundedCorner(RoundedRectCorners corners, RoundedRectCorners corner) const;

        /// Emits one textured screen-space quad in pixel coordinates.
        void DrawTexturedQuad2D(float x, float y, float width, float height, const float4& sourceRect, GXColor color, GameCubeRuntimeTexture* texture);

        /// Resolves the active clip rectangle for one 2D drawable by intersecting ancestor clip regions.
        bool TryResolveClipRect(Entity* entity, float4& clipRect);

        /// Applies one resolved clip rectangle as the active GX scissor state.
        void ApplyClipScissor(const float4& clipRect, uint16_t frameWidth, uint16_t frameHeight);
    };
}
