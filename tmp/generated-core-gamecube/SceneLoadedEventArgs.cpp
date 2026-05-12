#ifdef DrawText
#undef DrawText
#endif
#include "SceneLoadedEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

std::string SceneLoadedEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

List<::Entity*>* SceneLoadedEventArgs::get_RootEntities()
{
return this->RootEntities;
}

std::string SceneLoadedEventArgs::get_SceneId()
{
return this->SceneId;
}

SceneLoadedEventArgs::SceneLoadedEventArgs(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities) : CookedRelativePath(), RootEntities(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000141 = "Scene id is required.";
auto __ctor_arg_00000142 = "sceneId";
return new ArgumentException(__ctor_arg_00000141, __ctor_arg_00000142);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000143 = "Cooked relative path is required.";
auto __ctor_arg_00000144 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000143, __ctor_arg_00000144);
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

