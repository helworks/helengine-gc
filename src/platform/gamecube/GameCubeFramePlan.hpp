#pragma once

#include "CameraComponent.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"

namespace helengine::gamecube {
    /// Owns the first-tier reduced execution payload for one GameCube camera frame.
    class GameCubeFramePlan {
    public:
        /// Creates a frame plan with resolved camera state, viewport, and drawable submissions.
        GameCubeFramePlan(
            CameraComponent* camera,
            List<RenderFrameDrawableSubmission*>* drawableSubmissions,
            float4 viewport,
            float4x4 view,
            float4x4 projection,
            float4x4 viewProjection)
            : Camera(camera)
            , DrawableSubmissions(drawableSubmissions)
            , Viewport(viewport)
            , View(view)
            , Projection(projection)
            , ViewProjection(viewProjection) {
        }

        /// Camera that owns the render queues and authored clear settings for this frame.
        CameraComponent* Camera;

        /// Opaque drawable submissions extracted from the generated runtime graph.
        List<RenderFrameDrawableSubmission*>* DrawableSubmissions;

        /// Pixel-space viewport resolved from the authored runtime viewport.
        float4 Viewport;

        /// View matrix derived from the authored runtime camera transform.
        float4x4 View;

        /// Perspective projection derived from the authored runtime camera settings.
        float4x4 Projection;

        /// Cached view-projection matrix reused by the raster renderer.
        float4x4 ViewProjection;
    };
}
