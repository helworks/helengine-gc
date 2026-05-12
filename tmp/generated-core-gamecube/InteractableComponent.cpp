#ifdef DrawText
#undef DrawText
#endif
#include "InteractableComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "PointerInteractionSystem.hpp"
#include "runtime/native_event.hpp"

InteractableComponent::InteractableComponent() : CursorEvent(), HoverCursor(), Size()
{
}

::PointerCursorKind InteractableComponent::get_HoverCursor()
{
return this->HoverCursor;
}

void InteractableComponent::set_HoverCursor(::PointerCursorKind value)
{
this->HoverCursor = value;
}

::int2 InteractableComponent::get_Size()
{
return this->Size;
}

void InteractableComponent::set_Size(::int2 value)
{
this->Size = value;
}

void InteractableComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::get_Instance()->get_ObjectManager()->RegisterInteractable(this);
    }
}

void InteractableComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::get_Instance()->get_ObjectManager()->RemoveInteractable(this);
    if (Core::get_Instance() != nullptr && Core::get_Instance()->get_PointerInteractionSystem() != nullptr)
    {
Core::get_Instance()->get_PointerInteractionSystem()->ClearInteractionFor(this);
    }
}

void InteractableComponent::OnCursor(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
this->CursorEvent.Invoke(relPos, delta, state);
}

void InteractableComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::get_Instance()->get_ObjectManager()->RegisterInteractable(this);
    }
else {
Core::get_Instance()->get_ObjectManager()->RemoveInteractable(this);
    if (Core::get_Instance() != nullptr && Core::get_Instance()->get_PointerInteractionSystem() != nullptr)
    {
Core::get_Instance()->get_PointerInteractionSystem()->ClearInteractionFor(this);
    }
}
}

::Entity* InteractableComponent::get_Parent()
{
return this->Component::get_Parent();
}

void InteractableComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

