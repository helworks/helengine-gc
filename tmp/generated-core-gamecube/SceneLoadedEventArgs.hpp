#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;

#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"

class SceneLoadedEventArgs
{
public:
    virtual ~SceneLoadedEventArgs() = default;

    std::string CookedRelativePath;

    std::string get_CookedRelativePath();

    List<::Entity*>* RootEntities;

    List<::Entity*>* get_RootEntities();

    std::string SceneId;

    std::string get_SceneId();

    SceneLoadedEventArgs(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities);
};
