#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class SceneUnloadedEventArgs
{
public:
    virtual ~SceneUnloadedEventArgs() = default;

    std::string CookedRelativePath;

    std::string get_CookedRelativePath();

    std::string SceneId;

    std::string get_SceneId();

    SceneUnloadedEventArgs(std::string sceneId, std::string cookedRelativePath);
};
