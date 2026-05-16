#include "platform/gamecube/GameCubeSceneRenderBridge.hpp"

#include "AmbientLightComponent.hpp"
#include "CameraComponent.hpp"
#include "CameraProjectionUtils.hpp"
#include "CameraViewportResolver.hpp"
#include "Core.hpp"
#include "DirectionalLightComponent.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "IDrawable3D.hpp"
#include "LightComponent.hpp"
#include "ObjectManager.hpp"
#include "RenderFrame.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "RenderFrameExtractionService.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeRenderQueueSnapshotVisitor.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    /// Builds one strict frame plan for the active camera and visible opaque drawables.
    GameCubeFramePlan* GameCubeSceneRenderBridge::BuildFramePlan(RendererBackendCapabilityProfile* capabilities, int32_t targetWidth, int32_t targetHeight) {
        if (capabilities == nullptr) {
            throw new ArgumentNullException("capabilities");
        } else if (targetWidth < 1) {
            throw new ArgumentOutOfRangeException("targetWidth");
        } else if (targetHeight < 1) {
            throw new ArgumentOutOfRangeException("targetHeight");
        }

        CameraComponent* camera = ResolveActiveCamera();
        List<IDrawable3D*>* drawables = SnapshotVisibleDrawables(camera);
        List<CameraComponent*>* cameras = new List<CameraComponent*>(1);
        cameras->Add(camera);
        List<LightComponent*>* lights = new List<LightComponent*>();
        ObjectManager* objectManager = Core::get_Instance()->get_ObjectManager();
        List<AmbientLightComponent*>* ambientLights = objectManager->get_AmbientLights();
        for (int32_t lightIndex = 0; lightIndex < ambientLights->get_Count(); lightIndex++) {
            lights->Add((*ambientLights)[lightIndex]);
        }

        List<DirectionalLightComponent*>* directionalLights = objectManager->get_DirectionalLights();
        for (int32_t lightIndex = 0; lightIndex < directionalLights->get_Count(); lightIndex++) {
            lights->Add((*directionalLights)[lightIndex]);
        }

        RenderFrameExtractionService extractor;
        RenderFrameExtractionResult* extraction = extractor.Extract(cameras, drawables, lights, capabilities);
        RenderFrame* frame = (*extraction->get_Frames())[0];
        if (frame->get_HasTransparentDrawables()) {
            throw new NotSupportedException("Transparent 3D submissions are not supported in the first GameCube renderer tier.");
        }

        if (frame->get_DrawableSubmissions()->get_Count() == 0) {
            throw new InvalidOperationException("The startup GameCube scene produced no visible 3D drawable submissions.");
        }

        float4 viewport = CameraViewportResolver::ResolveViewport(camera->get_Viewport(), targetWidth, targetHeight);
        float4x4 view = BuildViewMatrix(camera);
        float4x4 projection = CameraProjectionUtils::CreatePerspectiveProjection(camera, static_cast<float>(3.14159265358979323846 / 4.0), viewport.Z / viewport.W);
        float4x4 viewProjection;
        float4x4::Multiply(view, projection, viewProjection);
        return new GameCubeFramePlan(
            camera,
            frame->get_DrawableSubmissions(),
            frame->get_LightSubmissions(),
            viewport,
            view,
            projection,
            viewProjection);
    }

    /// Resolves the first enabled runtime camera the GameCube backend is willing to render.
    CameraComponent* GameCubeSceneRenderBridge::ResolveActiveCamera() {
        List<ICamera*>* cameras = Core::get_Instance()->get_ObjectManager()->get_Cameras();
        for (int32_t index = 0; index < cameras->get_Count(); index++) {
            CameraComponent* camera = he_cpp_try_cast<CameraComponent>((*cameras)[index]);
            if (camera == nullptr || camera->get_Parent() == nullptr || !camera->get_Parent()->get_IsHierarchyEnabled()) {
                continue;
            }

            return camera;
        }

        throw new InvalidOperationException("No active runtime camera is available for the GameCube frame plan.");
    }

    /// Copies the ordered visible 3D queue for one camera into a backend-local list.
    List<IDrawable3D*>* GameCubeSceneRenderBridge::SnapshotVisibleDrawables(CameraComponent* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        GameCubeRenderQueueSnapshotVisitor visitor;
        camera->get_RenderQueue3D()->VisitOrdered(&visitor);
        return visitor.Items;
    }

    /// Builds the authored view matrix from the generated entity transform.
    float4x4 GameCubeSceneRenderBridge::BuildViewMatrix(CameraComponent* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        } else if (camera->get_Parent() == nullptr) {
            throw new InvalidOperationException("GameCube frame-plan construction requires a camera parent entity.");
        }

        float3 cameraPosition = camera->get_Parent()->get_Position();
        float4 cameraOrientation = camera->get_Parent()->get_Orientation();
        float3 cameraForward = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), cameraOrientation);
        float3 cameraUp = float4::RotateVector(float3(0.0f, 1.0f, 0.0f), cameraOrientation);
        float3 cameraTarget = cameraPosition + cameraForward;
        float4x4 view;
        float4x4::CreateLookAt(cameraPosition, cameraTarget, cameraUp, view);
        return view;
    }
}
