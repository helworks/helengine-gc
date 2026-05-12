#ifdef DrawText
#undef DrawText
#endif
#include "SceneSettingsAsset.hpp"
#include "SceneCanvasProfile.hpp"

SceneSettingsAsset::SceneSettingsAsset() : CanvasProfile(new ::SceneCanvasProfile())
{
}

::SceneCanvasProfile* SceneSettingsAsset::get_CanvasProfile()
{
return this->CanvasProfile;
}

void SceneSettingsAsset::set_CanvasProfile(::SceneCanvasProfile* value)
{
this->CanvasProfile = value;
}

