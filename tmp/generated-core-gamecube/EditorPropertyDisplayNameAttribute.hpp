#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "Attribute.hpp"
#include "runtime/native_string.hpp"

class EditorPropertyDisplayNameAttribute : public Attribute
{
public:
    virtual ~EditorPropertyDisplayNameAttribute() = default;

    std::string DisplayName;

    std::string get_DisplayName();

    EditorPropertyDisplayNameAttribute(std::string displayName);
};
