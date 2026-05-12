#ifdef DrawText
#undef DrawText
#endif
#include "UpdateComponent.hpp"
#include "Component.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "ComponentExecutionPolicy.hpp"

UpdateComponent::UpdateComponent() : updateOrder()
{
}

uint8_t UpdateComponent::get_UpdateOrder()
{
return this->updateOrder;}

void UpdateComponent::set_UpdateOrder(uint8_t value)
{
    if (this->updateOrder != value)
    {
    if (Parent != nullptr && Parent->get_IsHierarchyEnabled() && ComponentExecutionPolicy::ShouldRunComponentLifecycle(this, Parent))
    {
Core::get_Instance()->get_ObjectManager()->RemoveFromUpdate(this, this->updateOrder);
this->updateOrder = value;
Core::get_Instance()->get_ObjectManager()->RegisterForUpdate(this);
    }
else {
this->updateOrder = value;
}
    }
}

void UpdateComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled() && ComponentExecutionPolicy::ShouldRunComponentLifecycle(this, entity))
    {
Core::get_Instance()->get_ObjectManager()->RegisterForUpdate(this);
    }
}

void UpdateComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!ComponentExecutionPolicy::ShouldRunComponentLifecycle(this, Parent))
    {
return;    }
    if (newEnabled)
    {
Core::get_Instance()->get_ObjectManager()->RegisterForUpdate(this);
    }
else {
Core::get_Instance()->get_ObjectManager()->RemoveFromUpdate(this, this->updateOrder);
}
}

void UpdateComponent::Update()
{
}

::Entity* UpdateComponent::get_Parent()
{
return this->Component::get_Parent();
}

void UpdateComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

