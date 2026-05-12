#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "Attribute.hpp"

class EditorPropertyHiddenAttribute : public Attribute
{
public:
    virtual ~EditorPropertyHiddenAttribute() = default;
};
