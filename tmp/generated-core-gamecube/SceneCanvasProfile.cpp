#ifdef DrawText
#undef DrawText
#endif
#include "SceneCanvasProfile.hpp"

SceneCanvasProfile::SceneCanvasProfile() : Height(DefaultHeight), Width(DefaultWidth)
{
}

int32_t SceneCanvasProfile::DefaultHeight = 720;

int32_t SceneCanvasProfile::DefaultWidth = 1280;

int32_t SceneCanvasProfile::get_Height()
{
return this->Height;
}

void SceneCanvasProfile::set_Height(int32_t value)
{
this->Height = value;
}

int32_t SceneCanvasProfile::get_Width()
{
return this->Width;
}

void SceneCanvasProfile::set_Width(int32_t value)
{
this->Width = value;
}

