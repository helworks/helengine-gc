#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneAsset;
class ContentManager;
class RuntimeSceneLoadService;
class LoadedSceneRecord;
class RuntimeSceneCatalogEntry;
class ObjectManager;
class PendingSceneOperation;
class RuntimeSceneCatalog;
class Entity;

#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneAsset.hpp"
#include "ContentManager.hpp"
#include "runtime/native_list.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "LoadedSceneRecord.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_event.hpp"
#include "ContentManager.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "ObjectManager.hpp"
#include "runtime/native_list.hpp"
#include "PendingSceneOperation.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneLoadMode.hpp"
#include "Entity.hpp"

class SceneManager
{
public:
    virtual ~SceneManager() = default;

    List<::LoadedSceneRecord*>* get_LoadedScenes();

    ::Event SceneLoaded;

    ::Event SceneLoading;

    ::Event SceneUnloaded;

    ::Event SceneUnloading;

    void FlushPendingOperations();

    bool IsSceneLoaded(std::string sceneId);

    void LoadScene(std::string sceneId, ::SceneLoadMode loadMode);

    SceneManager(::RuntimeSceneCatalog* sceneCatalog, ::ContentManager* contentManager, ::RuntimeSceneLoadService* sceneLoadService, ::ObjectManager* objectManager);

    bool TryGetLoadedScene(std::string sceneId, ::LoadedSceneRecord*& loadedSceneRecord);

    void UnloadScene(std::string sceneId);
private:
    ::ContentManager* ContentManager;

    bool IsFlushingPendingOperations;

    List<::LoadedSceneRecord*>* LoadedSceneRecords;

    Dictionary<std::string, ::LoadedSceneRecord*>* LoadedSceneRecordsById;

    ::ObjectManager* ObjectManager;

    List<::PendingSceneOperation*>* PendingOperations;

    ::RuntimeSceneCatalog* SceneCatalog;

    ::RuntimeSceneLoadService* SceneLoadService;

    void DisposeSceneRoots(List<::Entity*>* rootEntities);

    void DisposeUntrackedRootEntities();

    void LoadSceneImmediate(std::string sceneId, ::SceneLoadMode loadMode);

    void UnloadAllScenes();

    void UnloadSceneImmediate(std::string sceneId);
};
