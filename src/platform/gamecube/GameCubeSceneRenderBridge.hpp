#pragma once

#include "runtime/native_list.hpp"

class CameraComponent;
class IDrawable3D;
class RendererBackendCapabilityProfile;
class float4x4;

namespace helengine::gamecube {
    class GameCubeFramePlan;

    /// Resolves generated runtime state into the first backend-local GameCube frame plan.
    class GameCubeSceneRenderBridge {
    public:
        /// Builds one strict frame plan for the active camera and visible opaque drawables.
        GameCubeFramePlan* BuildFramePlan(RendererBackendCapabilityProfile* capabilities, int32_t targetWidth, int32_t targetHeight);
    private:
        /// Resolves the first enabled runtime camera the GameCube backend is willing to render.
        CameraComponent* ResolveActiveCamera();

        /// Copies the ordered visible 3D queue for one camera into a backend-local list.
        List<IDrawable3D*>* SnapshotVisibleDrawables(CameraComponent* camera);

        /// Builds the authored view matrix from the generated entity transform.
        float4x4 BuildViewMatrix(CameraComponent* camera);
    };
}
