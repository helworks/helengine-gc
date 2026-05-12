#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "runtime/array.hpp"

SceneEntityPlatformComponentOverrideAsset::SceneEntityPlatformComponentOverrideAsset() : AddedComponents(Array<SceneEntityPlatformAddedComponentAsset*>::Empty()), PlatformId(), RemovedComponentKeys(Array<std::string>::Empty())
{
}

Array<::SceneEntityPlatformAddedComponentAsset*>* SceneEntityPlatformComponentOverrideAsset::get_AddedComponents()
{
return this->AddedComponents;
}

void SceneEntityPlatformComponentOverrideAsset::set_AddedComponents(Array<::SceneEntityPlatformAddedComponentAsset*>* value)
{
this->AddedComponents = value;
}

std::string SceneEntityPlatformComponentOverrideAsset::get_PlatformId()
{
return this->PlatformId;
}

void SceneEntityPlatformComponentOverrideAsset::set_PlatformId(std::string value)
{
this->PlatformId = value;
}

Array<std::string>* SceneEntityPlatformComponentOverrideAsset::get_RemovedComponentKeys()
{
return this->RemovedComponentKeys;
}

void SceneEntityPlatformComponentOverrideAsset::set_RemovedComponentKeys(Array<std::string>* value)
{
this->RemovedComponentKeys = value;
}

