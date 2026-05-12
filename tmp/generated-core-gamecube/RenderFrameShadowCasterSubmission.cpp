#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameShadowCasterSubmission.hpp"
#include "runtime/native_exceptions.hpp"

::IDrawable3D* RenderFrameShadowCasterSubmission::get_Drawable()
{
return this->Drawable;
}

::RuntimeMaterial* RenderFrameShadowCasterSubmission::get_Material()
{
return this->Material;
}

int32_t RenderFrameShadowCasterSubmission::get_SubmeshIndex()
{
return this->SubmeshIndex;
}

RenderFrameShadowCasterSubmission::RenderFrameShadowCasterSubmission(::IDrawable3D* drawable) : Drawable(), Material(), SubmeshIndex(0)
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
this->Material = drawable->get_Material();
}

RenderFrameShadowCasterSubmission::RenderFrameShadowCasterSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material) : Drawable(), Material(), SubmeshIndex(0)
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
    if (submeshIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_00000119 = "submeshIndex";
auto __ctor_arg_0000011A = "Submesh index must be non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000119, __ctor_arg_0000011A);
})();
    }
this->SubmeshIndex = submeshIndex;
this->Material = material;
}

