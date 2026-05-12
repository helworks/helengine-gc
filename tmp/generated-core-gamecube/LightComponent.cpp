#ifdef DrawText
#undef DrawText
#endif
#include "LightComponent.hpp"
#include "float4.hpp"

::float4 LightComponent::get_Color()
{
return this->Color;
}

void LightComponent::set_Color(::float4 value)
{
this->Color = value;
}

float LightComponent::get_Intensity()
{
return this->Intensity;
}

void LightComponent::set_Intensity(float value)
{
this->Intensity = value;
}

::LightType LightComponent::get_LightType()
{
return this->LightType;
}

::ShadowMapMode LightComponent::get_ShadowMapMode()
{
return this->ShadowMapMode;
}

void LightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
this->ShadowMapMode = value;
}

float LightComponent::get_ShadowStrength()
{
return this->ShadowStrength;
}

void LightComponent::set_ShadowStrength(float value)
{
this->ShadowStrength = value;
}

bool LightComponent::get_ShadowsEnabled()
{
return this->ShadowsEnabled;
}

void LightComponent::set_ShadowsEnabled(bool value)
{
this->ShadowsEnabled = value;
}

::Entity* LightComponent::get_Parent()
{
return this->Component::get_Parent();
}

void LightComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

LightComponent::LightComponent(::LightType lightType) : Color(), Intensity(), LightType(), ShadowMapMode(), ShadowStrength(), ShadowsEnabled()
{
this->LightType = lightType;
this->set_Color(::float4(1.0f, 1.0f, 1.0f, 1.0f));
this->set_Intensity(1.0f);
this->set_ShadowStrength(1.0f);
this->set_ShadowMapMode(::ShadowMapMode::Auto);
}

