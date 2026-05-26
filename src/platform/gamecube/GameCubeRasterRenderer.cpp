#include "platform/gamecube/GameCubeRasterRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#include <ogc/system.h>

#include "byte4.hpp"
#include "CameraClearSettings.hpp"
#include "CameraComponent.hpp"
#include "ClipRectComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "IDrawable3D.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "FontAsset.hpp"
#include "FontChar.hpp"
#include "FontInfo.hpp"
#include "LightComponent.hpp"
#include "LightType.hpp"
#include "MaterialRenderState.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeSubmesh.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "TextLayoutUtils.hpp"
#include "int2.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"
#include "platform/gamecube/GameCubeRuntimeMaterial.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "platform/gamecube/GameCubeRuntimeTexture.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

namespace {
    constexpr u8 OpaqueMeshColorRed = 255;
    constexpr u8 OpaqueMeshColorGreen = 255;
    constexpr u8 OpaqueMeshColorBlue = 255;
    constexpr u8 OpaqueMeshColorAlpha = 255;
}

namespace helengine::gamecube {
    /// Creates the raster renderer with a shared runtime-model cache.
    GameCubeRasterRenderer::GameCubeRasterRenderer(GameCubeMeshCache* meshCache)
        : MeshCache(meshCache)
        , HasLoggedFirstTexturedDraw(false) {
        if (MeshCache == nullptr) {
            throw new ArgumentNullException("meshCache");
        }
    }

