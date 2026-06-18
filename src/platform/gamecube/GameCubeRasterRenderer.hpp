#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <gccore.h>

#include "MaterialCullMode.hpp"
#include "RoundedRectCorners.hpp"
#include "helengine_float2.hpp"

class CameraClearSettings;
class Entity;
class FontAsset;
class ITextDrawable2D;
class RenderFrameDrawableSubmission;
class RuntimeMaterial;
class RuntimeTexture;
class RuntimeSubmesh;
class byte4;
class float3;
class float4;
class float4x4;

namespace helengine::gamecube {
    class GameCubeCachedMeshData;
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

        /// Draws the captured 2D overlay drawables for the current frame through GX using the active frame-plan camera to resolve 2D-only background clears.
        void Render2D(GameCubeFramePlan* framePlan, const GameCubeRenderManager2D& renderManager2D, uint16_t frameWidth, uint16_t frameHeight);
    private:
        /// Stores one persistent wrapped-text result so unchanged UI labels do not rebuild layout every frame.
        struct CachedTextLayoutEntry {
            /// Drawable instance that owns the wrapped text state.
            ITextDrawable2D* Drawable;

            /// Font asset used when the wrapped text was last rebuilt.
            FontAsset* Font;

            /// Pixel width constraint used when the wrapped text was last rebuilt.
            int32_t MaxWidth;

            /// Source text captured from the drawable during the last rebuild.
            std::string SourceText;

            /// Wrapped text content reused until the drawable text or width changes.
            std::string WrappedText;

            /// Render2D frame id that last touched this cache entry.
            uint32_t LastFrameUsed;
        };

        /// Shared runtime-model cache used by the draw path.
        GameCubeMeshCache* MeshCache;

        /// Tracks whether the renderer already logged the first textured 3D draw for runtime diagnosis.
        bool HasLoggedFirstTexturedDraw;

        /// Persistent wrapped-text cache reused across overlay frames for stable menu labels.
        std::vector<CachedTextLayoutEntry> CachedTextLayouts;

        /// Scratch rounded-rectangle outline reused across 2D shape draws to avoid per-call heap churn.
        std::vector<float2> RoundedRectOutlineScratch;

        /// Identifies the active Render2D frame while text-layout cache pruning runs.
        uint32_t ActiveTextLayoutFrameId;

        /// Configures the GX state used by the current opaque mesh path.
        void ConfigurePipeline(bool useTexturedBranch, bool useIndexedGeometry);

        /// Configures the GX state used by the indexed lit mesh path with GX fixed-function lighting enabled.
        void ConfigureLitPipeline(bool useTexturedBranch, bool useIndexedGeometry);

        /// Binds the cached mesh arrays used by the indexed GameCube draw path.
        void BindCachedMeshArrays(GameCubeCachedMeshData* cachedMeshData, bool useTexturedBranch);

        /// Configures one GX directional-light state block from the extracted render-frame lighting inputs.
        void ConfigureDirectionalLight(GameCubeFramePlan* framePlan, GXLightObj& lightObject, GXColor& ambientColor, bool& hasDirectionalLight);

        /// Converts the authored runtime clear settings into the presented GX clear color.
        GXColor ResolveClearColor(CameraClearSettings clearSettings);

        /// Converts the authored runtime clear depth into GX packed depth.
        uint32_t ResolveClearDepth(CameraClearSettings clearSettings);

        /// Configures the GX state used by the current 2D overlay path.
        void Configure2DPipeline(bool useTexturedBranch);

        /// Copies one generated affine matrix directly into a GX position matrix without runtime reinterpretation.
        void CopyAffineMatrixToGx(const float4x4& source, Mtx& destination);

        /// Copies one generated projection matrix into the GX projection upload layout.
        void CopyProjectionMatrixToGx(const float4x4& source, Mtx44& destination);

        /// Maps one logical 2D rectangle from shared-engine viewport space into the active physical GameCube viewport.
        void TransformLogicalRectToPhysicalViewport(GameCubeFramePlan* framePlan, float& x, float& y, float& width, float& height) const;

