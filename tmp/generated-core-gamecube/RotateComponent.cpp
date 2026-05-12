#ifdef DrawText
#undef DrawText
#endif
#include "RotateComponent.hpp"
#include "UpdateComponent.hpp"
#include "float4.hpp"
#include "float3.hpp"
#include "Entity.hpp"

void RotateComponent::Update()
{
UpdateComponent::Update();
::float4 deltaRotation;
::float3 axis = ::float3(0, 1, 0);
float4::CreateFromAxisAngle(axis, 0.07f, deltaRotation);
::float4 orientation = Parent->get_Orientation();
orientation = deltaRotation * orientation;
orientation.Normalize();
Parent->set_Orientation(orientation);
}

uint8_t RotateComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void RotateComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* RotateComponent::get_Parent()
{
return this->Component::get_Parent();
}

void RotateComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

