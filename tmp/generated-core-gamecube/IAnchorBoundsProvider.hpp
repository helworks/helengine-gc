#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class AnchorSpace;

#include "runtime/native_event.hpp"
#include "AnchorSpace.hpp"

class IAnchorBoundsProvider
{
public:
    ::Event AnchorBoundsChanged;

    virtual ::AnchorSpace* get_AnchorSpace() = 0;
};
