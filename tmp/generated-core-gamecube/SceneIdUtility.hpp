#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"
#include "system/io/path.hpp"
#include "system/io/path.hpp"

class SceneIdUtility
{
public:
    virtual ~SceneIdUtility() = default;

    static std::string FromPath(std::string scenePath);
};
