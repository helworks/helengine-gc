#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityAsset.hpp"
#include "runtime/array.hpp"

SceneEntityAsset::SceneEntityAsset() : Children(Array<SceneEntityAsset*>::Empty()), Components(Array<SceneComponentAssetRecord*>::Empty()), Id(), LocalOrientation(), LocalPosition(), LocalScale(), Name(), PlatformComponentOverrides(Array<SceneEntityPlatformComponentOverrideAsset*>::Empty()), PlatformTransformOverrides(Array<SceneEntityPlatformTransformOverrideAsset*>::Empty())
{
}

Array<::SceneEntityAsset*>* SceneEntityAsset::get_Children()
{
return this->Children;
}

void SceneEntityAsset::set_Children(Array<::SceneEntityAsset*>* value)
{
this->Children = value;
}

Array<::SceneComponentAssetRecord*>* SceneEntityAsset::get_Components()
{
return this->Components;
}

void SceneEntityAsset::set_Components(Array<::SceneComponentAssetRecord*>* value)
{
this->Components = value;
}

std::string SceneEntityAsset::get_Id()
{
return this->Id;
}

void SceneEntityAsset::set_Id(std::string value)
{
this->Id = value;
}

::float4 SceneEntityAsset::get_LocalOrientation()
{
return this->LocalOrientation;
}

void SceneEntityAsset::set_LocalOrientation(::float4 value)
{
this->LocalOrientation = value;
}

::float3 SceneEntityAsset::get_LocalPosition()
{
return this->LocalPosition;
}

void SceneEntityAsset::set_LocalPosition(::float3 value)
{
this->LocalPosition = value;
}

::float3 SceneEntityAsset::get_LocalScale()
{
return this->LocalScale;
}

void SceneEntityAsset::set_LocalScale(::float3 value)
{
this->LocalScale = value;
}

std::string SceneEntityAsset::get_Name()
{
return this->Name;
}

void SceneEntityAsset::set_Name(std::string value)
{
this->Name = value;
}

Array<::SceneEntityPlatformComponentOverrideAsset*>* SceneEntityAsset::get_PlatformComponentOverrides()
{
return this->PlatformComponentOverrides;
}

void SceneEntityAsset::set_PlatformComponentOverrides(Array<::SceneEntityPlatformComponentOverrideAsset*>* value)
{
this->PlatformComponentOverrides = value;
}

Array<::SceneEntityPlatformTransformOverrideAsset*>* SceneEntityAsset::get_PlatformTransformOverrides()
{
return this->PlatformTransformOverrides;
}

void SceneEntityAsset::set_PlatformTransformOverrides(Array<::SceneEntityPlatformTransformOverrideAsset*>* value)
{
this->PlatformTransformOverrides = value;
}

