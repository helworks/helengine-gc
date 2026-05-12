#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class MenuDefinition;

#include "MenuDefinition.hpp"

class IMenuDefinitionProvider
{
public:
    virtual ::MenuDefinition* CreateMenuDefinition() = 0;
};
