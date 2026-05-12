#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameDrawableClassifier.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "IDrawable3D.hpp"
#include "RuntimeMaterial.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeSubmesh.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "MaterialBlendMode.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<::RenderFrameDrawableSubmission*>* RenderFrameDrawableClassifier::Classify(::IDrawable3D* drawable)
{
    if (drawable == nullptr)
    {
throw new ArgumentNullException("drawable");
    }
Array<::RuntimeSubmesh*> *submeshes = this->ResolveSubmeshes(drawable->get_Model());
Array<::RenderFrameDrawableSubmission*> *submissions = new Array<RenderFrameDrawableSubmission*>(submeshes->Length);
for (int32_t submeshIndex = 0; submeshIndex < submeshes->Length; submeshIndex++) {
::RuntimeMaterial *material = this->ResolveMaterial(drawable, submeshIndex);
(*submissions)[submeshIndex] = ([&]() {
auto __ctor_arg_0000010B = drawable;
auto __ctor_arg_0000010C = submeshIndex;
auto __ctor_arg_0000010D = material;
auto __ctor_arg_0000010E = this->IsTransparent(material);
auto __ctor_arg_0000010F = new ::RenderFrameBatchingMetadata(false, false, false);
return new ::RenderFrameDrawableSubmission(__ctor_arg_0000010B, __ctor_arg_0000010C, __ctor_arg_0000010D, __ctor_arg_0000010E, __ctor_arg_0000010F);
})();
}
return submissions;}

bool RenderFrameDrawableClassifier::IsTransparent(::RuntimeMaterial* material)
{
    if (material == nullptr)
    {
return false;    }
::MaterialRenderState *renderState = material->get_RenderState();
return renderState != nullptr && renderState->get_BlendMode() == MaterialBlendMode::AlphaBlend;}

::RuntimeMaterial* RenderFrameDrawableClassifier::ResolveMaterial(::IDrawable3D* drawable, int32_t submeshIndex)
{
    if (drawable == nullptr)
    {
throw new ArgumentNullException("drawable");
    }
else     if (submeshIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_00000110 = "submeshIndex";
auto __ctor_arg_00000111 = "Submesh index must be non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000110, __ctor_arg_00000111);
})();
    }
Array<::RuntimeMaterial*> *materials = drawable->get_Materials();
    if (materials == nullptr || materials->Length == 0)
    {
return drawable->get_Material();    }
    if (submeshIndex < materials->Length)
    {
return (*materials)[submeshIndex];    }
return (*materials)[0];}

Array<::RuntimeSubmesh*>* RenderFrameDrawableClassifier::ResolveSubmeshes(::RuntimeModel* model)
{
    if (model == nullptr || model->get_Submeshes() == nullptr || model->get_Submeshes()->Length == 0)
    {
return new Array<RuntimeSubmesh*>({ ([&]() {
auto __object_00000112 = new ::RuntimeSubmesh();
__object_00000112->set_MaterialSlotName(String::Empty);
__object_00000112->set_IndexStart(0);
__object_00000112->set_IndexCount(0);
return __object_00000112;
})() });    }
return model->get_Submeshes();}

