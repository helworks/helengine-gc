#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class byte4;

#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "MenuActionKind.hpp"
#include "byte4.hpp"

class MenuItemComponent : public Component
{
public:
    virtual ~MenuItemComponent() = default;

    static uint8_t CurrentVersion;

    static std::string SerializedComponentTypeId;

    ::MenuActionKind ActionKind;

    ::MenuActionKind get_ActionKind();
    void set_ActionKind(::MenuActionKind value);

    std::string get_Description();

    void set_Description(std::string value);

    ::byte4 IdleBorderColor;

    ::byte4 get_IdleBorderColor();
    void set_IdleBorderColor(::byte4 value);

    ::byte4 IdleFillColor;

    ::byte4 get_IdleFillColor();
    void set_IdleFillColor(::byte4 value);

    std::string get_ItemId();

    void set_ItemId(std::string value);

    std::string get_PanelId();

    void set_PanelId(std::string value);

    ::byte4 SelectedBorderColor;

    ::byte4 get_SelectedBorderColor();
    void set_SelectedBorderColor(::byte4 value);

    ::byte4 SelectedFillColor;

    ::byte4 get_SelectedFillColor();
    void set_SelectedFillColor(::byte4 value);

    std::string get_TargetId();

    void set_TargetId(std::string value);

    MenuItemComponent();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    std::string DescriptionValue;

    std::string ItemIdValue;

    std::string PanelIdValue;

    std::string TargetIdValue;
};
