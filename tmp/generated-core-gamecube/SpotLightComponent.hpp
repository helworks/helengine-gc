#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LightComponent;

#include "LightComponent.hpp"

class SpotLightComponent : public LightComponent
{
public:
    virtual ~SpotLightComponent() = default;

    float InnerConeAngleDegrees;

    float get_InnerConeAngleDegrees();
    void set_InnerConeAngleDegrees(float value);

    float OuterConeAngleDegrees;

    float get_OuterConeAngleDegrees();
    void set_OuterConeAngleDegrees(float value);

    float Range;

    float get_Range();
    void set_Range(float value);

    SpotLightComponent();

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
