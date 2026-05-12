#ifdef DrawText
#undef DrawText
#endif
#include "SceneAssetReference.hpp"

SceneAssetReference::SceneAssetReference() : AssetId(), ProviderId(), RelativePath(), SourceKind()
{
}

std::string SceneAssetReference::get_AssetId()
{
return this->AssetId;
}

void SceneAssetReference::set_AssetId(std::string value)
{
this->AssetId = value;
}

std::string SceneAssetReference::get_ProviderId()
{
return this->ProviderId;
}

void SceneAssetReference::set_ProviderId(std::string value)
{
this->ProviderId = value;
}

std::string SceneAssetReference::get_RelativePath()
{
return this->RelativePath;
}

void SceneAssetReference::set_RelativePath(std::string value)
{
this->RelativePath = value;
}

::SceneAssetReferenceSourceKind SceneAssetReference::get_SourceKind()
{
return this->SourceKind;
}

void SceneAssetReference::set_SourceKind(::SceneAssetReferenceSourceKind value)
{
this->SourceKind = value;
}

