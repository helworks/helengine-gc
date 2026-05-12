#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "runtime/native_exceptions.hpp"

::RenderFrameBatchingMetadata* RenderFrameDrawableSubmission::get_BatchingMetadata()
{
return this->BatchingMetadata;
}

::IDrawable3D* RenderFrameDrawableSubmission::get_Drawable()
{
return this->Drawable;
}

bool RenderFrameDrawableSubmission::get_IsTransparent()
{
return this->IsTransparent;
}

::RuntimeMaterial* RenderFrameDrawableSubmission::get_Material()
{
return this->Material;
}

int32_t RenderFrameDrawableSubmission::get_SubmeshIndex()
{
return this->SubmeshIndex;
}

RenderFrameDrawableSubmission::RenderFrameDrawableSubmission(::IDrawable3D* drawable) : BatchingMetadata(), Drawable(), IsTransparent(), Material(), SubmeshIndex(0)
{
    if (drawable == nullptr)
    {
throw new ArgumentNullException("drawable");
    }
this->Drawable = drawable;
this->BatchingMetadata = new ::RenderFrameBatchingMetadata(false, false, false);
this->Material = drawable->get_Material();
}

RenderFrameDrawableSubmission::RenderFrameDrawableSubmission(::IDrawable3D* drawable, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata) : BatchingMetadata(), Drawable(), IsTransparent(), Material(), SubmeshIndex(0)
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
this->BatchingMetadata = (batchingMetadata != nullptr ? batchingMetadata : throw new ArgumentNullException("batchingMetadata"));
this->Material = drawable->get_Material();
this->IsTransparent = isTransparent;
}

RenderFrameDrawableSubmission::RenderFrameDrawableSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata) : BatchingMetadata(), Drawable(), IsTransparent(), Material(), SubmeshIndex(0)
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
    if (submeshIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_00000113 = "submeshIndex";
auto __ctor_arg_00000114 = "Submesh index must be non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000113, __ctor_arg_00000114);
})();
    }
this->BatchingMetadata = (batchingMetadata != nullptr ? batchingMetadata : throw new ArgumentNullException("batchingMetadata"));
this->SubmeshIndex = submeshIndex;
this->Material = material;
this->IsTransparent = isTransparent;
}

