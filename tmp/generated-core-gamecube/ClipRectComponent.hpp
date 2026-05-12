#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IClipRegion2D;
class float4;
class int2;

#include "Component.hpp"
#include "IClipRegion2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "float4.hpp"

class ClipRectComponent : public Component, public IClipRegion2D
{
public:
    virtual ~ClipRectComponent() = default;

    ClipRectComponent();

    ::int2 get_Size();

    void set_Size(::int2 value);

    ::float4 GetClipRect();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    ::int2 SizeValue;
};