        /// Loads one GX normal matrix derived from the current authored model-view transform so fixed-function lighting stays in view space.
        void LoadNormalMatrix(const Mtx& modelViewMatrix);

        /// Resolves whether one submission should use the lit branch for the current checkpoint.
        bool UsesLitBranch(RenderFrameDrawableSubmission* submission);

        /// Resolves one GameCube-native runtime texture from the current material graph when present.
        GameCubeRuntimeTexture* ResolveBoundTexture(GameCubeRuntimeMaterial* material);

        /// Maps the shared material cull-mode contract onto the reversed GX face-culling convention.
        u8 ResolveGxCullMode(MaterialCullMode cullMode);

        /// Converts a normalized RGB lighting value into a GX color with full alpha.
        GXColor ConvertLightingColorToGx(float3 color);

        /// Converts one shared-engine byte color into a GX color.
        GXColor ConvertByteColorToGx(const byte4& color);

        /// Transforms one world-space direction through the frame-plan view rotation so GX lighting receives a view-space light direction.
        float3 TransformDirectionToViewSpace(const float3& direction, const float4x4& viewMatrix);

        /// Builds one authored world matrix through the generated platform-adapted float4x4 runtime.
        void BuildWorldMatrix(Entity* entity, float4x4& worldMatrix);

        /// Multiplies two row-vector matrices using the shared engine convention expected by the GameCube raster path.
        void MultiplyMatrices(const float4x4& left, const float4x4& right, float4x4& result);

        /// Builds one authored model-view matrix through the generated platform-adapted float4x4 runtime.
        void BuildModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix);

        /// Builds one intentionally wrong model-view matrix used only by the temporary capture simulation.
        void BuildBadCaptureModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix);

        /// Returns whether one of the two simulated capture triangles should be emitted this frame.
        bool ShouldDrawCaptureTriangle(uint32_t frameIndex, int32_t triangleIndex) const;

        /// Returns one per-frame scale factor used to simulate the old matrix explosions.
        float GetCaptureDistortionScale(uint32_t frameIndex) const;

        /// Emits one temporary capture triangle through the intentionally bad transform path.
        void DrawCaptureTriangle(GameCubeFramePlan* framePlan, Entity* entity, const float3& localA, const float3& localB, const float3& localC, uint32_t frameIndex, int32_t triangleIndex);

        /// Draws one unlit or textured cached submesh through indexed GX array submission.
        void DrawCachedSubmesh(GameCubeRuntimeMaterial* material, GameCubeCachedMeshData* cachedMeshData, RuntimeSubmesh* runtimeSubmesh, bool useTexturedBranch);

        /// Draws one lit cached submesh through the indexed GX lighting path.
        void DrawCachedLitSubmesh(GameCubeFramePlan* framePlan, Entity* entity, GameCubeRuntimeMaterial* material, GameCubeCachedMeshData* cachedMeshData, RuntimeSubmesh* runtimeSubmesh, bool useTexturedBranch);

        /// Draws one authored runtime submesh through immediate GX triangle submission and the active entity transform.
        void DrawSubmesh(GameCubeFramePlan* framePlan, RenderFrameDrawableSubmission* submission, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity);

        /// Draws one captured rounded rectangle through the current 2D GX path.
        void RenderRoundedRect2D(GameCubeFramePlan* framePlan, const GameCubeRoundedRectDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);

        /// Draws one captured sprite through the current 2D GX path.
        void RenderSprite2D(GameCubeFramePlan* framePlan, const GameCubeSpriteDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);

        /// Draws one captured text drawable through the current 2D GX path.
        void RenderText2D(GameCubeFramePlan* framePlan, const GameCubeTextDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);

        /// Resolves reusable text-layout content for one text drawable, rebuilding wrapped text only when authored inputs change.
        const std::string& ResolveTextLayoutContent(ITextDrawable2D* drawable, FontAsset* font, double fontScale);

        /// Discards wrapped-text cache entries that were not used by the active overlay frame.
        void PruneTextLayoutCache();

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
        void ApplyClipScissor(GameCubeFramePlan* framePlan, const float4& clipRect, uint16_t frameWidth, uint16_t frameHeight);
    };
}
