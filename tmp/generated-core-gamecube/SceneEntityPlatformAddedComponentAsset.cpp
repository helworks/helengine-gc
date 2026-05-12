#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformAddedComponentAsset.hpp"

SceneEntityPlatformAddedComponentAsset::SceneEntityPlatformAddedComponentAsset() : Component()
{
}

::SceneComponentAssetRecord* SceneEntityPlatformAddedComponentAsset::get_Component()
{
return this->Component;
}

void SceneEntityPlatformAddedComponentAsset::set_Component(::SceneComponentAssetRecord* value)
{
this->Component = value;
}

