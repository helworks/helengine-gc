#ifdef DrawText
#undef DrawText
#endif
#include "MenuPanelDefinition.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string MenuPanelDefinition::get_Description()
{
return this->Description;
}

std::string MenuPanelDefinition::get_Heading()
{
return this->Heading;
}

Array<::MenuItemDefinition*>* MenuPanelDefinition::get_Items()
{
return this->Items;
}

std::string MenuPanelDefinition::get_PanelId()
{
return this->PanelId;
}

int32_t MenuPanelDefinition::get_VisibleItemCount()
{
return this->VisibleItemCount;
}

MenuPanelDefinition::MenuPanelDefinition(std::string panelId, std::string heading, std::string description, int32_t visibleItemCount, Array<::MenuItemDefinition*>* items) : Description(), Heading(), Items(), PanelId(), VisibleItemCount(0)
{
    if (String::IsNullOrWhiteSpace(panelId))
    {
throw ([&]() {
auto __ctor_arg_000000F2 = "Panel id must be provided.";
auto __ctor_arg_000000F3 = "panelId";
return new ArgumentException(__ctor_arg_000000F2, __ctor_arg_000000F3);
})();
    }
    if (String::IsNullOrWhiteSpace(heading))
    {
throw ([&]() {
auto __ctor_arg_000000F4 = "Panel heading must be provided.";
auto __ctor_arg_000000F5 = "heading";
return new ArgumentException(__ctor_arg_000000F4, __ctor_arg_000000F5);
})();
    }
    if (visibleItemCount < 1)
    {
throw ([&]() {
auto __ctor_arg_000000F6 = "visibleItemCount";
auto __ctor_arg_000000F7 = "Visible item count must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_000000F6, __ctor_arg_000000F7);
})();
    }
    if (items == nullptr)
    {
throw new ArgumentNullException("items");
    }
    if (items->Length == 0)
    {
throw new InvalidOperationException("Menu panels must contain at least one item.");
    }
this->PanelId = panelId;
this->Heading = heading;
this->Description = description;
this->VisibleItemCount = visibleItemCount;
this->Items = items;
}

