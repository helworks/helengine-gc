#ifdef DrawText
#undef DrawText
#endif
#include "MenuSelectedDescriptionComponent.hpp"

uint8_t MenuSelectedDescriptionComponent::CurrentVersion = 1;

std::string MenuSelectedDescriptionComponent::SerializedComponentTypeId = "helengine.MenuSelectedDescriptionComponent";

::Entity* MenuSelectedDescriptionComponent::get_Parent()
{
return this->Component::get_Parent();
}

void MenuSelectedDescriptionComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

