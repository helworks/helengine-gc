#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;
class Entity;
class LightComponent;

#include "runtime/native_exceptions.hpp"
#include "float3.hpp"
#include "runtime/native_exceptions.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "LightComponent.hpp"

class LightDirectionUtility
{
public:
    virtual ~LightDirectionUtility() = default;

    static ::float3 get_AuthoredForwardAxis();

    static ::float3 GetEntityForwardDirection(::Entity* entity);

    static ::float3 GetLightDirection(::LightComponent* lightComponent);
};
