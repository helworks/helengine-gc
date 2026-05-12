#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameBatchingMetadata
{
public:
    virtual ~RenderFrameBatchingMetadata() = default;

    bool IsDynamicEligible;

    bool get_IsDynamicEligible();

    bool IsInstancingEligible;

    bool get_IsInstancingEligible();

    bool IsStaticEligible;

    bool get_IsStaticEligible();

    RenderFrameBatchingMetadata(bool isStaticEligible, bool isDynamicEligible, bool isInstancingEligible);
};
