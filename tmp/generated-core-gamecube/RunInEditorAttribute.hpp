#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "Attribute.hpp"

class RunInEditorAttribute : public Attribute
{
public:
    virtual ~RunInEditorAttribute() = default;
};
