#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float2;
class int2;

#include "float2.hpp"
#include "int2.hpp"

class AnchorSpace
{
public:
    virtual ~AnchorSpace() = default;

    ::float2 Origin;

    ::float2 get_Origin();

    ::int2 Size;

    ::int2 get_Size();

    AnchorSpace(::int2 size, ::float2 origin);
};
