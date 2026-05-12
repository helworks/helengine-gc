#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class float4;
class IDrawable2D;
class IClipRegion2D;

#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "float4.hpp"
#include "IDrawable2D.hpp"
#include "runtime/native_list.hpp"
#include "IClipRegion2D.hpp"
#include "float4.hpp"

class ClipRegionStackBuilder2D
{
public:
    virtual ~ClipRegionStackBuilder2D() = default;

    void BuildClipChain(::IDrawable2D* drawable, List<::IClipRegion2D*>* clipChain);

    ::float4 Intersect(::float4 current, ::float4 next);
};
