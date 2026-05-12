#ifdef DrawText
#undef DrawText
#endif
#include "SceneLoadingEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string SceneLoadingEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

std::string SceneLoadingEventArgs::get_SceneId()
{
return this->SceneId;
}

SceneLoadingEventArgs::SceneLoadingEventArgs(std::string sceneId, std::string cookedRelativePath) : CookedRelativePath(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000145 = "Scene id is required.";
auto __ctor_arg_00000146 = "sceneId";
return new ArgumentException(__ctor_arg_00000145, __ctor_arg_00000146);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000147 = "Cooked relative path is required.";
auto __ctor_arg_00000148 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000147, __ctor_arg_00000148);
})();
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
}

