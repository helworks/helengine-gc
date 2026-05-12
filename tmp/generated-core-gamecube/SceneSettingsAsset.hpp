#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneCanvasProfile;

#include "SceneCanvasProfile.hpp"

class SceneSettingsAsset
{
public:
    virtual ~SceneSettingsAsset() = default;

    SceneSettingsAsset();

    ::SceneCanvasProfile* CanvasProfile;

    ::SceneCanvasProfile* get_CanvasProfile();
    void set_CanvasProfile(::SceneCanvasProfile* value);
};
