#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LightComponent;

#include "LightComponent.hpp"

class DirectionalLightComponent : public LightComponent
{
public:
    virtual ~DirectionalLightComponent() = default;

    float ShadowDistance;

    float get_ShadowDistance();
    void set_ShadowDistance(float value);

    DirectionalLightComponent();

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
private:
    static float DefaultShadowDistance;
};
