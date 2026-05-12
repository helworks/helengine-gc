#ifdef DrawText
#undef DrawText
#endif
#include "MenuPanelComponent.hpp"
#include "runtime/native_string.hpp"

uint8_t MenuPanelComponent::CurrentVersion = 1;

std::string MenuPanelComponent::SerializedComponentTypeId = "helengine.MenuPanelComponent";

std::string MenuPanelComponent::get_PanelId()
{
return this->PanelIdValue;}

void MenuPanelComponent::set_PanelId(std::string value)
{
this->PanelIdValue = value;
}

MenuPanelComponent::MenuPanelComponent() : PanelIdValue()
{
this->PanelIdValue = String::Empty;
}

::Entity* MenuPanelComponent::get_Parent()
{
return this->Component::get_Parent();
}

void MenuPanelComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

