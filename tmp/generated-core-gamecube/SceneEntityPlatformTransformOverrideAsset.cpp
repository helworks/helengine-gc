#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "runtime/native_string.hpp"

SceneEntityPlatformTransformOverrideAsset::SceneEntityPlatformTransformOverrideAsset() : HasLocalOrientationOverride(), HasLocalPositionOverride(), HasLocalScaleOverride(), LocalOrientation(), LocalPosition(), LocalScale(), PlatformId(String::Empty)
{
}

bool SceneEntityPlatformTransformOverrideAsset::get_HasLocalOrientationOverride()
{
return this->HasLocalOrientationOverride;
}

void SceneEntityPlatformTransformOverrideAsset::set_HasLocalOrientationOverride(bool value)
{
this->HasLocalOrientationOverride = value;
}

bool SceneEntityPlatformTransformOverrideAsset::get_HasLocalPositionOverride()
{
return this->HasLocalPositionOverride;
}

void SceneEntityPlatformTransformOverrideAsset::set_HasLocalPositionOverride(bool value)
{
this->HasLocalPositionOverride = value;
}

bool SceneEntityPlatformTransformOverrideAsset::get_HasLocalScaleOverride()
{
return this->HasLocalScaleOverride;
}

void SceneEntityPlatformTransformOverrideAsset::set_HasLocalScaleOverride(bool value)
{
this->HasLocalScaleOverride = value;
}

::float4 SceneEntityPlatformTransformOverrideAsset::get_LocalOrientation()
{
return this->LocalOrientation;
}

void SceneEntityPlatformTransformOverrideAsset::set_LocalOrientation(::float4 value)
{
this->LocalOrientation = value;
}

::float3 SceneEntityPlatformTransformOverrideAsset::get_LocalPosition()
{
return this->LocalPosition;
}

void SceneEntityPlatformTransformOverrideAsset::set_LocalPosition(::float3 value)
{
this->LocalPosition = value;
}

::float3 SceneEntityPlatformTransformOverrideAsset::get_LocalScale()
{
return this->LocalScale;
}

void SceneEntityPlatformTransformOverrideAsset::set_LocalScale(::float3 value)
{
this->LocalScale = value;
}

std::string SceneEntityPlatformTransformOverrideAsset::get_PlatformId()
{
return this->PlatformId;
}

void SceneEntityPlatformTransformOverrideAsset::set_PlatformId(std::string value)
{
this->PlatformId = value;
}

