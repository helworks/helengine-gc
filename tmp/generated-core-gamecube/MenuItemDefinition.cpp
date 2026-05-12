#ifdef DrawText
#undef DrawText
#endif
#include "MenuItemDefinition.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::MenuActionDefinition* MenuItemDefinition::get_Action()
{
return this->Action;
}

std::string MenuItemDefinition::get_Description()
{
return this->Description;
}

bool MenuItemDefinition::get_Enabled()
{
return this->Enabled;
}

std::string MenuItemDefinition::get_ItemId()
{
return this->ItemId;
}

std::string MenuItemDefinition::get_Label()
{
return this->Label;
}

MenuItemDefinition::MenuItemDefinition(std::string itemId, std::string label, std::string description, bool enabled, ::MenuActionDefinition* action) : Action(), Description(), Enabled(), ItemId(), Label()
{
    if (String::IsNullOrWhiteSpace(itemId))
    {
throw ([&]() {
auto __ctor_arg_000000EE = "Menu item id must be provided.";
auto __ctor_arg_000000EF = "itemId";
return new ArgumentException(__ctor_arg_000000EE, __ctor_arg_000000EF);
})();
    }
    if (String::IsNullOrWhiteSpace(label))
    {
throw ([&]() {
auto __ctor_arg_000000F0 = "Menu item label must be provided.";
auto __ctor_arg_000000F1 = "label";
return new ArgumentException(__ctor_arg_000000F0, __ctor_arg_000000F1);
})();
    }
    if (action == nullptr)
    {
throw new ArgumentNullException("action");
    }
this->ItemId = itemId;
this->Label = label;
this->Description = description;
this->Enabled = enabled;
this->Action = action;
}

