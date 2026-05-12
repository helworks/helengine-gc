#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;

#include "Component.hpp"
#include "runtime/native_string.hpp"

class MenuSelectedDescriptionComponent : public Component
{
public:
    virtual ~MenuSelectedDescriptionComponent() = default;

    static uint8_t CurrentVersion;

    static std::string SerializedComponentTypeId;

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
};
