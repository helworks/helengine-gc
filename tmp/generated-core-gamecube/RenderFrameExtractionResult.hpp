#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RendererBackendCapabilityProfile;
class RenderFrame;

#include "RendererBackendCapabilityProfile.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrame.hpp"

class RenderFrameExtractionResult
{
public:
    virtual ~RenderFrameExtractionResult() = default;

    ::RendererBackendCapabilityProfile* BackendCapabilities;

    ::RendererBackendCapabilityProfile* get_BackendCapabilities();

    List<::RenderFrame*>* Frames;

    List<::RenderFrame*>* get_Frames();

    RenderFrameExtractionResult(List<::RenderFrame*>* frames, ::RendererBackendCapabilityProfile* backendCapabilities);
};
