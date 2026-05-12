#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/array.hpp"
#include "runtime/native_string.hpp"

class RuntimeSceneCatalogEntry
{
public:
    virtual ~RuntimeSceneCatalogEntry() = default;

    std::string CookedRelativePath;

    std::string get_CookedRelativePath();

    std::string SceneId;

    std::string get_SceneId();

    RuntimeSceneCatalogEntry(std::string sceneId, std::string cookedRelativePath);
private:
    static std::string NormalizeCookedRelativePath(std::string cookedRelativePath);
};
