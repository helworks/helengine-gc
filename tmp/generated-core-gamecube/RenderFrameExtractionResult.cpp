#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameExtractionResult.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::RendererBackendCapabilityProfile* RenderFrameExtractionResult::get_BackendCapabilities()
{
return this->BackendCapabilities;
}

List<::RenderFrame*>* RenderFrameExtractionResult::get_Frames()
{
return this->Frames;
}

RenderFrameExtractionResult::RenderFrameExtractionResult(List<::RenderFrame*>* frames, ::RendererBackendCapabilityProfile* backendCapabilities) : BackendCapabilities(), Frames()
{
this->Frames = (frames != nullptr ? frames : throw new ArgumentNullException("frames"));
this->BackendCapabilities = (backendCapabilities != nullptr ? backendCapabilities : throw new ArgumentNullException("backendCapabilities"));
}

