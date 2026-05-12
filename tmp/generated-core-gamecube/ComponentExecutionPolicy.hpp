#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class Entity;

#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "Entity.hpp"

class ComponentExecutionPolicy
{
public:
    virtual ~ComponentExecutionPolicy() = default;

    static bool ShouldRunComponentLifecycle(::Component* component, ::Entity* entity);
};