    /// Draws one extracted camera frame through GX and reports whether this frame claimed scene presentation ownership.
    bool GameCubeRasterRenderer::DrawFrame(GameCubeFramePlan* framePlan) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        }

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
        const float viewportHeight = framePlan->Viewport.W > 0.0f ? framePlan->Viewport.W : 1.0f;
        const float aspectRatio = framePlan->Viewport.Z / viewportHeight;
        guPerspective(
            projectionMatrix,
            45.0f,
            aspectRatio,
            framePlan->Camera->get_NearPlaneDistance(),
            framePlan->Camera->get_FarPlaneDistance());
        GX_LoadProjectionMtx(projectionMatrix, GX_PERSPECTIVE);

        if (framePlan->DrawableSubmissions->get_Count() <= 0) {
            return true;
        }

        for (int32_t submissionIndex = 0; submissionIndex < framePlan->DrawableSubmissions->get_Count(); submissionIndex++) {
            RenderFrameDrawableSubmission* submission = (*framePlan->DrawableSubmissions)[submissionIndex];
            if (submission == nullptr || submission->get_Drawable() == nullptr) {
                continue;
            }

            GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());
            if (runtimeModel == nullptr) {
                throw new InvalidOperationException("GameCube mesh cache must resolve runtime models for extracted drawable submissions.");
            }

            Array<RuntimeSubmesh*>* submeshes = runtimeModel->get_Submeshes();
            if (submeshes == nullptr || submeshes == Array<RuntimeSubmesh*>::Empty()) {
                throw new InvalidOperationException("GameCube runtime models must provide runtime submesh metadata.");
            }

            const int32_t submeshIndex = submission->get_SubmeshIndex();
            if (submeshIndex < 0 || submeshIndex >= submeshes->get_Length()) {
                throw new InvalidOperationException("GameCube drawable submission submesh index is outside the runtime model submesh range.");
            }

            Entity* entity = submission->get_Drawable()->get_Parent();
            if (entity == nullptr) {
                throw new InvalidOperationException("GameCube drawable submissions require a parent entity.");
            }

            DrawSubmesh(framePlan, submission, runtimeModel, (*submeshes)[submeshIndex], entity);
        }

        return true;
    }

    /// Draws the captured 2D overlay drawables for the current frame through GX.
    void GameCubeRasterRenderer::Render2D(const GameCubeRenderManager2D& renderManager2D, uint16_t frameWidth, uint16_t frameHeight) {
        if (!renderManager2D.HasCapturedDrawables()) {
            return;
        }

        Mtx44 projectionMatrix;
        guOrtho(
            projectionMatrix,
            0.0f,
            static_cast<float>(frameHeight),
            0.0f,
            static_cast<float>(frameWidth),
            -1.0f,
            1.0f);
        GX_LoadProjectionMtx(projectionMatrix, GX_ORTHOGRAPHIC);

        Mtx identityMatrix = {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f }
        };
        GX_LoadPosMtxImm(identityMatrix, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);

        for (const GameCubeRoundedRectDrawCommand& command : renderManager2D.GetRoundedRectQueue()) {
            RenderRoundedRect2D(command, frameWidth, frameHeight);
        }

        for (const GameCubeSpriteDrawCommand& command : renderManager2D.GetSpriteQueue()) {
            RenderSprite2D(command, frameWidth, frameHeight);
        }

        for (const GameCubeTextDrawCommand& command : renderManager2D.GetTextQueue()) {
            RenderText2D(command, frameWidth, frameHeight);
        }

        GX_SetScissor(0U, 0U, frameWidth, frameHeight);
    }

    /// Configures the GX state used by the current opaque mesh path.
    void GameCubeRasterRenderer::ConfigurePipeline(bool useTexturedBranch, bool useIndexedGeometry) {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, useIndexedGeometry ? GX_INDEX16 : GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_TEX0, useTexturedBranch ? (useIndexedGeometry ? GX_INDEX16 : GX_DIRECT) : GX_NONE);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GX_SetNumChans(1);
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
        GX_SetNumTexGens(useTexturedBranch ? 1 : 0);
        if (useTexturedBranch) {
            GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        }
        GX_SetNumTevStages(1);
        GX_SetTevOrder(GX_TEVSTAGE0, useTexturedBranch ? GX_TEXCOORD0 : GX_TEXCOORDNULL, useTexturedBranch ? GX_TEXMAP0 : GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, useTexturedBranch ? GX_MODULATE : GX_PASSCLR);
        GX_SetCullMode(GX_CULL_FRONT);
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_FALSE);
    }

    /// Configures the GX state used by the indexed lit mesh path with GX fixed-function lighting enabled.
    void GameCubeRasterRenderer::ConfigureLitPipeline(bool useTexturedBranch, bool useIndexedGeometry) {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, useIndexedGeometry ? GX_INDEX16 : GX_DIRECT);
        GX_SetVtxDesc(GX_VA_NRM, useIndexedGeometry ? GX_INDEX16 : GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_NONE);
        GX_SetVtxDesc(GX_VA_TEX0, useTexturedBranch ? (useIndexedGeometry ? GX_INDEX16 : GX_DIRECT) : GX_NONE);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GX_SetNumChans(1);
        GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
        GX_SetNumTexGens(useTexturedBranch ? 1 : 0);
        if (useTexturedBranch) {
            GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        }
        GX_SetNumTevStages(1);
        GX_SetTevOrder(GX_TEVSTAGE0, useTexturedBranch ? GX_TEXCOORD0 : GX_TEXCOORDNULL, useTexturedBranch ? GX_TEXMAP0 : GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, useTexturedBranch ? GX_MODULATE : GX_PASSCLR);
        GX_SetCullMode(GX_CULL_FRONT);
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_FALSE);
    }

    /// Binds the cached mesh arrays used by the indexed GameCube draw path.
    void GameCubeRasterRenderer::BindCachedMeshArrays(GameCubeCachedMeshData* cachedMeshData, bool useTexturedBranch) {
        if (cachedMeshData == nullptr) {
            throw new ArgumentNullException("cachedMeshData");
        } else if (cachedMeshData->PackedPositions == nullptr || cachedMeshData->PackedPositions == Array<GameCubePackedPosition3>::Empty() || cachedMeshData->PackedPositions->Length == 0) {
            throw new InvalidOperationException("GameCube cached mesh arrays must contain cached positions.");
        }

        GX_SetArray(GX_VA_POS, &(*cachedMeshData->PackedPositions)[0], sizeof(GameCubePackedPosition3));
        if (cachedMeshData->HasNormals) {
            if (cachedMeshData->PackedNormals == nullptr || cachedMeshData->PackedNormals == Array<GameCubePackedNormal3>::Empty() || cachedMeshData->PackedNormals->Length == 0) {
                throw new InvalidOperationException("GameCube lit cached mesh arrays require packed normals.");
            }

            GX_SetArray(GX_VA_NRM, &(*cachedMeshData->PackedNormals)[0], sizeof(GameCubePackedNormal3));
        }

        if (useTexturedBranch) {
            if (!cachedMeshData->HasTexCoords || cachedMeshData->PackedTexCoords == nullptr || cachedMeshData->PackedTexCoords == Array<GameCubePackedTexCoord2>::Empty() || cachedMeshData->PackedTexCoords->Length == 0) {
                throw new InvalidOperationException("GameCube textured cached mesh arrays require cached texture coordinates.");
            }

            GX_SetArray(GX_VA_TEX0, &(*cachedMeshData->PackedTexCoords)[0], sizeof(GameCubePackedTexCoord2));
        }
    }

    /// Configures one GX directional-light state block from the extracted render-frame lighting inputs.
    void GameCubeRasterRenderer::ConfigureDirectionalLight(GameCubeFramePlan* framePlan, GXLightObj& lightObject, GXColor& ambientColor, bool& hasDirectionalLight) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        }

        float3 ambientRgb(0.0f, 0.0f, 0.0f);
        float3 directionalRgb(0.0f, 0.0f, 0.0f);
        float3 directionalDirection(0.0f, 0.0f, -1.0f);
        hasDirectionalLight = false;

        for (int32_t lightIndex = 0; lightIndex < framePlan->LightSubmissions->get_Count(); lightIndex++) {
            RenderFrameLightSubmission* submission = (*framePlan->LightSubmissions)[lightIndex];
            if (submission == nullptr) {
                continue;
            }

            LightComponent* light = submission->get_Light();
            if (light == nullptr) {
                continue;
            }

            float4 lightColor = light->get_Color();
            float3 rgb = float3(lightColor.X, lightColor.Y, lightColor.Z) * light->get_Intensity();
            if (submission->get_LightType() == LightType::Ambient) {
                ambientRgb = ambientRgb + rgb;
                continue;
            } else if (submission->get_LightType() != LightType::Directional || hasDirectionalLight) {
                continue;
            }

            Entity* lightEntity = light->get_Parent();
            if (lightEntity == nullptr) {
                continue;
            }

            float4 lightOrientation = lightEntity->get_Orientation();
            lightOrientation.Normalize();
            directionalDirection = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), lightOrientation) * -1.0f;
            directionalDirection = TransformDirectionToViewSpace(float3::Normalize(directionalDirection), framePlan->View);
            directionalRgb = rgb;
            hasDirectionalLight = true;
        }

        ambientColor = ConvertLightingColorToGx(ambientRgb);
        GX_SetChanAmbColor(GX_COLOR0A0, ambientColor);

        if (!hasDirectionalLight) {
            GX_InitLightDir(&lightObject, 0.0f, 0.0f, -1.0f);
            GX_InitLightColor(&lightObject, GXColor { 0x00, 0x00, 0x00, 0xFF });
            GX_LoadLightObj(&lightObject, GX_LIGHT0);
            return;
        }

        GX_InitLightDir(&lightObject, directionalDirection.X, directionalDirection.Y, directionalDirection.Z);
        GX_InitLightColor(&lightObject, ConvertLightingColorToGx(directionalRgb));
        GX_LoadLightObj(&lightObject, GX_LIGHT0);
    }

    /// Configures the GX state used by the current 2D overlay path.
    void GameCubeRasterRenderer::Configure2DPipeline(bool useTexturedBranch) {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_TEX0, useTexturedBranch ? GX_DIRECT : GX_NONE);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GX_SetNumChans(1);
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
        GX_SetNumTexGens(useTexturedBranch ? 1 : 0);
        if (useTexturedBranch) {
            GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        }
        GX_SetNumTevStages(1);
        GX_SetTevOrder(GX_TEVSTAGE0, useTexturedBranch ? GX_TEXCOORD0 : GX_TEXCOORDNULL, useTexturedBranch ? GX_TEXMAP0 : GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, useTexturedBranch ? GX_MODULATE : GX_PASSCLR);
        GX_SetCullMode(GX_CULL_NONE);
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_TRUE);
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
        destination[0][1] = source.M21;
        destination[0][2] = source.M31;
        destination[0][3] = source.M41;
        destination[1][0] = source.M12;
        destination[1][1] = source.M22;
        destination[1][2] = source.M32;
        destination[1][3] = source.M42;
        destination[2][0] = source.M13;
        destination[2][1] = source.M23;
        destination[2][2] = source.M33;
        destination[2][3] = source.M43;
    }

    /// Loads one GX normal matrix derived from the current authored model-view transform so fixed-function lighting stays in view space.
    void GameCubeRasterRenderer::LoadNormalMatrix(const Mtx& modelViewMatrix) {
        Mtx sourceModelViewMatrix;
        std::memcpy(sourceModelViewMatrix, modelViewMatrix, sizeof(Mtx));
        Mtx inverseModelViewMatrix;
        if (guMtxInverse(sourceModelViewMatrix, inverseModelViewMatrix) == 0U) {
            throw new InvalidOperationException("GameCube lit rendering requires an invertible model-view matrix.");
        }

        Mtx normalMatrix;
        guMtxTranspose(inverseModelViewMatrix, normalMatrix);
        GX_LoadNrmMtxImm(normalMatrix, GX_PNMTX0);
    }

    /// Resolves whether one submission should use the lit branch for the current checkpoint.
    bool GameCubeRasterRenderer::UsesLitBranch(RenderFrameDrawableSubmission* submission) {
        if (submission == nullptr) {
            throw new ArgumentNullException("submission");
        }

        RuntimeMaterial* material = submission->get_Material();
        if (material == nullptr) {
            throw new InvalidOperationException("GameCube drawable submission requires a runtime material.");
        }

        if (material->get_LightingModel() == RuntimeMaterialLightingModel::Unlit) {
            return false;
        }

        return material->get_LightingModel() == RuntimeMaterialLightingModel::MetalRoughPbr;
    }

    /// Resolves one GameCube-native runtime texture from the current material graph when present.
    GameCubeRuntimeTexture* GameCubeRasterRenderer::ResolveBoundTexture(GameCubeRuntimeMaterial* material) {
        if (material == nullptr) {
            throw new ArgumentNullException("material");
        }

        RuntimeTexture* runtimeTexture = material->ResolveTexture();
        if (runtimeTexture == nullptr) {
            return nullptr;
        }

        GameCubeRuntimeTexture* gameCubeRuntimeTexture = static_cast<GameCubeRuntimeTexture*>(runtimeTexture);
        if (gameCubeRuntimeTexture == nullptr || !gameCubeRuntimeTexture->HasNativeTextureObject()) {
            return nullptr;
        }

        return gameCubeRuntimeTexture;
    }

    /// Maps the shared material cull-mode contract onto GX, which interprets front and back winding in reverse.
    u8 GameCubeRasterRenderer::ResolveGxCullMode(MaterialCullMode cullMode) {
        switch (cullMode) {
            case MaterialCullMode::None:
                return GX_CULL_NONE;

            case MaterialCullMode::Back:
                return GX_CULL_FRONT;

            case MaterialCullMode::Front:
                return GX_CULL_BACK;
        }

        throw new InvalidOperationException("Unsupported material cull mode for GameCube GX submission.");
    }

    /// Converts a normalized RGB lighting value into a GX color with full alpha.
    GXColor GameCubeRasterRenderer::ConvertLightingColorToGx(float3 color) {
        return GXColor {
            static_cast<u8>(std::clamp(color.X, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(std::clamp(color.Y, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(std::clamp(color.Z, 0.0f, 1.0f) * 255.0f),
            0xFF
        };
    }

    /// Converts one shared-engine byte color into a GX color.
    GXColor GameCubeRasterRenderer::ConvertByteColorToGx(const byte4& color) {
        return GXColor {
            color.X,
            color.Y,
            color.Z,
            color.W
        };
    }

    /// Transforms one world-space direction through the frame-plan view rotation so GX lighting receives a view-space light direction.
    float3 GameCubeRasterRenderer::TransformDirectionToViewSpace(const float3& direction, const float4x4& viewMatrix) {
        float3 transformedDirection;
        transformedDirection.X = (direction.X * viewMatrix.M11) + (direction.Y * viewMatrix.M21) + (direction.Z * viewMatrix.M31);
        transformedDirection.Y = (direction.X * viewMatrix.M12) + (direction.Y * viewMatrix.M22) + (direction.Z * viewMatrix.M32);
        transformedDirection.Z = (direction.X * viewMatrix.M13) + (direction.Y * viewMatrix.M23) + (direction.Z * viewMatrix.M33);
        return float3::Normalize(transformedDirection);
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
        const float xx = entityOrientation.X * entityOrientation.X;
        const float yy = entityOrientation.Y * entityOrientation.Y;
        const float zz = entityOrientation.Z * entityOrientation.Z;
        const float xy = entityOrientation.X * entityOrientation.Y;
        const float xz = entityOrientation.X * entityOrientation.Z;
        const float yz = entityOrientation.Y * entityOrientation.Z;
        const float xw = entityOrientation.X * entityOrientation.W;
        const float yw = entityOrientation.Y * entityOrientation.W;
        const float zw = entityOrientation.Z * entityOrientation.W;

        worldMatrix.M11 = (1.0f - (2.0f * (yy + zz))) * entityScale.X;
        worldMatrix.M12 = (2.0f * (xy + zw)) * entityScale.X;
        worldMatrix.M13 = (2.0f * (xz - yw)) * entityScale.X;
        worldMatrix.M14 = 0.0f;
        worldMatrix.M21 = (2.0f * (xy - zw)) * entityScale.Y;
        worldMatrix.M22 = (1.0f - (2.0f * (zz + xx))) * entityScale.Y;
        worldMatrix.M23 = (2.0f * (yz + xw)) * entityScale.Y;
        worldMatrix.M24 = 0.0f;
        worldMatrix.M31 = (2.0f * (xz + yw)) * entityScale.Z;
        worldMatrix.M32 = (2.0f * (yz - xw)) * entityScale.Z;
        worldMatrix.M33 = (1.0f - (2.0f * (yy + xx))) * entityScale.Z;
        worldMatrix.M34 = 0.0f;
        worldMatrix.M41 = entityPosition.X;
        worldMatrix.M42 = entityPosition.Y;
        worldMatrix.M43 = entityPosition.Z;
        worldMatrix.M44 = 1.0f;
    }

    /// Multiplies two row-vector matrices using the shared engine convention expected by the GameCube raster path.
    void GameCubeRasterRenderer::MultiplyMatrices(const float4x4& left, const float4x4& right, float4x4& result) {
        result.M11 = (((left.M11 * right.M11) + (left.M12 * right.M21)) + (left.M13 * right.M31)) + (left.M14 * right.M41);
        result.M12 = (((left.M11 * right.M12) + (left.M12 * right.M22)) + (left.M13 * right.M32)) + (left.M14 * right.M42);
        result.M13 = (((left.M11 * right.M13) + (left.M12 * right.M23)) + (left.M13 * right.M33)) + (left.M14 * right.M43);
        result.M14 = (((left.M11 * right.M14) + (left.M12 * right.M24)) + (left.M13 * right.M34)) + (left.M14 * right.M44);
        result.M21 = (((left.M21 * right.M11) + (left.M22 * right.M21)) + (left.M23 * right.M31)) + (left.M24 * right.M41);
        result.M22 = (((left.M21 * right.M12) + (left.M22 * right.M22)) + (left.M23 * right.M32)) + (left.M24 * right.M42);
        result.M23 = (((left.M21 * right.M13) + (left.M22 * right.M23)) + (left.M23 * right.M33)) + (left.M24 * right.M43);
        result.M24 = (((left.M21 * right.M14) + (left.M22 * right.M24)) + (left.M23 * right.M34)) + (left.M24 * right.M44);
        result.M31 = (((left.M31 * right.M11) + (left.M32 * right.M21)) + (left.M33 * right.M31)) + (left.M34 * right.M41);
        result.M32 = (((left.M31 * right.M12) + (left.M32 * right.M22)) + (left.M33 * right.M32)) + (left.M34 * right.M42);
        result.M33 = (((left.M31 * right.M13) + (left.M32 * right.M23)) + (left.M33 * right.M33)) + (left.M34 * right.M43);
        result.M34 = (((left.M31 * right.M14) + (left.M32 * right.M24)) + (left.M33 * right.M34)) + (left.M34 * right.M44);
        result.M41 = (((left.M41 * right.M11) + (left.M42 * right.M21)) + (left.M43 * right.M31)) + (left.M44 * right.M41);
        result.M42 = (((left.M41 * right.M12) + (left.M42 * right.M22)) + (left.M43 * right.M32)) + (left.M44 * right.M42);
        result.M43 = (((left.M41 * right.M13) + (left.M42 * right.M23)) + (left.M43 * right.M33)) + (left.M44 * right.M43);
        result.M44 = (((left.M41 * right.M14) + (left.M42 * right.M24)) + (left.M43 * right.M34)) + (left.M44 * right.M44);
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
        MultiplyMatrices(worldMatrix, framePlan->View, modelViewMatrix);
    }

    /// Builds one intentionally wrong model-view matrix used only by the temporary capture simulation.
    void GameCubeRasterRenderer::BuildBadCaptureModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        float4x4 worldMatrix;
        BuildWorldMatrix(entity, worldMatrix);
        MultiplyMatrices(framePlan->View, worldMatrix, modelViewMatrix);
    }

    /// Returns whether one of the two simulated capture triangles should be emitted this frame.
    bool GameCubeRasterRenderer::ShouldDrawCaptureTriangle(uint32_t frameIndex, int32_t triangleIndex) const {
        const uint32_t phase = frameIndex % 7200U;
        if (phase >= 2100U && phase < 2280U) {
            return triangleIndex == 0;
        }

        if (phase >= 4200U && phase < 4380U) {
            return triangleIndex == 1;
        }

        if (phase >= 6200U && phase < 6440U) {
            return false;
        }

        return true;
    }

    /// Returns one per-frame scale factor used to simulate the old matrix explosions.
    float GameCubeRasterRenderer::GetCaptureDistortionScale(uint32_t frameIndex) const {
        const uint32_t phase = frameIndex % 7200U;
        if (phase >= 1200U && phase < 1440U) {
            return 1.18f;
        }

        if (phase >= 3200U && phase < 3440U) {
            return 0.84f;
        }

        if (phase >= 5200U && phase < 5440U) {
            return 1.2f;
        }

        return 1.0f;
    }

    /// Emits one temporary capture triangle through the intentionally bad transform path.
    void GameCubeRasterRenderer::DrawCaptureTriangle(GameCubeFramePlan* framePlan, Entity* entity, const float3& localA, const float3& localB, const float3& localC, uint32_t frameIndex, int32_t triangleIndex) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        if (!ShouldDrawCaptureTriangle(frameIndex, triangleIndex)) {
            return;
        }

        const float distortionScale = GetCaptureDistortionScale(frameIndex);
        const float offsetX = triangleIndex == 0 ? -1.2f : -4.8f;
        const float baseOffsetY = triangleIndex == 0 ? 0.0f : 3.2f;
        const float offsetY = ((frameIndex % 1800U) < 900U ? 0.0f : 0.25f) + baseOffsetY;

        float4x4 modelViewMatrix;
        BuildBadCaptureModelViewMatrix(framePlan, entity, modelViewMatrix);
        modelViewMatrix.M41 += offsetX;
        modelViewMatrix.M42 += offsetY;
        modelViewMatrix.M11 *= distortionScale;
        modelViewMatrix.M22 *= distortionScale;
        modelViewMatrix.M33 *= distortionScale;

        Mtx nativeModelViewMatrix;
        CopyAffineMatrixToGx(modelViewMatrix, nativeModelViewMatrix);
        GX_LoadPosMtxImm(nativeModelViewMatrix, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);

        ConfigurePipeline(false, false);
        GX_SetCullMode(GX_CULL_NONE);

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
        GX_Position3f32(localA.X, localA.Y, localA.Z);
        GX_Color4u8(255, 255, 255, 255);
        GX_Position3f32(localB.X, localB.Y, localB.Z);
        GX_Color4u8(255, 255, 255, 255);
        GX_Position3f32(localC.X, localC.Y, localC.Z);
        GX_Color4u8(255, 255, 255, 255);
        GX_End();
    }

    /// Draws one authored runtime submesh through immediate GX triangle submission and the active entity transform.
    void GameCubeRasterRenderer::DrawSubmesh(GameCubeFramePlan* framePlan, RenderFrameDrawableSubmission* submission, GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh, Entity* entity) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (submission == nullptr) {
            throw new ArgumentNullException("submission");
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

        RuntimeMaterial* material = submission->get_Material();
        if (material == nullptr) {
            throw new InvalidOperationException("GameCube drawable submission requires a runtime material.");
        }

        GameCubeRuntimeMaterial* gameCubeRuntimeMaterial = static_cast<GameCubeRuntimeMaterial*>(material);
        if (gameCubeRuntimeMaterial == nullptr) {
            throw new InvalidOperationException("GameCube drawable submission requires a GameCubeRuntimeMaterial.");
        }

        const bool expectsTexture = !gameCubeRuntimeMaterial->GetTextureRelativePath().empty();
        GameCubeRuntimeTexture* boundTexture = expectsTexture
            ? ResolveBoundTexture(gameCubeRuntimeMaterial)
            : nullptr;
        if (expectsTexture && boundTexture == nullptr) {
            throw new InvalidOperationException("GameCube textured material requires one resolved runtime texture.");
        }

        GameCubeCachedMeshData* cachedMeshData = runtimeModel->CachedMeshData;
        if (cachedMeshData == nullptr) {
            throw new InvalidOperationException("GameCube drawable submission requires cached mesh data.");
        }

        const bool useTexturedBranch = boundTexture != nullptr;
        if (useTexturedBranch) {
            GX_LoadTexObj(boundTexture->GetNativeTextureObject(), GX_TEXMAP0);
        }

        const bool useLitBranch = UsesLitBranch(submission);
        if (useTexturedBranch && !HasLoggedFirstTexturedDraw) {
            HasLoggedFirstTexturedDraw = true;
            SYS_Report(
                "[GC] First textured draw material=%s texturePath=%s textureSize=%dx%d lit=%d cachedTexCoords=%d\n",
                material->get_Id().c_str(),
                gameCubeRuntimeMaterial->GetTextureRelativePath().c_str(),
                boundTexture->get_Width(),
                boundTexture->get_Height(),
                useLitBranch ? 1 : 0,
                cachedMeshData->HasTexCoords ? 1 : 0);
        }

        GX_SetCullMode(ResolveGxCullMode(material->get_RenderState()->get_CullMode()));
        if (useLitBranch) {
            LoadNormalMatrix(nativeModelViewMatrix);
            ConfigureLitPipeline(useTexturedBranch, true);
            DrawCachedLitSubmesh(framePlan, entity, gameCubeRuntimeMaterial, cachedMeshData, runtimeSubmesh, useTexturedBranch);
        } else {
            ConfigurePipeline(useTexturedBranch, true);
            BindCachedMeshArrays(cachedMeshData, useTexturedBranch);
            DrawCachedSubmesh(cachedMeshData, runtimeSubmesh, useTexturedBranch);
        }
    }

    /// Draws one unlit or textured cached submesh through indexed GX array submission.
    void GameCubeRasterRenderer::DrawCachedSubmesh(GameCubeCachedMeshData* cachedMeshData, RuntimeSubmesh* runtimeSubmesh, bool useTexturedBranch) {
        if (cachedMeshData == nullptr) {
            throw new ArgumentNullException("cachedMeshData");
        } else if (runtimeSubmesh == nullptr) {
            throw new ArgumentNullException("runtimeSubmesh");
        } else if (cachedMeshData->Indices16 == nullptr || cachedMeshData->Indices16 == Array<uint16_t>::Empty() || cachedMeshData->Indices16->Length == 0) {
            throw new InvalidOperationException("GameCube cached mesh arrays must contain cached 16-bit indices.");
        }

        const int32_t indexStart = runtimeSubmesh->get_IndexStart();
        const int32_t indexCount = runtimeSubmesh->get_IndexCount();
        if (indexStart < 0 || indexCount <= 0 || indexStart + indexCount > cachedMeshData->Indices16->Length) {
            throw new InvalidOperationException("GameCube cached submesh ranges must stay within the cached index buffer.");
        }

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, indexCount);
        for (int32_t indexOffset = 0; indexOffset < indexCount; indexOffset++) {
            const uint16_t cachedIndex = (*cachedMeshData->Indices16)[indexStart + indexOffset];
            GX_Position1x16(cachedIndex);
            GX_Color4u8(OpaqueMeshColorRed, OpaqueMeshColorGreen, OpaqueMeshColorBlue, OpaqueMeshColorAlpha);
            if (useTexturedBranch) {
                GX_TexCoord1x16(cachedIndex);
            }
        }
        GX_End();
    }

    /// Draws one lit cached submesh through the indexed GX lighting path.
    void GameCubeRasterRenderer::DrawCachedLitSubmesh(GameCubeFramePlan* framePlan, Entity* entity, GameCubeRuntimeMaterial* material, GameCubeCachedMeshData* cachedMeshData, RuntimeSubmesh* runtimeSubmesh, bool useTexturedBranch) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        } else if (material == nullptr) {
            throw new ArgumentNullException("material");
        } else if (cachedMeshData == nullptr) {
            throw new ArgumentNullException("cachedMeshData");
        } else if (runtimeSubmesh == nullptr) {
            throw new ArgumentNullException("runtimeSubmesh");
        } else if (!cachedMeshData->HasNormals || cachedMeshData->PackedNormals == nullptr || cachedMeshData->PackedNormals == Array<GameCubePackedNormal3>::Empty() || cachedMeshData->PackedNormals->Length == 0) {
            throw new InvalidOperationException("GameCube lit rendering requires cached packed mesh normals.");
        } else if (cachedMeshData->Indices16 == nullptr || cachedMeshData->Indices16 == Array<uint16_t>::Empty() || cachedMeshData->Indices16->Length == 0) {
            throw new InvalidOperationException("GameCube cached lit meshes must contain cached 16-bit indices.");
        }

        const int32_t indexStart = runtimeSubmesh->get_IndexStart();
        const int32_t indexCount = runtimeSubmesh->get_IndexCount();
        if (indexStart < 0 || indexCount <= 0 || indexStart + indexCount > cachedMeshData->Indices16->Length) {
            throw new InvalidOperationException("GameCube cached lit submesh ranges must stay within the cached index buffer.");
        }

        BindCachedMeshArrays(cachedMeshData, useTexturedBranch);
        GXLightObj lightObject;
        GXColor ambientColor;
        bool hasDirectionalLight = false;
        ConfigureDirectionalLight(framePlan, lightObject, ambientColor, hasDirectionalLight);
        const float3 baseColor = material->GetBaseColor();
        GX_SetChanMatColor(GX_COLOR0A0, ConvertLightingColorToGx(baseColor));

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, indexCount);
        for (int32_t indexOffset = 0; indexOffset < indexCount; indexOffset++) {
            const uint16_t cachedIndex = (*cachedMeshData->Indices16)[indexStart + indexOffset];
            GX_Position1x16(cachedIndex);
            GX_Normal1x16(cachedIndex);
            if (useTexturedBranch) {
                GX_TexCoord1x16(cachedIndex);
            }
        }
        GX_End();
    }

    /// Draws one captured rounded rectangle through the current 2D GX path.
    void GameCubeRasterRenderer::RenderRoundedRect2D(const GameCubeRoundedRectDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight) {
        IRoundedRectDrawable2D* drawable = command.Drawable;
        if (drawable == nullptr || drawable->get_Parent() == nullptr || !drawable->get_Parent()->get_Enabled()) {
            return;
        }

        int2 size = drawable->get_Size();
        if (size.X <= 0 || size.Y <= 0) {
            return;
        }

        float4 clipRect;
        if (TryResolveClipRect(drawable->get_Parent(), clipRect)) {
            ApplyClipScissor(clipRect, frameWidth, frameHeight);
        } else {
            GX_SetScissor(0U, 0U, frameWidth, frameHeight);
        }

        Configure2DPipeline(false);
        const float3 position = drawable->get_Parent()->get_Position();
        const float radius = std::max(0.0f, drawable->get_Radius());
        const float borderThickness = std::max(0.0f, drawable->get_BorderThickness());
        const GXColor fillColor = ConvertByteColorToGx(drawable->get_FillColor());
        const GXColor borderColor = ConvertByteColorToGx(drawable->get_BorderColor());

        if (borderThickness > 0.0f) {
            DrawRoundedPolygon2D(
                position.X,
                position.Y,
                static_cast<float>(size.X),
                static_cast<float>(size.Y),
                radius,
                drawable->get_Corners(),
                borderColor);
        }

        const float innerX = position.X + borderThickness;
        const float innerY = position.Y + borderThickness;
        const float innerWidth = static_cast<float>(size.X) - (borderThickness * 2.0f);
        const float innerHeight = static_cast<float>(size.Y) - (borderThickness * 2.0f);
        if (innerWidth > 0.0f && innerHeight > 0.0f) {
            DrawRoundedPolygon2D(
                innerX,
                innerY,
                innerWidth,
                innerHeight,
                std::max(0.0f, radius - borderThickness),
                drawable->get_Corners(),
                fillColor);
        }
    }

    /// Draws one captured sprite through the current 2D GX path.
    void GameCubeRasterRenderer::RenderSprite2D(const GameCubeSpriteDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight) {
        ISpriteDrawable2D* drawable = command.Drawable;
        if (drawable == nullptr || drawable->get_Parent() == nullptr || !drawable->get_Parent()->get_Enabled()) {
            return;
        }

        RuntimeTexture* runtimeTexture = drawable->get_Texture();
        GameCubeRuntimeTexture* texture = static_cast<GameCubeRuntimeTexture*>(runtimeTexture);
        if (texture == nullptr || !texture->HasNativeTextureObject()) {
            return;
        }

        int2 size = drawable->get_Size();
        const float width = size.X > 0 ? static_cast<float>(size.X) : static_cast<float>(runtimeTexture->get_Width());
        const float height = size.Y > 0 ? static_cast<float>(size.Y) : static_cast<float>(runtimeTexture->get_Height());
        if (width <= 0.0f || height <= 0.0f) {
            return;
        }

        float4 clipRect;
        if (TryResolveClipRect(drawable->get_Parent(), clipRect)) {
            ApplyClipScissor(clipRect, frameWidth, frameHeight);
        } else {
            GX_SetScissor(0U, 0U, frameWidth, frameHeight);
        }

        Configure2DPipeline(true);
        const float3 position = drawable->get_Parent()->get_Position();
        DrawTexturedQuad2D(position.X, position.Y, width, height, drawable->get_SourceRect(), ConvertByteColorToGx(drawable->get_Color()), texture);
    }

    /// Draws one captured text drawable through the current 2D GX path.
    void GameCubeRasterRenderer::RenderText2D(const GameCubeTextDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight) {
        ITextDrawable2D* drawable = command.Drawable;
        if (drawable == nullptr || drawable->get_Parent() == nullptr || !drawable->get_Parent()->get_Enabled()) {
            return;
        }

        FontAsset* font = drawable->get_Font();
        if (font == nullptr || font->get_Texture() == nullptr) {
            return;
        }

        GameCubeRuntimeTexture* texture = static_cast<GameCubeRuntimeTexture*>(font->get_Texture());
        if (texture == nullptr || !texture->HasNativeTextureObject()) {
            return;
        }

        float4 clipRect;
        if (TryResolveClipRect(drawable->get_Parent(), clipRect)) {
            ApplyClipScissor(clipRect, frameWidth, frameHeight);
        } else {
            GX_SetScissor(0U, 0U, frameWidth, frameHeight);
        }

        Configure2DPipeline(true);
        GX_LoadTexObj(texture->GetNativeTextureObject(), GX_TEXMAP0);

        std::string content = drawable->get_Text();
        const double fontScale = std::max(static_cast<double>(drawable->get_FontScale()), 0.0001);
        if (drawable->get_WrapText()) {
            content = TextLayoutUtils::WrapText(content, font, std::max(1, static_cast<int32_t>(std::lround(drawable->get_Size().X / fontScale))));
        }

        const GXColor glyphColor = ConvertByteColorToGx(drawable->get_Color());
        const float3 position = drawable->get_Parent()->get_Position();
        const double baseX = std::round(position.X);
        const double baseY = std::round(position.Y);
        const double lineHeight = std::max(static_cast<double>(font->get_LineHeight()) * fontScale, 1.0);
        double offsetX = 0.0;
        double offsetY = 0.0;

        for (int32_t index = 0; index < static_cast<int32_t>(content.size()); index++) {
            const char character = content[index];
            if (character == '\n') {
                offsetY += lineHeight;
                offsetX = 0.0;
                continue;
            }

            if (character == ' ') {
                offsetX += font->get_FontInfo()->get_SpaceWidth() * fontScale;
                continue;
            }

            FontChar glyph;
            if (!font->get_Characters()->TryGetValue(character, glyph)) {
                continue;
            }

            const double glyphWidth = glyph.SourceRect.Z * font->get_AtlasWidth() * fontScale;
            const double glyphHeight = glyph.SourceRect.W * font->get_AtlasHeight() * fontScale;
            const double snappedLineOffsetY = std::round(offsetY);
            DrawTexturedQuad2D(
                static_cast<float>(baseX + offsetX),
                static_cast<float>(baseY + snappedLineOffsetY + (glyph.OffsetY * fontScale)),
                static_cast<float>(glyphWidth),
                static_cast<float>(glyphHeight),
                glyph.SourceRect,
                glyphColor,
                texture);

            const double advanceWidth = glyph.AdvanceWidth > 0.0f
                ? glyph.AdvanceWidth * fontScale
                : glyphWidth;
            offsetX += advanceWidth;
        }
    }

    /// Emits one untextured screen-space quad in pixel coordinates.
    void GameCubeRasterRenderer::DrawSolidQuad2D(float x, float y, float width, float height, GXColor color) {
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_Position3f32(x + width, y, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_Position3f32(x + width, y + height, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_Position3f32(x, y + height, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_End();
    }

    /// Emits one rounded screen-space convex polygon in pixel coordinates.
    void GameCubeRasterRenderer::DrawRoundedPolygon2D(float x, float y, float width, float height, float radius, RoundedRectCorners corners, GXColor color) {
        if (width <= 0.0f || height <= 0.0f) {
            return;
        }

        std::vector<float2> outline;
        BuildRoundedRectOutline(x, y, width, height, radius, corners, outline);
        if (outline.size() < 3U) {
            return;
        }

        const float centerX = x + (width * 0.5f);
        const float centerY = y + (height * 0.5f);
        GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast<u16>(outline.size() + 2U));
        GX_Position3f32(centerX, centerY, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        for (std::size_t vertexIndex = 0; vertexIndex < outline.size(); vertexIndex++) {
            GX_Position3f32(outline[vertexIndex].X, outline[vertexIndex].Y, 0.0f);
            GX_Color4u8(color.r, color.g, color.b, color.a);
        }
        GX_Position3f32(outline[0].X, outline[0].Y, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_End();
    }

    /// Builds one clockwise rounded-rectangle outline in screen-space coordinates.
    void GameCubeRasterRenderer::BuildRoundedRectOutline(float x, float y, float width, float height, float radius, RoundedRectCorners corners, std::vector<float2>& outline) {
        outline.clear();
        const float clampedRadius = std::min(radius, std::min(width * 0.5f, height * 0.5f));
        const bool topLeftRounded = HasRoundedCorner(corners, RoundedRectCorners::TopLeft) && clampedRadius > 0.0f;
        const bool topRightRounded = HasRoundedCorner(corners, RoundedRectCorners::TopRight) && clampedRadius > 0.0f;
        const bool bottomRightRounded = HasRoundedCorner(corners, RoundedRectCorners::BottomRight) && clampedRadius > 0.0f;
        const bool bottomLeftRounded = HasRoundedCorner(corners, RoundedRectCorners::BottomLeft) && clampedRadius > 0.0f;

        outline.emplace_back(topLeftRounded ? x + clampedRadius : x, y);
        if (topRightRounded) {
            outline.emplace_back(x + width - clampedRadius, y);
            AppendCornerArc(outline, x + width - clampedRadius, y + clampedRadius, clampedRadius, -1.57079632679f, 0.0f, true);
        } else {
            outline.emplace_back(x + width, y);
        }

        if (bottomRightRounded) {
            outline.emplace_back(x + width, y + height - clampedRadius);
            AppendCornerArc(outline, x + width - clampedRadius, y + height - clampedRadius, clampedRadius, 0.0f, 1.57079632679f, true);
        } else {
            outline.emplace_back(x + width, y + height);
        }

        if (bottomLeftRounded) {
            outline.emplace_back(x + clampedRadius, y + height);
            AppendCornerArc(outline, x + clampedRadius, y + height - clampedRadius, clampedRadius, 1.57079632679f, 3.14159265359f, true);
        } else {
            outline.emplace_back(x, y + height);
        }

        if (topLeftRounded) {
            outline.emplace_back(x, y + clampedRadius);
            AppendCornerArc(outline, x + clampedRadius, y + clampedRadius, clampedRadius, 3.14159265359f, 4.71238898038f, true);
        } else {
            outline.emplace_back(x, y);
        }
    }

    /// Appends one clockwise corner arc into the outline builder.
    void GameCubeRasterRenderer::AppendCornerArc(std::vector<float2>& outline, float centerX, float centerY, float radius, float startAngle, float endAngle, bool skipFirstPoint) {
        constexpr int ArcSegments = 8;
        const int startIndex = skipFirstPoint ? 1 : 0;
        for (int segmentIndex = startIndex; segmentIndex <= ArcSegments; segmentIndex++) {
            const float interpolation = static_cast<float>(segmentIndex) / static_cast<float>(ArcSegments);
            const float angle = startAngle + ((endAngle - startAngle) * interpolation);
            outline.emplace_back(
                centerX + (std::cos(angle) * radius),
                centerY + (std::sin(angle) * radius));
        }
    }

    /// Resolves whether the authored corner mask includes one specific corner.
    bool GameCubeRasterRenderer::HasRoundedCorner(RoundedRectCorners corners, RoundedRectCorners corner) const {
        return (static_cast<int32_t>(corners) & static_cast<int32_t>(corner)) != 0;
    }

    /// Emits one textured screen-space quad in pixel coordinates.
    void GameCubeRasterRenderer::DrawTexturedQuad2D(float x, float y, float width, float height, const float4& sourceRect, GXColor color, GameCubeRuntimeTexture* texture) {
        if (texture == nullptr) {
            throw new ArgumentNullException("texture");
        }

        GX_LoadTexObj(texture->GetNativeTextureObject(), GX_TEXMAP0);
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(x, y, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_TexCoord2f32(sourceRect.X, sourceRect.Y);
        GX_Position3f32(x + width, y, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_TexCoord2f32(sourceRect.X + sourceRect.Z, sourceRect.Y);
        GX_Position3f32(x + width, y + height, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_TexCoord2f32(sourceRect.X + sourceRect.Z, sourceRect.Y + sourceRect.W);
        GX_Position3f32(x, y + height, 0.0f);
        GX_Color4u8(color.r, color.g, color.b, color.a);
        GX_TexCoord2f32(sourceRect.X, sourceRect.Y + sourceRect.W);
        GX_End();
    }

    /// Resolves the active clip rectangle for one 2D drawable by intersecting ancestor clip regions.
    bool GameCubeRasterRenderer::TryResolveClipRect(Entity* entity, float4& clipRect) {
        if (entity == nullptr) {
            return false;
        }

        bool hasClip = false;
        Entity* current = entity;
        while (current != nullptr) {
            List<Component*>* components = current->get_Components();
            if (components != nullptr) {
                for (int32_t componentIndex = 0; componentIndex < components->get_Count(); componentIndex++) {
                    ClipRectComponent* clipComponent = dynamic_cast<ClipRectComponent*>((*components)[componentIndex]);
                    if (clipComponent == nullptr) {
                        continue;
                    }

                    float4 currentClipRect = clipComponent->GetClipRect();
                    if (!hasClip) {
                        clipRect = currentClipRect;
                        hasClip = true;
                        continue;
                    }

                    const float left = std::max(clipRect.X, currentClipRect.X);
                    const float top = std::max(clipRect.Y, currentClipRect.Y);
                    const float right = std::min(clipRect.X + clipRect.Z, currentClipRect.X + currentClipRect.Z);
                    const float bottom = std::min(clipRect.Y + clipRect.W, currentClipRect.Y + currentClipRect.W);
                    clipRect = float4(left, top, std::max(0.0f, right - left), std::max(0.0f, bottom - top));
                }
            }

            current = current->get_Parent();
        }

        return hasClip;
    }

    /// Applies one resolved clip rectangle as the active GX scissor state.
    void GameCubeRasterRenderer::ApplyClipScissor(const float4& clipRect, uint16_t frameWidth, uint16_t frameHeight) {
        const int32_t left = std::max(0, static_cast<int32_t>(std::floor(clipRect.X)));
        const int32_t top = std::max(0, static_cast<int32_t>(std::floor(clipRect.Y)));
        const int32_t right = std::min(static_cast<int32_t>(frameWidth), static_cast<int32_t>(std::ceil(clipRect.X + clipRect.Z)));
        const int32_t bottom = std::min(static_cast<int32_t>(frameHeight), static_cast<int32_t>(std::ceil(clipRect.Y + clipRect.W)));
        const u32 width = right > left ? static_cast<u32>(right - left) : 0U;
        const u32 height = bottom > top ? static_cast<u32>(bottom - top) : 0U;
        GX_SetScissor(static_cast<u32>(left), static_cast<u32>(top), width, height);
    }
}
