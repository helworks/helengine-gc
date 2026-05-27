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
        /// Builds one strict frame plan for the active camera and visible opaque drawables, or returns null when no active camera is available yet.
        GameCubeFramePlan* BuildFramePlan(RendererBackendCapabilityProfile* capabilities, int32_t targetWidth, int32_t targetHeight);

        /// Returns whether the current runtime state exposes at least one enabled camera the GameCube backend can render.
        bool HasActiveCamera();
    private:
        /// Resolves the first enabled runtime camera the GameCube backend is willing to render.
        CameraComponent* ResolveActiveCamera();

        /// Copies the ordered visible 3D queue for one camera into a backend-local list.
        List<IDrawable3D*>* SnapshotVisibleDrawables(CameraComponent* camera);

        /// Builds the authored view matrix from the generated entity transform.
        float4x4 BuildViewMatrix(CameraComponent* camera);

        /// Builds the authored perspective projection matrix using the shared row-vector convention expected by the GameCube raster path.
        float4x4 BuildProjectionMatrix(CameraComponent* camera, float aspectRatio);

        /// Multiplies two row-vector matrices using the shared engine convention expected by the GameCube frame plan.
        void MultiplyMatrices(const float4x4& left, const float4x4& right, float4x4& result);
    };
}
