#ifdef DrawText
#undef DrawText
#endif
#include "SceneUnloadedEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string SceneUnloadedEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

std::string SceneUnloadedEventArgs::get_SceneId()
{
return this->SceneId;
}

SceneUnloadedEventArgs::SceneUnloadedEventArgs(std::string sceneId, std::string cookedRelativePath) : CookedRelativePath(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000152 = "Scene id is required.";
auto __ctor_arg_00000153 = "sceneId";
return new ArgumentException(__ctor_arg_00000152, __ctor_arg_00000153);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000154 = "Cooked relative path is required.";
auto __ctor_arg_00000155 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000154, __ctor_arg_00000155);
})();
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
}

