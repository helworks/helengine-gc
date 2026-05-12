#ifdef DrawText
#undef DrawText
#endif
#include "DirectionalLightComponent.hpp"

float DirectionalLightComponent::get_ShadowDistance()
{
return this->ShadowDistance;
}

void DirectionalLightComponent::set_ShadowDistance(float value)
{
this->ShadowDistance = value;
}

DirectionalLightComponent::DirectionalLightComponent() : LightComponent(::LightType::Directional), ShadowDistance()
{
this->set_ShadowsEnabled(true);
this->set_ShadowDistance(DefaultShadowDistance);
}

::float4 DirectionalLightComponent::get_Color()
{
return this->LightComponent::get_Color();
}

void DirectionalLightComponent::set_Color(::float4 value)
{
this->LightComponent::set_Color(value);
}

float DirectionalLightComponent::get_Intensity()
{
return this->LightComponent::get_Intensity();
}

void DirectionalLightComponent::set_Intensity(float value)
{
this->LightComponent::set_Intensity(value);
}

::LightType DirectionalLightComponent::get_LightType()
{
return this->LightComponent::get_LightType();
}

::ShadowMapMode DirectionalLightComponent::get_ShadowMapMode()
{
return this->LightComponent::get_ShadowMapMode();
}

void DirectionalLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
this->LightComponent::set_ShadowMapMode(value);
}

float DirectionalLightComponent::get_ShadowStrength()
{
return this->LightComponent::get_ShadowStrength();
}

void DirectionalLightComponent::set_ShadowStrength(float value)
{
this->LightComponent::set_ShadowStrength(value);
}

bool DirectionalLightComponent::get_ShadowsEnabled()
{
return this->LightComponent::get_ShadowsEnabled();
}

void DirectionalLightComponent::set_ShadowsEnabled(bool value)
{
this->LightComponent::set_ShadowsEnabled(value);
}

::Entity* DirectionalLightComponent::get_Parent()
{
return this->Component::get_Parent();
}

void DirectionalLightComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

float DirectionalLightComponent::DefaultShadowDistance = 50.0f;

