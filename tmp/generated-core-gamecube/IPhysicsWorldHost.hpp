#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Core;

#include "Core.hpp"

class IPhysicsWorldHost
{
public:
    virtual ::Core* get_Core() = 0;
};
