#ifdef DrawText
#undef DrawText
#endif
#include "PendingSceneOperation.hpp"
#include "PendingSceneOperation.hpp"
#include "PendingSceneOperationKind.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::SceneLoadMode PendingSceneOperation::get_LoadMode()
{
return this->LoadMode;
}

::PendingSceneOperationKind PendingSceneOperation::get_OperationKind()
{
return this->OperationKind;
}

std::string PendingSceneOperation::get_SceneId()
{
return this->SceneId;
}

::PendingSceneOperation* PendingSceneOperation::CreateLoad(std::string sceneId, ::SceneLoadMode loadMode)
{
return new ::PendingSceneOperation(PendingSceneOperationKind::Load, sceneId, loadMode);}

::PendingSceneOperation* PendingSceneOperation::CreateUnload(std::string sceneId)
{
return new ::PendingSceneOperation(PendingSceneOperationKind::Unload, sceneId, SceneLoadMode::Single);}

PendingSceneOperation::PendingSceneOperation(::PendingSceneOperationKind operationKind, std::string sceneId, ::SceneLoadMode loadMode) : LoadMode(), OperationKind(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_000000FF = "Scene id is required.";
auto __ctor_arg_00000100 = "sceneId";
return new ArgumentException(__ctor_arg_000000FF, __ctor_arg_00000100);
})();
    }
this->OperationKind = operationKind;
this->SceneId = sceneId;
this->LoadMode = loadMode;
}

