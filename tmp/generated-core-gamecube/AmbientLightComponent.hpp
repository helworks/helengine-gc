#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LightComponent;

#include "LightComponent.hpp"

class AmbientLightComponent : public LightComponent
{
public:
    virtual ~AmbientLightComponent() = default;

    AmbientLightComponent();

    ::float4 get_Color();

    void set_Color(::float4 value);

    float get_Intensity();

    void set_Intensity(float value);

    ::LightType get_LightType();

    ::ShadowMapMode get_ShadowMapMode();

    void set_ShadowMapMode(::ShadowMapMode value);

    float get_ShadowStrength();

    void set_ShadowStrength(float value);

    bool get_ShadowsEnabled();

    void set_ShadowsEnabled(bool value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
};
