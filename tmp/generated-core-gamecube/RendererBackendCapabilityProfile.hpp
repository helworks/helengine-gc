#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RendererBackendCapabilityProfile
{
public:
    virtual ~RendererBackendCapabilityProfile() = default;

    int32_t MaximumShadowedLights;

    int32_t get_MaximumShadowedLights();

    int32_t MaximumVisibleLights;

    int32_t get_MaximumVisibleLights();

    bool SupportsDeferredRendering;

    bool get_SupportsDeferredRendering();

    bool SupportsForwardRendering;

    bool get_SupportsForwardRendering();

    bool SupportsHdr;

    bool get_SupportsHdr();

    bool SupportsNormalMaps;

    bool get_SupportsNormalMaps();

    RendererBackendCapabilityProfile(bool supportsForwardRendering, bool supportsDeferredRendering, bool supportsHdr, bool supportsNormalMaps, int32_t maximumVisibleLights, int32_t maximumShadowedLights);
};
