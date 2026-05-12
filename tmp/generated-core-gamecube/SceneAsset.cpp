#ifdef DrawText
#undef DrawText
#endif
#include "SceneAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "runtime/array.hpp"

SceneAsset::SceneAsset() : AssetReferences(Array<SceneAssetReference*>::Empty()), Physics3DSceneFeatureFlags(0), RootEntities(Array<SceneEntityAsset*>::Empty()), SceneSettings(new ::SceneSettingsAsset())
{
}

std::string SceneAsset::FileExtension = ".helen";

Array<::SceneAssetReference*>* SceneAsset::get_AssetReferences()
{
return this->AssetReferences;
}

void SceneAsset::set_AssetReferences(Array<::SceneAssetReference*>* value)
{
this->AssetReferences = value;
}

uint32_t SceneAsset::get_Physics3DSceneFeatureFlags()
{
return this->Physics3DSceneFeatureFlags;
}

void SceneAsset::set_Physics3DSceneFeatureFlags(uint32_t value)
{
this->Physics3DSceneFeatureFlags = value;
}

Array<::SceneEntityAsset*>* SceneAsset::get_RootEntities()
{
return this->RootEntities;
}

void SceneAsset::set_RootEntities(Array<::SceneEntityAsset*>* value)
{
this->RootEntities = value;
}

::SceneSettingsAsset* SceneAsset::get_SceneSettings()
{
return this->SceneSettings;
}

void SceneAsset::set_SceneSettings(::SceneSettingsAsset* value)
{
this->SceneSettings = value;
}

std::string SceneAsset::get_Id()
{
return this->Asset::get_Id();
}

void SceneAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}

