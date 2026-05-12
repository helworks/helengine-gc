#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "MenuActionKind.hpp"
#include "runtime/native_string.hpp"

class MenuActionDefinition
{
public:
    virtual ~MenuActionDefinition() = default;

    ::MenuActionKind Kind;

    ::MenuActionKind get_Kind();

    std::string TargetId;

    std::string get_TargetId();

    MenuActionDefinition(::MenuActionKind kind, std::string targetId);
};
