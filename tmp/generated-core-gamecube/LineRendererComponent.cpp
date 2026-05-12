#ifdef DrawText
#undef DrawText
#endif
#include "LineRendererComponent.hpp"

::Entity* LineRendererComponent::get_Parent()
{
return this->Component::get_Parent();
}

void LineRendererComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

