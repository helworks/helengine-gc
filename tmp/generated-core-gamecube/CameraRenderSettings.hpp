#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "DepthPrepassMode.hpp"
#include "PostProcessTier.hpp"

class CameraRenderSettings
{
public:
    virtual ~CameraRenderSettings() = default;

    ::DepthPrepassMode DepthPrepassMode;

    ::DepthPrepassMode get_DepthPrepassMode();
    void set_DepthPrepassMode(::DepthPrepassMode value);

    ::PostProcessTier PostProcessTier;

    ::PostProcessTier get_PostProcessTier();
    void set_PostProcessTier(::PostProcessTier value);

    float ShadowDistance;

    float get_ShadowDistance();
    void set_ShadowDistance(float value);

    CameraRenderSettings();

    CameraRenderSettings(::CameraRenderSettings* other);
};
