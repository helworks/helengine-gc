#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class MenuActionDefinition;

#include "MenuActionDefinition.hpp"
#include "runtime/native_string.hpp"

class MenuItemDefinition
{
public:
    virtual ~MenuItemDefinition() = default;

    ::MenuActionDefinition* Action;

    ::MenuActionDefinition* get_Action();

    std::string Description;

    std::string get_Description();

    bool Enabled;

    bool get_Enabled();

    std::string ItemId;

    std::string get_ItemId();

    std::string Label;

    std::string get_Label();

    MenuItemDefinition(std::string itemId, std::string label, std::string description, bool enabled, ::MenuActionDefinition* action);
};
