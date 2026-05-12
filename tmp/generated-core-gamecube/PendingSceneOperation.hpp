#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "SceneLoadMode.hpp"
#include "PendingSceneOperationKind.hpp"
#include "runtime/native_string.hpp"

class PendingSceneOperation
{
public:
    virtual ~PendingSceneOperation() = default;

    ::SceneLoadMode LoadMode;

    ::SceneLoadMode get_LoadMode();

    ::PendingSceneOperationKind OperationKind;

    ::PendingSceneOperationKind get_OperationKind();

    std::string SceneId;

    std::string get_SceneId();

    static ::PendingSceneOperation* CreateLoad(std::string sceneId, ::SceneLoadMode loadMode);

    static ::PendingSceneOperation* CreateUnload(std::string sceneId);
private:
    PendingSceneOperation(::PendingSceneOperationKind operationKind, std::string sceneId, ::SceneLoadMode loadMode);
};
