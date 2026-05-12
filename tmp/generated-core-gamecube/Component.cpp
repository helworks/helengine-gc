#ifdef DrawText
#undef DrawText
#endif
#include "Component.hpp"

Component::Component() : Parent()
{
}

::Entity* Component::get_Parent()
{
return this->Parent;
}

void Component::set_Parent(::Entity* value)
{
this->Parent = value;
}

void Component::AttachToEntity(::Entity* entity)
{
this->set_Parent(entity);
}

void Component::ComponentAdded(::Entity* entity)
{
}

void Component::ComponentRemoved(::Entity* entity)
{
}

void Component::DetachFromEntity()
{
this->set_Parent(nullptr);
}

void Component::ParentEnabledChange(bool newEnabled)
{
}

void Component::ParentStaticChange(bool newEnabled)
{
}

