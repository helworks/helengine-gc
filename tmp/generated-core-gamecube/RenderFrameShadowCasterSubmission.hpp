#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable3D;
class RuntimeMaterial;

#include "IDrawable3D.hpp"
#include "RuntimeMaterial.hpp"

class RenderFrameShadowCasterSubmission
{
public:
    virtual ~RenderFrameShadowCasterSubmission() = default;

    ::IDrawable3D* Drawable;

    ::IDrawable3D* get_Drawable();

    ::RuntimeMaterial* Material;

    ::RuntimeMaterial* get_Material();

    int32_t SubmeshIndex;

    int32_t get_SubmeshIndex();

    RenderFrameShadowCasterSubmission(::IDrawable3D* drawable);

    RenderFrameShadowCasterSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material);
};
