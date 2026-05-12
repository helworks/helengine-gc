#ifdef DrawText
#undef DrawText
#endif
#include "RendererBackendCapabilityProfile.hpp"

int32_t RendererBackendCapabilityProfile::get_MaximumShadowedLights()
{
return this->MaximumShadowedLights;
}

int32_t RendererBackendCapabilityProfile::get_MaximumVisibleLights()
{
return this->MaximumVisibleLights;
}

bool RendererBackendCapabilityProfile::get_SupportsDeferredRendering()
{
return this->SupportsDeferredRendering;
}

bool RendererBackendCapabilityProfile::get_SupportsForwardRendering()
{
return this->SupportsForwardRendering;
}

bool RendererBackendCapabilityProfile::get_SupportsHdr()
{
return this->SupportsHdr;
}

bool RendererBackendCapabilityProfile::get_SupportsNormalMaps()
{
return this->SupportsNormalMaps;
}

RendererBackendCapabilityProfile::RendererBackendCapabilityProfile(bool supportsForwardRendering, bool supportsDeferredRendering, bool supportsHdr, bool supportsNormalMaps, int32_t maximumVisibleLights, int32_t maximumShadowedLights) : MaximumShadowedLights(0), MaximumVisibleLights(0), SupportsDeferredRendering(), SupportsForwardRendering(), SupportsHdr(), SupportsNormalMaps()
{
this->SupportsForwardRendering = supportsForwardRendering;
this->SupportsDeferredRendering = supportsDeferredRendering;
this->SupportsHdr = supportsHdr;
this->SupportsNormalMaps = supportsNormalMaps;
this->MaximumVisibleLights = maximumVisibleLights;
this->MaximumShadowedLights = maximumShadowedLights;
}

