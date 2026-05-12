#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_exceptions.hpp"
#include "ComponentExecutionMode.hpp"

class ComponentExecutionContext
{
public:
    virtual ~ComponentExecutionContext() = default;

    static ::ComponentExecutionMode get_CurrentMode();

    static void EnterEditor();

    static void ExitEditor();
private:
    static int32_t EditorExecutionDepth;
};
