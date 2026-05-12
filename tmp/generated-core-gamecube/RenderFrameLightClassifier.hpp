#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LightComponent;
class RenderFrameLightSubmission;

#include "runtime/native_exceptions.hpp"
#include "LightComponent.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "LightComponent.hpp"

class RenderFrameLightClassifier
{
public:
    virtual ~RenderFrameLightClassifier() = default;

    ::RenderFrameLightSubmission* Classify(::LightComponent* light);
};
