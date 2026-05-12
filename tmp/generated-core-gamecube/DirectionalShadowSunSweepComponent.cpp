#ifdef DrawText
#undef DrawText
#endif
#include "DirectionalShadowSunSweepComponent.hpp"
#include "UpdateComponent.hpp"
#include "float4.hpp"
#include "Core.hpp"
#include "system/math.hpp"

DirectionalShadowSunSweepComponent::DirectionalShadowSunSweepComponent() : MaxYawRadians(), MinYawRadians(), PitchRadians(), SweepSpeedRadians()
{
}

std::string DirectionalShadowSunSweepComponent::SerializedComponentTypeId = "helengine.DirectionalShadowSunSweepComponent";

float DirectionalShadowSunSweepComponent::get_MaxYawRadians()
{
return this->MaxYawRadians;
}

void DirectionalShadowSunSweepComponent::set_MaxYawRadians(float value)
{
this->MaxYawRadians = value;
}

float DirectionalShadowSunSweepComponent::get_MinYawRadians()
{
return this->MinYawRadians;
}

void DirectionalShadowSunSweepComponent::set_MinYawRadians(float value)
{
this->MinYawRadians = value;
}

float DirectionalShadowSunSweepComponent::get_PitchRadians()
{
return this->PitchRadians;
}

void DirectionalShadowSunSweepComponent::set_PitchRadians(float value)
{
this->PitchRadians = value;
}

float DirectionalShadowSunSweepComponent::get_SweepSpeedRadians()
{
return this->SweepSpeedRadians;
}

void DirectionalShadowSunSweepComponent::set_SweepSpeedRadians(float value)
{
this->SweepSpeedRadians = value;
}

void DirectionalShadowSunSweepComponent::Update()
{
UpdateComponent::Update();
const double normalized = (Math::Sin(Core::get_Instance()->get_TotalElapsedSeconds() * this->SweepSpeedRadians) * 0.5) + 0.5;
const double yawRadians = this->MinYawRadians + ((this->MaxYawRadians - this->MinYawRadians) * normalized);
::float4 orientation;
float4::CreateFromYawPitchRoll(static_cast<float>(yawRadians), this->PitchRadians, 0.0f, orientation);
orientation.Normalize();
Parent->set_LocalOrientation(orientation);
}

uint8_t DirectionalShadowSunSweepComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void DirectionalShadowSunSweepComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* DirectionalShadowSunSweepComponent::get_Parent()
{
return this->Component::get_Parent();
}

void DirectionalShadowSunSweepComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

