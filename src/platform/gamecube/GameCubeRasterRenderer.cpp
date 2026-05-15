#include "platform/gamecube/GameCubeRasterRenderer.hpp"

#include <ogc/system.h>

#include "CameraClearSettings.hpp"
#include "CameraComponent.hpp"
#include "Entity.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RuntimeSubmesh.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    /// Creates the raster renderer with a shared runtime-model cache.
    GameCubeRasterRenderer::GameCubeRasterRenderer(GameCubeMeshCache* meshCache)
        : MeshCache(meshCache)
        , RasterizedFrameCount(0U) {
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
        GX_SetViewport(framePlan->Viewport.X, framePlan->Viewport.Y, framePlan->Viewport.Z, framePlan->Viewport.W, 0.0f, 1.0f);
        GX_SetScissor(
            static_cast<u32>(framePlan->Viewport.X),
            static_cast<u32>(framePlan->Viewport.Y),
            static_cast<u32>(framePlan->Viewport.Z),
            static_cast<u32>(framePlan->Viewport.W));

        Mtx44 projection;
        BuildProjectionMatrix(framePlan, projection);
        GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

        Mtx viewMatrix;
        BuildViewMatrix(framePlan->Camera, viewMatrix);
        RasterizedFrameCount++;
        DrawProbeFullscreenClear(framePlan);
        return true;

        for (int32_t submissionIndex = 0; submissionIndex < framePlan->DrawableSubmissions->get_Count(); submissionIndex++) {
            RenderFrameDrawableSubmission* submission = (*framePlan->DrawableSubmissions)[submissionIndex];
            GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());
            RuntimeSubmesh* runtimeSubmesh = (*runtimeModel->get_Submeshes())[submission->get_SubmeshIndex()];
            if ((RasterizedFrameCount <= 5U || (RasterizedFrameCount % 60U) == 0U) && submissionIndex == 0) {
                const int32_t firstIndex = runtimeSubmesh->get_IndexStart();
                const uint32_t firstPositionIndex = runtimeModel->Uses32BitIndices
                    ? (*runtimeModel->Indices32)[firstIndex]
                    : (*runtimeModel->Indices16)[firstIndex];
                const float3 firstPosition = (*runtimeModel->Positions)[firstPositionIndex];
                const float3 entityPosition = submission->get_Drawable()->get_Parent()->get_Position();
                SYS_Report(
                    "[GC] Frame %u raster. indexCount=%d firstVertex=(%.3f, %.3f, %.3f) entity=(%.3f, %.3f, %.3f)\n",
                    RasterizedFrameCount,
                    runtimeSubmesh->get_IndexCount(),
                    firstPosition.X,
                    firstPosition.Y,
                    firstPosition.Z,
                    entityPosition.X,
                    entityPosition.Y,
                    entityPosition.Z);
            }
            Mtx worldMatrix;
            BuildWorldMatrix(submission->get_Drawable()->get_Parent(), worldMatrix);

            Mtx modelView;
            guMtxConcat(viewMatrix, worldMatrix, modelView);
            GX_LoadPosMtxImm(modelView, GX_PNMTX0);
            GX_SetCurrentMtx(GX_PNMTX0);
            DrawSubmesh(runtimeModel, runtimeSubmesh);
        }

        return true;
    }

    /// Configures the GX state used by the first unlit triangle path.
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
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GX_SetCullMode(GX_CULL_FRONT);
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
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

    /// Builds one native GX view matrix from the active runtime camera.
    void GameCubeRasterRenderer::BuildViewMatrix(CameraComponent* camera, Mtx& viewMatrix) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        } else if (camera->get_Parent() == nullptr) {
            throw new InvalidOperationException("GameCube GX view construction requires a camera parent entity.");
        }

        const float3 cameraPosition = camera->get_Parent()->get_Position();
        const float4 cameraOrientation = camera->get_Parent()->get_Orientation();
        const float3 cameraForward = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), cameraOrientation);
        const float3 cameraUp = float4::RotateVector(float3(0.0f, 1.0f, 0.0f), cameraOrientation);
        const float3 cameraTarget = cameraPosition + cameraForward;
        guVector nativeCameraPosition { cameraPosition.X, cameraPosition.Y, cameraPosition.Z };
        guVector nativeCameraUp { cameraUp.X, cameraUp.Y, cameraUp.Z };
        guVector nativeCameraTarget { cameraTarget.X, cameraTarget.Y, cameraTarget.Z };
        guLookAt(viewMatrix, &nativeCameraPosition, &nativeCameraUp, &nativeCameraTarget);
    }

    /// Builds one native GX world matrix from the authored entity transform.
    void GameCubeRasterRenderer::BuildWorldMatrix(Entity* entity, Mtx& worldMatrix) {
        if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        const float4 orientation = entity->get_Orientation();
        guQuaternion nativeOrientation { orientation.X, orientation.Y, orientation.Z, orientation.W };
        c_guMtxQuat(worldMatrix, &nativeOrientation);

        const float3 worldScale = entity->get_Scale();
        guMtxScaleApply(worldMatrix, worldMatrix, worldScale.X, worldScale.Y, worldScale.Z);

        const float3 worldPosition = entity->get_Position();
        guMtxTransApply(worldMatrix, worldMatrix, worldPosition.X, worldPosition.Y, worldPosition.Z);
    }

    /// Builds one native GX projection matrix from the active runtime camera and viewport.
    void GameCubeRasterRenderer::BuildProjectionMatrix(GameCubeFramePlan* framePlan, Mtx44& projectionMatrix) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (framePlan->Camera == nullptr) {
            throw new ArgumentNullException("framePlan->Camera");
        }

        const float aspectRatio = framePlan->Viewport.Z / framePlan->Viewport.W;
        guPerspective(
            projectionMatrix,
            45.0f,
            aspectRatio,
            framePlan->Camera->get_NearPlaneDistance(),
            framePlan->Camera->get_FarPlaneDistance());
    }

    /// Draws a fullscreen diagnostic quad that writes a visible clear color directly into the current EFB.
    void GameCubeRasterRenderer::DrawProbeFullscreenClear(GameCubeFramePlan* framePlan) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        }

        Mtx44 projection;
        guOrtho(projection, 0.0f, framePlan->Viewport.W, 0.0f, framePlan->Viewport.Z, -1.0f, 1.0f);
        GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

        Mtx modelView;
        guMtxIdentity(modelView);
        GX_LoadPosMtxImm(modelView, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);
        GX_SetCullMode(GX_CULL_NONE);
        GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GX_Position3f32(0.0f, 0.0f, 0.0f);
        GX_Color4u8(0x64, 0x95, 0xED, 0xFF);
        GX_Position3f32(framePlan->Viewport.Z, 0.0f, 0.0f);
        GX_Color4u8(0x64, 0x95, 0xED, 0xFF);
        GX_Position3f32(0.0f, framePlan->Viewport.W, 0.0f);
        GX_Color4u8(0x64, 0x95, 0xED, 0xFF);
        GX_Position3f32(framePlan->Viewport.Z, framePlan->Viewport.W, 0.0f);
        GX_Color4u8(0x64, 0x95, 0xED, 0xFF);
        GX_End();
    }

    /// Draws one authored runtime submesh through immediate GX triangle submission.
    void GameCubeRasterRenderer::DrawSubmesh(GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeSubmesh == nullptr) {
            throw new ArgumentNullException("runtimeSubmesh");
        }

        const int32_t firstIndex = runtimeSubmesh->get_IndexStart();
        const int32_t indexCount = runtimeSubmesh->get_IndexCount();
        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, indexCount);
        for (int32_t localIndex = 0; localIndex < indexCount; localIndex++) {
            const int32_t sourceIndex = firstIndex + localIndex;
            const uint32_t positionIndex = runtimeModel->Uses32BitIndices
                ? (*runtimeModel->Indices32)[sourceIndex]
                : (*runtimeModel->Indices16)[sourceIndex];
            const float3 position = (*runtimeModel->Positions)[positionIndex];
            GX_Position3f32(position.X, position.Y, position.Z);
            GX_Color4u8(255, 255, 255, 255);
        }
        GX_End();
    }
}
