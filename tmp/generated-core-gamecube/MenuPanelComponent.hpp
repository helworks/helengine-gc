#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;

#include "Component.hpp"
#include "runtime/native_string.hpp"

class MenuPanelComponent : public Component
{
public:
    virtual ~MenuPanelComponent() = default;

    static uint8_t CurrentVersion;

    static std::string SerializedComponentTypeId;

    std::string get_PanelId();

    void set_PanelId(std::string value);

    MenuPanelComponent();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    std::string PanelIdValue;
};
