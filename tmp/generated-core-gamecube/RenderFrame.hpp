#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class CameraComponent;
class RenderFrameDrawableSubmission;
class RenderFrameLightSubmission;
class RenderFrameShadowCasterSubmission;

#include "CameraComponent.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"

class RenderFrame
{
public:
    virtual ~RenderFrame() = default;

    ::CameraComponent* Camera;

    ::CameraComponent* get_Camera();

    List<::RenderFrameDrawableSubmission*>* DrawableSubmissions;

    List<::RenderFrameDrawableSubmission*>* get_DrawableSubmissions();

    bool get_HasTransparentDrawables();

    List<::RenderFrameLightSubmission*>* LightSubmissions;

    List<::RenderFrameLightSubmission*>* get_LightSubmissions();

    List<::RenderFrameShadowCasterSubmission*>* ShadowCasterSubmissions;

    List<::RenderFrameShadowCasterSubmission*>* get_ShadowCasterSubmissions();

    RenderFrame(::CameraComponent* camera, List<::RenderFrameDrawableSubmission*>* drawableSubmissions, List<::RenderFrameLightSubmission*>* lightSubmissions, List<::RenderFrameShadowCasterSubmission*>* shadowCasterSubmissions);
};
