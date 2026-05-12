#ifdef DrawText
#undef DrawText
#endif
#include "DirectionalShadowTowerSpinComponent.hpp"
#include "UpdateComponent.hpp"
#include "float4.hpp"
#include "Core.hpp"

DirectionalShadowTowerSpinComponent::DirectionalShadowTowerSpinComponent() : AngularSpeedRadians(), BaseYawRadians()
{
}

std::string DirectionalShadowTowerSpinComponent::SerializedComponentTypeId = "helengine.DirectionalShadowTowerSpinComponent";

float DirectionalShadowTowerSpinComponent::get_AngularSpeedRadians()
{
return this->AngularSpeedRadians;
}

void DirectionalShadowTowerSpinComponent::set_AngularSpeedRadians(float value)
{
this->AngularSpeedRadians = value;
}

float DirectionalShadowTowerSpinComponent::get_BaseYawRadians()
{
return this->BaseYawRadians;
}

void DirectionalShadowTowerSpinComponent::set_BaseYawRadians(float value)
{
this->BaseYawRadians = value;
}

void DirectionalShadowTowerSpinComponent::Update()
{
UpdateComponent::Update();
const double yawRadians = this->BaseYawRadians + (this->AngularSpeedRadians * Core::get_Instance()->get_TotalElapsedSeconds());
::float4 orientation;
float4::CreateFromYawPitchRoll(static_cast<float>(yawRadians), 0.0f, 0.0f, orientation);
orientation.Normalize();
Parent->set_LocalOrientation(orientation);
}

uint8_t DirectionalShadowTowerSpinComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void DirectionalShadowTowerSpinComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* DirectionalShadowTowerSpinComponent::get_Parent()
{
return this->Component::get_Parent();
}

void DirectionalShadowTowerSpinComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

