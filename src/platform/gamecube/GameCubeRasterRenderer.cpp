#include "platform/gamecube/GameCubeRasterRenderer.hpp"

#include <ogc/system.h>

#include "CameraClearSettings.hpp"
#include "CameraComponent.hpp"
#include "Entity.hpp"
#include "IDrawable3D.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RuntimeSubmesh.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/native_exceptions.hpp"

namespace {
    constexpr u8 OpaqueMeshColorRed = 255;
    constexpr u8 OpaqueMeshColorGreen = 255;
    constexpr u8 OpaqueMeshColorBlue = 0;
    constexpr u8 OpaqueMeshColorAlpha = 255;
}

namespace helengine::gamecube {
    /// Creates the raster renderer with a shared runtime-model cache.
    GameCubeRasterRenderer::GameCubeRasterRenderer(GameCubeMeshCache* meshCache)
        : MeshCache(meshCache) {
        if (MeshCache == nullptr) {
            throw new ArgumentNullException("meshCache");
        }
    }

    /// Draws one extracted camera frame through GX and reports whether this frame claimed scene presentation ownership.
    bool GameCubeRasterRenderer::DrawFrame(GameCubeFramePlan* framePlan) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        }

        ConfigurePipeline();
        CameraClearSettings clearSettings = framePlan->Camera->get_ClearSettings();
        GX_SetCopyClear(ResolveClearColor(clearSettings), ResolveClearDepth(clearSettings));
        GX_SetViewport(framePlan->Viewport.X, framePlan->Viewport.Y, framePlan->Viewport.Z, framePlan->Viewport.W, 0.0f, 1.0f);
        GX_SetScissor(
            static_cast<u32>(framePlan->Viewport.X),
            static_cast<u32>(framePlan->Viewport.Y),
            static_cast<u32>(framePlan->Viewport.Z),
            static_cast<u32>(framePlan->Viewport.W));
        GX_InvVtxCache();
        GX_InvalidateTexAll();

        Mtx44 projectionMatrix;
        CopyProjectionMatrixToGx(framePlan->Projection, projectionMatrix);
        GX_LoadProjectionMtx(projectionMatrix, GX_PERSPECTIVE);

        for (int32_t submissionIndex = 0; submissionIndex < framePlan->DrawableSubmissions->get_Count(); submissionIndex++) {
            RenderFrameDrawableSubmission* submission = (*framePlan->DrawableSubmissions)[submissionIndex];
            GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());
            RuntimeSubmesh* runtimeSubmesh = (*runtimeModel->get_Submeshes())[submission->get_SubmeshIndex()];
            Entity* entity = submission->get_Drawable()->get_Parent();

            DrawSubmesh(framePlan, runtimeModel, runtimeSubmesh, entity);
        }

        return true;
    }

    /// Configures the GX state used by the first opaque mesh path.
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
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GX_SetCullMode(GX_CULL_NONE);
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_FALSE);
    }

    /// Converts the authored runtime clear settings into the presented GX clear color.
    GXColor GameCubeRasterRenderer::ResolveClearColor(CameraClearSettings clearSettings) {
        if (!clearSettings.get_ClearColorEnabled()) {
            return GXColor { 0x64, 0x95, 0xED, 0xFF };
        }

        float4 color = clearSettings.get_ClearColor();
        return GXColor {
            static_cast<u8>(color.X * 255.0f),
            static_cast<u8>(color.Y * 255.0f),
            static_cast<u8>(color.Z * 255.0f),
            static_cast<u8>(color.W * 255.0f)
        };
    }

    /// Converts the authored runtime clear depth into GX packed depth.
    uint32_t GameCubeRasterRenderer::ResolveClearDepth(CameraClearSettings clearSettings) {
        if (!clearSettings.get_ClearDepthEnabled()) {
            return 0x00FFFFFF;
        }

        return static_cast<uint32_t>(clearSettings.get_ClearDepth() * 16777215.0f);
    }

    /// Copies one generated affine matrix directly into a GX position matrix without runtime reinterpretation.
    void GameCubeRasterRenderer::CopyAffineMatrixToGx(const float4x4& source, Mtx& destination) {
        destination[0][0] = source.M11;
        destination[0][1] = source.M12;
        destination[0][2] = source.M13;
        destination[0][3] = source.M14;
        destination[1][0] = source.M21;
        destination[1][1] = source.M22;
        destination[1][2] = source.M23;
        destination[1][3] = source.M24;
        destination[2][0] = source.M31;
        destination[2][1] = source.M32;
        destination[2][2] = source.M33;
        destination[2][3] = source.M34;
    }

    /// Copies one generated projection matrix directly into a GX projection matrix without runtime reinterpretation.
    void GameCubeRasterRenderer::CopyProjectionMatrixToGx(const float4x4& source, Mtx44& destination) {
        destination[0][0] = source.M11;
        destination[0][1] = source.M12;
        destination[0][2] = source.M13;
        destination[0][3] = source.M14;
        destination[1][0] = source.M21;
        destination[1][1] = source.M22;
        destination[1][2] = source.M23;
        destination[1][3] = source.M24;
        destination[2][0] = source.M31;
        destination[2][1] = source.M32;
        destination[2][2] = source.M33;
        destination[2][3] = source.M34;
        destination[3][0] = source.M41;
        destination[3][1] = source.M42;
        destination[3][2] = source.M43;
        destination[3][3] = source.M44;
    }

    /// Builds one authored world matrix through the generated platform-adapted float4x4 runtime.
    void GameCubeRasterRenderer::BuildWorldMatrix(Entity* entity, float4x4& worldMatrix) {
        if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        float3 entityScale = entity->get_Scale();
        float4 entityOrientation = entity->get_Orientation();
        float3 entityPosition = entity->get_Position();
        entityOrientation.Normalize();

        float4x4 scaleMatrix;
        float4x4::CreateScale(entityScale.X, entityScale.Y, entityScale.Z, scaleMatrix);

        float4x4 rotationMatrix;
        float4x4::CreateFromQuaternion(entityOrientation, rotationMatrix);

        float4x4 translationMatrix;
        float4x4::CreateTranslation(entityPosition, translationMatrix);

        float4x4 scaledRotationMatrix;
        float4x4::Multiply(scaleMatrix, rotationMatrix, scaledRotationMatrix);
        float4x4::Multiply(scaledRotationMatrix, translationMatrix, worldMatrix);
    }

    /// Builds one authored model-view matrix through the generated platform-adapted float4x4 runtime.
    void GameCubeRasterRenderer::BuildModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        float4x4 worldMatrix;
        BuildWorldMatrix(entity, worldMatrix);
        float4x4::Multiply(worldMatrix, framePlan->View, modelViewMatrix);
    }

    /// Draws one authored runtime submesh through immediate GX triangle submission and the active entity transform.
    void GameCubeRasterRenderer::DrawSubmesh(GameCubeFramePlan* framePlan, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeSubmesh == nullptr) {
            throw new ArgumentNullException("runtimeSubmesh");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        float4x4 modelViewMatrix;
        BuildModelViewMatrix(framePlan, entity, modelViewMatrix);

        Mtx nativeModelViewMatrix;
        CopyAffineMatrixToGx(modelViewMatrix, nativeModelViewMatrix);
        GX_LoadPosMtxImm(nativeModelViewMatrix, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, runtimeSubmesh->get_IndexCount());
        for (int32_t indexOffset = 0; indexOffset < runtimeSubmesh->get_IndexCount(); indexOffset++) {
            const uint32_t positionIndex = runtimeModel->Uses32BitIndices
                ? (*runtimeModel->Indices32)[runtimeSubmesh->get_IndexStart() + indexOffset]
                : (*runtimeModel->Indices16)[runtimeSubmesh->get_IndexStart() + indexOffset];
            const float3 localPosition = (*runtimeModel->Positions)[positionIndex];
            GX_Position3f32(localPosition.X, localPosition.Y, localPosition.Z);
            GX_Color4u8(OpaqueMeshColorRed, OpaqueMeshColorGreen, OpaqueMeshColorBlue, OpaqueMeshColorAlpha);
        }
        GX_End();
    }
}
