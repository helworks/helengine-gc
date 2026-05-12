#ifdef DrawText
#undef DrawText
#endif
#include "LoadedSceneRecord.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

std::string LoadedSceneRecord::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

List<::Entity*>* LoadedSceneRecord::get_RootEntities()
{
return this->RootEntities;
}

std::string LoadedSceneRecord::get_SceneId()
{
return this->SceneId;
}

LoadedSceneRecord::LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities) : CookedRelativePath(), RootEntities(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_000000C1 = "Scene id is required.";
auto __ctor_arg_000000C2 = "sceneId";
return new ArgumentException(__ctor_arg_000000C1, __ctor_arg_000000C2);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_000000C3 = "Cooked relative path is required.";
auto __ctor_arg_000000C4 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_000000C3, __ctor_arg_000000C4);
})();
    }
    if (rootEntities == nullptr)
    {
throw new ArgumentNullException("rootEntities");
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
this->RootEntities = rootEntities;
}

