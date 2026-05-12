#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class float4;

#include "Component.hpp"
#include "float4.hpp"
#include "LightType.hpp"
#include "ShadowMapMode.hpp"

class LightComponent : public Component
{
public:
    virtual ~LightComponent() = default;

    ::float4 Color;

    ::float4 get_Color();
    void set_Color(::float4 value);

    float Intensity;

    float get_Intensity();
    void set_Intensity(float value);

    ::LightType LightType;

    ::LightType get_LightType();

    ::ShadowMapMode ShadowMapMode;

    ::ShadowMapMode get_ShadowMapMode();
    void set_ShadowMapMode(::ShadowMapMode value);

    float ShadowStrength;

    float get_ShadowStrength();
    void set_ShadowStrength(float value);

    bool ShadowsEnabled;

    bool get_ShadowsEnabled();
    void set_ShadowsEnabled(bool value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
protected:
    LightComponent(::LightType lightType);
};
