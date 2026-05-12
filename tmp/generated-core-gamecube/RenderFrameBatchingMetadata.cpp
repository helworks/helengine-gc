#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameBatchingMetadata.hpp"

bool RenderFrameBatchingMetadata::get_IsDynamicEligible()
{
return this->IsDynamicEligible;
}

bool RenderFrameBatchingMetadata::get_IsInstancingEligible()
{
return this->IsInstancingEligible;
}

bool RenderFrameBatchingMetadata::get_IsStaticEligible()
{
return this->IsStaticEligible;
}

RenderFrameBatchingMetadata::RenderFrameBatchingMetadata(bool isStaticEligible, bool isDynamicEligible, bool isInstancingEligible) : IsDynamicEligible(), IsInstancingEligible(), IsStaticEligible()
{
this->IsStaticEligible = isStaticEligible;
this->IsDynamicEligible = isDynamicEligible;
this->IsInstancingEligible = isInstancingEligible;
}

