#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputFrameState;

#include "InputFrameState.hpp"

class IInputBackend
{
public:
    virtual bool get_ReceiveInputInBackground() = 0;

    virtual void set_ReceiveInputInBackground(bool value) = 0;

    virtual ::InputFrameState CaptureFrame() = 0;
};
