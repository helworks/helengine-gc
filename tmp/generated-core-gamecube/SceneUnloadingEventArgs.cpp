#ifdef DrawText
#undef DrawText
#endif
#include "SceneUnloadingEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

std::string SceneUnloadingEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

List<::Entity*>* SceneUnloadingEventArgs::get_RootEntities()
{
return this->RootEntities;
}

std::string SceneUnloadingEventArgs::get_SceneId()
{
return this->SceneId;
}

SceneUnloadingEventArgs::SceneUnloadingEventArgs(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities) : CookedRelativePath(), RootEntities(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000156 = "Scene id is required.";
auto __ctor_arg_00000157 = "sceneId";
return new ArgumentException(__ctor_arg_00000156, __ctor_arg_00000157);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000158 = "Cooked relative path is required.";
auto __ctor_arg_00000159 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000158, __ctor_arg_00000159);
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

