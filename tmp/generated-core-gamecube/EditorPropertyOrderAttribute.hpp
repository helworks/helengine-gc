#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "Attribute.hpp"

class EditorPropertyOrderAttribute : public Attribute
{
public:
    virtual ~EditorPropertyOrderAttribute() = default;

    int32_t Order;

    int32_t get_Order();

    EditorPropertyOrderAttribute(int32_t order);
};
