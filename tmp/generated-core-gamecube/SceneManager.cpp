#ifdef DrawText
#undef DrawText
#endif
#include "SceneManager.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "SceneAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "LoadedSceneRecord.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/native_dictionary.hpp"
#include "PendingSceneOperation.hpp"
#include "PendingSceneOperationKind.hpp"
#include "Entity.hpp"
#include "SceneLoadMode.hpp"
#include "SceneLoadingEventArgs.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "SceneLoadedEventArgs.hpp"
#include "SceneUnloadingEventArgs.hpp"
#include "SceneUnloadedEventArgs.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/string_comparer.hpp"

List<::LoadedSceneRecord*>* SceneManager::get_LoadedScenes()
{
return this->LoadedSceneRecords;
}

void SceneManager::FlushPendingOperations()
{
    if (this->PendingOperations->Count() == 0)
    {
return;    }
this->IsFlushingPendingOperations = true;
{
auto __finallyGuard_00000149 = he_cpp_make_scope_exit([&]() {
this->IsFlushingPendingOperations = false;
});
while (this->PendingOperations->Count() > 0) {
::PendingSceneOperation *operation = (*this->PendingOperations)[0];
this->PendingOperations->RemoveAt(0);
    if (operation->get_OperationKind() == PendingSceneOperationKind::Load)
    {
this->LoadSceneImmediate(operation->get_SceneId(), operation->get_LoadMode());
    }
else {
this->UnloadSceneImmediate(operation->get_SceneId());
}
}
}
}

bool SceneManager::IsSceneLoaded(std::string sceneId)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_0000014A = "Scene id is required.";
auto __ctor_arg_0000014B = "sceneId";
return new ArgumentException(__ctor_arg_0000014A, __ctor_arg_0000014B);
})();
    }
return this->LoadedSceneRecordsById->ContainsKey(sceneId);}

void SceneManager::LoadScene(std::string sceneId, ::SceneLoadMode loadMode)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_0000014C = "Scene id is required.";
auto __ctor_arg_0000014D = "sceneId";
return new ArgumentException(__ctor_arg_0000014C, __ctor_arg_0000014D);
})();
    }
    if (this->ObjectManager->get_IsUpdateLoopActive() && !this->IsFlushingPendingOperations)
    {
this->PendingOperations->Add(PendingSceneOperation::CreateLoad(sceneId, loadMode));
return;    }
this->LoadSceneImmediate(sceneId, loadMode);
}

SceneManager::SceneManager(::RuntimeSceneCatalog* sceneCatalog, ::ContentManager* contentManager, ::RuntimeSceneLoadService* sceneLoadService, ::ObjectManager* objectManager) : SceneLoaded(), SceneLoading(), SceneUnloaded(), SceneUnloading(), ContentManager(), IsFlushingPendingOperations(), LoadedSceneRecords(), LoadedSceneRecordsById(), ObjectManager(), PendingOperations(), SceneCatalog(), SceneLoadService()
{
this->SceneCatalog = (sceneCatalog != nullptr ? sceneCatalog : throw new ArgumentNullException("sceneCatalog"));
this->ContentManager = (contentManager != nullptr ? contentManager : throw new ArgumentNullException("contentManager"));
this->SceneLoadService = (sceneLoadService != nullptr ? sceneLoadService : throw new ArgumentNullException("sceneLoadService"));
this->ObjectManager = (objectManager != nullptr ? objectManager : throw new ArgumentNullException("objectManager"));
this->LoadedSceneRecords = new List<::LoadedSceneRecord*>();
this->LoadedSceneRecordsById = new Dictionary<std::string, ::LoadedSceneRecord*>(StringComparer::OrdinalIgnoreCase);
this->PendingOperations = new List<::PendingSceneOperation*>();
}

bool SceneManager::TryGetLoadedScene(std::string sceneId, ::LoadedSceneRecord*& loadedSceneRecord)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_0000014E = "Scene id is required.";
auto __ctor_arg_0000014F = "sceneId";
return new ArgumentException(__ctor_arg_0000014E, __ctor_arg_0000014F);
})();
    }
return this->LoadedSceneRecordsById->TryGetValue(sceneId, loadedSceneRecord);}

