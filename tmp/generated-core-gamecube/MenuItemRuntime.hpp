#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RoundedRectComponent;
class MenuItemComponent;
class Entity;

#include "RoundedRectComponent.hpp"
#include "MenuItemComponent.hpp"
#include "Entity.hpp"

class MenuItemRuntime
{
public:
    virtual ~MenuItemRuntime() = default;

    ::RoundedRectComponent* Background;

    ::RoundedRectComponent* get_Background();

    ::MenuItemComponent* Definition;

    ::MenuItemComponent* get_Definition();

    ::Entity* Entity;

    ::Entity* get_Entity();

    int32_t Index;

    int32_t get_Index();

    MenuItemRuntime(::MenuItemComponent* definition, int32_t index, ::Entity* entity, ::RoundedRectComponent* background);
};
