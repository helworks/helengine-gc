#ifdef DrawText
#undef DrawText
#endif
#include "AmbientLightComponent.hpp"

AmbientLightComponent::AmbientLightComponent() : LightComponent(::LightType::Ambient)
{
}

::float4 AmbientLightComponent::get_Color()
{
return this->LightComponent::get_Color();
}

void AmbientLightComponent::set_Color(::float4 value)
{
this->LightComponent::set_Color(value);
}

float AmbientLightComponent::get_Intensity()
{
return this->LightComponent::get_Intensity();
}

void AmbientLightComponent::set_Intensity(float value)
{
this->LightComponent::set_Intensity(value);
}

::LightType AmbientLightComponent::get_LightType()
{
return this->LightComponent::get_LightType();
}

::ShadowMapMode AmbientLightComponent::get_ShadowMapMode()
{
return this->LightComponent::get_ShadowMapMode();
}

void AmbientLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
this->LightComponent::set_ShadowMapMode(value);
}

float AmbientLightComponent::get_ShadowStrength()
{
return this->LightComponent::get_ShadowStrength();
}

void AmbientLightComponent::set_ShadowStrength(float value)
{
this->LightComponent::set_ShadowStrength(value);
}

bool AmbientLightComponent::get_ShadowsEnabled()
{
return this->LightComponent::get_ShadowsEnabled();
}

void AmbientLightComponent::set_ShadowsEnabled(bool value)
{
this->LightComponent::set_ShadowsEnabled(value);
}

::Entity* AmbientLightComponent::get_Parent()
{
return this->Component::get_Parent();
}

void AmbientLightComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

