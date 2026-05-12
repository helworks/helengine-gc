#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneLoadService.hpp"
#include "runtime/native_exceptions.hpp"
#include "Logger.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "runtime/array.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "SceneEntityAsset.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "system/diagnostics/stopwatch.hpp"

List<::Entity*>* RuntimeSceneLoadService::Load(::SceneAsset* sceneAsset)
{
    if (sceneAsset == nullptr)
    {
throw new ArgumentNullException("sceneAsset");
    }
Logger::WriteLine("Loading packaged scene assets.");
Stopwatch *loadStopwatch = System::Diagnostics::Stopwatch::StartNew();
Array<::SceneEntityAsset*> *rootEntityAssets = ([&]() {
Array<::SceneEntityAsset*>* __coalesce_00000139 = sceneAsset->get_RootEntities();
return __coalesce_00000139 != nullptr ? __coalesce_00000139 : Array<SceneEntityAsset*>::Empty();
})();
List<::Entity*> *rootEntities = new List<::Entity*>(rootEntityAssets->Length);
for (int32_t index = 0; index < rootEntityAssets->Length; index++) {
rootEntities->Add(this->LoadEntity((*rootEntityAssets)[index]));
}
loadStopwatch->Stop();
Logger::WriteLine(std::string("Loaded packaged scene assets in ") + std::to_string(loadStopwatch->Elapsed.TotalMilliseconds) + std::string(" ms (") + std::to_string(rootEntities->Count()) + std::string(" root entities)."));
return rootEntities;}

RuntimeSceneLoadService::RuntimeSceneLoadService(::RuntimeSceneAssetReferenceResolver* referenceResolver) : ComponentRegistry(), ReferenceResolver()
{
this->ReferenceResolver = (referenceResolver != nullptr ? referenceResolver : throw new ArgumentNullException("referenceResolver"));
this->ComponentRegistry = RuntimeComponentRegistry::CreateDefault();
}

RuntimeSceneLoadService::RuntimeSceneLoadService(::RuntimeSceneAssetReferenceResolver* referenceResolver, ::RuntimeComponentRegistry* componentRegistry) : ComponentRegistry(), ReferenceResolver()
{
this->ReferenceResolver = (referenceResolver != nullptr ? referenceResolver : throw new ArgumentNullException("referenceResolver"));
this->ComponentRegistry = (componentRegistry != nullptr ? componentRegistry : throw new ArgumentNullException("componentRegistry"));
}

::Component* RuntimeSceneLoadService::LoadComponent(::SceneComponentAssetRecord* record)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
return this->ComponentRegistry->GetDeserializer(record->get_ComponentTypeId())->Deserialize(record, this->ReferenceResolver);}

::Entity* RuntimeSceneLoadService::LoadEntity(::SceneEntityAsset* entityAsset)
{
    if (entityAsset == nullptr)
    {
throw new ArgumentNullException("entityAsset");
    }
::Entity *entity = ([&]() {
auto __object_0000013A = new ::Entity();
__object_0000013A->set_LocalPosition(entityAsset->get_LocalPosition());
__object_0000013A->set_LocalScale(entityAsset->get_LocalScale());
__object_0000013A->set_LocalOrientation(entityAsset->get_LocalOrientation());
return __object_0000013A;
})();
entity->InitComponents();
entity->InitChildren();
Array<::SceneComponentAssetRecord*> *componentRecords = ([&]() {
Array<::SceneComponentAssetRecord*>* __coalesce_0000013B = entityAsset->get_Components();
return __coalesce_0000013B != nullptr ? __coalesce_0000013B : Array<SceneComponentAssetRecord*>::Empty();
})();
for (int32_t index = 0; index < componentRecords->Length; index++) {
entity->AddComponent(this->LoadComponent((*componentRecords)[index]));
}
Array<::SceneEntityAsset*> *childEntityAssets = ([&]() {
Array<::SceneEntityAsset*>* __coalesce_0000013C = entityAsset->get_Children();
return __coalesce_0000013C != nullptr ? __coalesce_0000013C : Array<SceneEntityAsset*>::Empty();
})();
for (int32_t index = 0; index < childEntityAssets->Length; index++) {
entity->AddChild(this->LoadEntity((*childEntityAssets)[index]));
}
return entity;}

