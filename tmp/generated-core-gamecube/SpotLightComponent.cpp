#ifdef DrawText
#undef DrawText
#endif
#include "SpotLightComponent.hpp"

float SpotLightComponent::get_InnerConeAngleDegrees()
{
return this->InnerConeAngleDegrees;
}

void SpotLightComponent::set_InnerConeAngleDegrees(float value)
{
this->InnerConeAngleDegrees = value;
}

float SpotLightComponent::get_OuterConeAngleDegrees()
{
return this->OuterConeAngleDegrees;
}

void SpotLightComponent::set_OuterConeAngleDegrees(float value)
{
this->OuterConeAngleDegrees = value;
}

float SpotLightComponent::get_Range()
{
return this->Range;
}

void SpotLightComponent::set_Range(float value)
{
this->Range = value;
}

SpotLightComponent::SpotLightComponent() : LightComponent(::LightType::Spot), InnerConeAngleDegrees(), OuterConeAngleDegrees(), Range()
{
this->set_Range(10.0f);
this->set_InnerConeAngleDegrees(25.0f);
this->set_OuterConeAngleDegrees(45.0f);
}

::float4 SpotLightComponent::get_Color()
{
return this->LightComponent::get_Color();
}

void SpotLightComponent::set_Color(::float4 value)
{
this->LightComponent::set_Color(value);
}

float SpotLightComponent::get_Intensity()
{
return this->LightComponent::get_Intensity();
}

void SpotLightComponent::set_Intensity(float value)
{
this->LightComponent::set_Intensity(value);
}

::LightType SpotLightComponent::get_LightType()
{
return this->LightComponent::get_LightType();
}

::ShadowMapMode SpotLightComponent::get_ShadowMapMode()
{
return this->LightComponent::get_ShadowMapMode();
}

void SpotLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
this->LightComponent::set_ShadowMapMode(value);
}

float SpotLightComponent::get_ShadowStrength()
{
return this->LightComponent::get_ShadowStrength();
}

void SpotLightComponent::set_ShadowStrength(float value)
{
this->LightComponent::set_ShadowStrength(value);
}

bool SpotLightComponent::get_ShadowsEnabled()
{
return this->LightComponent::get_ShadowsEnabled();
}

void SpotLightComponent::set_ShadowsEnabled(bool value)
{
this->LightComponent::set_ShadowsEnabled(value);
}

::Entity* SpotLightComponent::get_Parent()
{
return this->Component::get_Parent();
}

void SpotLightComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