void SceneManager::UnloadScene(std::string sceneId)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000150 = "Scene id is required.";
auto __ctor_arg_00000151 = "sceneId";
return new ArgumentException(__ctor_arg_00000150, __ctor_arg_00000151);
})();
    }
    if (this->ObjectManager->get_IsUpdateLoopActive() && !this->IsFlushingPendingOperations)
    {
this->PendingOperations->Add(PendingSceneOperation::CreateUnload(sceneId));
return;    }
this->UnloadSceneImmediate(sceneId);
}

void SceneManager::DisposeSceneRoots(List<::Entity*>* rootEntities)
{
    if (rootEntities == nullptr)
    {
throw new ArgumentNullException("rootEntities");
    }
for (int32_t index = rootEntities->get_Count() - 1; index >= 0; index--) {
(*rootEntities)[index]->Dispose();
}
}

void SceneManager::DisposeUntrackedRootEntities()
{
List<::Entity*> *rootEntities = new List<::Entity*>();
for (int32_t index = 0; index < this->ObjectManager->get_Entities()->Count(); index++) {
::Entity *entity = (*this->ObjectManager->get_Entities())[index];
    if (entity->get_Parent() == nullptr)
    {
rootEntities->Add(entity);
    }
}
for (int32_t index = rootEntities->Count() - 1; index >= 0; index--) {
(*rootEntities)[index]->Dispose();
}
}

void SceneManager::LoadSceneImmediate(std::string sceneId, ::SceneLoadMode loadMode)
{
::RuntimeSceneCatalogEntry* entry;
    if (!this->SceneCatalog->TryGetEntry(sceneId, entry))
    {
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' was not found in the build scene catalog."));
    }
    if (loadMode == SceneLoadMode::Additive && this->LoadedSceneRecordsById->ContainsKey(sceneId))
    {
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' is already loaded."));
    }
    if (loadMode == SceneLoadMode::Single)
    {
    if (this->LoadedSceneRecords->Count() == 0)
    {
this->DisposeUntrackedRootEntities();
    }
else {
this->UnloadAllScenes();
}
    }
this->SceneLoading.Invoke(this, new ::SceneLoadingEventArgs(entry->get_SceneId(), entry->get_CookedRelativePath()));
::SceneAsset *sceneAsset = this->ContentManager->Load<SceneAsset*>(entry->get_CookedRelativePath(), RuntimeContentProcessorIds::SceneAsset);
List<::Entity*> *rootEntities = this->SceneLoadService->Load(sceneAsset);
::LoadedSceneRecord *loadedSceneRecord = new ::LoadedSceneRecord(entry->get_SceneId(), entry->get_CookedRelativePath(), rootEntities);
this->LoadedSceneRecords->Add(loadedSceneRecord);
this->LoadedSceneRecordsById->Add(loadedSceneRecord->get_SceneId(), loadedSceneRecord);
this->SceneLoaded.Invoke(this, new ::SceneLoadedEventArgs(loadedSceneRecord->get_SceneId(), loadedSceneRecord->get_CookedRelativePath(), loadedSceneRecord->get_RootEntities()));
}

void SceneManager::UnloadAllScenes()
{
while (this->LoadedSceneRecords->Count() > 0) {
this->UnloadScene((*this->LoadedSceneRecords)[0]->get_SceneId());
}
}

void SceneManager::UnloadSceneImmediate(std::string sceneId)
{
::LoadedSceneRecord* loadedSceneRecord;
    if (!this->LoadedSceneRecordsById->TryGetValue(sceneId, loadedSceneRecord))
    {
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' is not currently loaded."));
    }
this->SceneUnloading.Invoke(this, new ::SceneUnloadingEventArgs(loadedSceneRecord->get_SceneId(), loadedSceneRecord->get_CookedRelativePath(), loadedSceneRecord->get_RootEntities()));
this->DisposeSceneRoots(loadedSceneRecord->get_RootEntities());
this->LoadedSceneRecordsById->Remove(loadedSceneRecord->get_SceneId());
this->LoadedSceneRecords->Remove(loadedSceneRecord);
this->SceneUnloaded.Invoke(this, new ::SceneUnloadedEventArgs(loadedSceneRecord->get_SceneId(), loadedSceneRecord->get_CookedRelativePath()));
}

