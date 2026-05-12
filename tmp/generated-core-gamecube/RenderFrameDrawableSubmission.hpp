#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameBatchingMetadata;
class IDrawable3D;
class RuntimeMaterial;

#include "RenderFrameBatchingMetadata.hpp"
#include "IDrawable3D.hpp"
#include "RuntimeMaterial.hpp"

class RenderFrameDrawableSubmission
{
public:
    virtual ~RenderFrameDrawableSubmission() = default;

    ::RenderFrameBatchingMetadata* BatchingMetadata;

    ::RenderFrameBatchingMetadata* get_BatchingMetadata();

    ::IDrawable3D* Drawable;

    ::IDrawable3D* get_Drawable();

    bool IsTransparent;

    bool get_IsTransparent();

    ::RuntimeMaterial* Material;

    ::RuntimeMaterial* get_Material();

    int32_t SubmeshIndex;

    int32_t get_SubmeshIndex();

    RenderFrameDrawableSubmission(::IDrawable3D* drawable);

    RenderFrameDrawableSubmission(::IDrawable3D* drawable, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata);

    RenderFrameDrawableSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata);
};
