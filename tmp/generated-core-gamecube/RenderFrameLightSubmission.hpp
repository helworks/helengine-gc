#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LightComponent;

#include "LightComponent.hpp"
#include "LightType.hpp"

class RenderFrameLightSubmission
{
public:
    virtual ~RenderFrameLightSubmission() = default;

    int32_t Importance;

    int32_t get_Importance();

    ::LightComponent* Light;

    ::LightComponent* get_Light();

    ::LightType get_LightType();

    RenderFrameLightSubmission(::LightComponent* light);

    RenderFrameLightSubmission(::LightComponent* light, int32_t importance);
};
