#ifdef DrawText
#undef DrawText
#endif
#include "PointLightComponent.hpp"

float PointLightComponent::get_Range()
{
return this->Range;
}

void PointLightComponent::set_Range(float value)
{
this->Range = value;
}

PointLightComponent::PointLightComponent() : LightComponent(::LightType::Point), Range()
{
this->set_Range(10.0f);
}

::float4 PointLightComponent::get_Color()
{
return this->LightComponent::get_Color();
}

void PointLightComponent::set_Color(::float4 value)
{
this->LightComponent::set_Color(value);
}

float PointLightComponent::get_Intensity()
{
return this->LightComponent::get_Intensity();
}

void PointLightComponent::set_Intensity(float value)
{
this->LightComponent::set_Intensity(value);
}

::LightType PointLightComponent::get_LightType()
{
return this->LightComponent::get_LightType();
}

::ShadowMapMode PointLightComponent::get_ShadowMapMode()
{
return this->LightComponent::get_ShadowMapMode();
}

void PointLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
this->LightComponent::set_ShadowMapMode(value);
}

float PointLightComponent::get_ShadowStrength()
{
return this->LightComponent::get_ShadowStrength();
}

void PointLightComponent::set_ShadowStrength(float value)
{
this->LightComponent::set_ShadowStrength(value);
}

bool PointLightComponent::get_ShadowsEnabled()
{
return this->LightComponent::get_ShadowsEnabled();
}

void PointLightComponent::set_ShadowsEnabled(bool value)
{
this->LightComponent::set_ShadowsEnabled(value);
}

::Entity* PointLightComponent::get_Parent()
{
return this->Component::get_Parent();
}

void PointLightComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

