#ifdef DrawText
#undef DrawText
#endif
#include "DirectionalShadowOrbitComponent.hpp"
#include "UpdateComponent.hpp"
#include "float4.hpp"
#include "Core.hpp"
#include "float3.hpp"
#include "system/math.hpp"

DirectionalShadowOrbitComponent::DirectionalShadowOrbitComponent() : AngularSpeedRadians(), BaseAngleRadians(), OrbitCenter(), OrbitHeight(), OrbitRadius()
{
}

std::string DirectionalShadowOrbitComponent::SerializedComponentTypeId = "helengine.DirectionalShadowOrbitComponent";

float DirectionalShadowOrbitComponent::get_AngularSpeedRadians()
{
return this->AngularSpeedRadians;
}

void DirectionalShadowOrbitComponent::set_AngularSpeedRadians(float value)
{
this->AngularSpeedRadians = value;
}

float DirectionalShadowOrbitComponent::get_BaseAngleRadians()
{
return this->BaseAngleRadians;
}

void DirectionalShadowOrbitComponent::set_BaseAngleRadians(float value)
{
this->BaseAngleRadians = value;
}

::float3 DirectionalShadowOrbitComponent::get_OrbitCenter()
{
return this->OrbitCenter;
}

void DirectionalShadowOrbitComponent::set_OrbitCenter(::float3 value)
{
this->OrbitCenter = value;
}

float DirectionalShadowOrbitComponent::get_OrbitHeight()
{
return this->OrbitHeight;
}

void DirectionalShadowOrbitComponent::set_OrbitHeight(float value)
{
this->OrbitHeight = value;
}

float DirectionalShadowOrbitComponent::get_OrbitRadius()
{
return this->OrbitRadius;
}

void DirectionalShadowOrbitComponent::set_OrbitRadius(float value)
{
this->OrbitRadius = value;
}

void DirectionalShadowOrbitComponent::Update()
{
UpdateComponent::Update();
const double angleRadians = this->BaseAngleRadians + (this->AngularSpeedRadians * Core::get_Instance()->get_TotalElapsedSeconds());
const double x = this->OrbitCenter.X + (Math::Sin(angleRadians) * this->OrbitRadius);
const double z = this->OrbitCenter.Z + (Math::Cos(angleRadians) * this->OrbitRadius);
Parent->set_LocalPosition(::float3(static_cast<float>(x), this->OrbitCenter.Y + this->OrbitHeight, static_cast<float>(z)));
::float4 orientation;
float4::CreateFromYawPitchRoll(static_cast<float>(-angleRadians), 0.0f, 0.0f, orientation);
orientation.Normalize();
Parent->set_LocalOrientation(orientation);
}

uint8_t DirectionalShadowOrbitComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void DirectionalShadowOrbitComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* DirectionalShadowOrbitComponent::get_Parent()
{
return this->Component::get_Parent();
}

void DirectionalShadowOrbitComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

