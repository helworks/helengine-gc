#ifdef DrawText
#undef DrawText
#endif
#include "CameraRenderSettings.hpp"
#include "runtime/native_exceptions.hpp"

::DepthPrepassMode CameraRenderSettings::get_DepthPrepassMode()
{
return this->DepthPrepassMode;
}

void CameraRenderSettings::set_DepthPrepassMode(::DepthPrepassMode value)
{
this->DepthPrepassMode = value;
}

::PostProcessTier CameraRenderSettings::get_PostProcessTier()
{
return this->PostProcessTier;
}

void CameraRenderSettings::set_PostProcessTier(::PostProcessTier value)
{
this->PostProcessTier = value;
}

float CameraRenderSettings::get_ShadowDistance()
{
return this->ShadowDistance;
}

void CameraRenderSettings::set_ShadowDistance(float value)
{
this->ShadowDistance = value;
}

CameraRenderSettings::CameraRenderSettings() : DepthPrepassMode(), PostProcessTier(), ShadowDistance()
{
this->set_DepthPrepassMode(::DepthPrepassMode::Auto);
this->set_ShadowDistance(50.0f);
this->set_PostProcessTier(::PostProcessTier::High);
}

CameraRenderSettings::CameraRenderSettings(::CameraRenderSettings* other) : DepthPrepassMode(), PostProcessTier(), ShadowDistance()
{
    if (other == nullptr)
    {
throw new ArgumentNullException("other");
    }
this->set_DepthPrepassMode(other->get_DepthPrepassMode());
this->set_ShadowDistance(other->get_ShadowDistance());
this->set_PostProcessTier(other->get_PostProcessTier());
}

