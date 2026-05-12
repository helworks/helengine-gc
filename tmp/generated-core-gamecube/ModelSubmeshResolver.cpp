#ifdef DrawText
#undef DrawText
#endif
#include "ModelSubmeshResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelSubmeshAsset.hpp"
#include "RuntimeSubmesh.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<::RuntimeSubmesh*>* ModelSubmeshResolver::BuildRuntimeSubmeshes(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
Array<::ModelSubmeshAsset*> *authoredSubmeshes = ResolveAssetSubmeshes(asset);
Array<::RuntimeSubmesh*> *runtimeSubmeshes = new Array<RuntimeSubmesh*>(authoredSubmeshes->Length);
for (int32_t submeshIndex = 0; submeshIndex < authoredSubmeshes->Length; submeshIndex++) {
::ModelSubmeshAsset *authoredSubmesh = (*authoredSubmeshes)[submeshIndex];
(*runtimeSubmeshes)[submeshIndex] = ([&]() {
auto __object_000000F9 = new ::RuntimeSubmesh();
__object_000000F9->set_MaterialSlotName(authoredSubmesh->get_MaterialSlotName());
__object_000000F9->set_IndexStart(authoredSubmesh->get_IndexStart());
__object_000000F9->set_IndexCount(authoredSubmesh->get_IndexCount());
return __object_000000F9;
})();
}
return runtimeSubmeshes;}

Array<::ModelSubmeshAsset*>* ModelSubmeshResolver::ResolveAssetSubmeshes(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
const int32_t elementCount = ResolveElementCount(asset);
    if (asset->Submeshes != nullptr && asset->Submeshes->Length > 0)
    {
ValidateSubmeshes(asset->Submeshes, elementCount);
return asset->Submeshes;    }
    if (elementCount == 0)
    {
return Array<ModelSubmeshAsset*>::Empty();    }
return new Array<ModelSubmeshAsset*>({ ([&]() {
auto __object_000000FA = new ::ModelSubmeshAsset();
__object_000000FA->set_MaterialSlotName(String::Empty);
__object_000000FA->set_IndexStart(0);
__object_000000FA->set_IndexCount(elementCount);
return __object_000000FA;
})() });}

int32_t ModelSubmeshResolver::ResolveElementCount(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
::ModelAssetIndexData *indexData = ModelAssetIndexData::Resolve(asset);
    if (indexData->get_IndexCount() > 0)
    {
return indexData->get_IndexCount();    }
return asset->Positions == nullptr ? 0 : asset->Positions->Length;}

void ModelSubmeshResolver::ValidateSubmeshes(Array<::ModelSubmeshAsset*>* submeshes, int32_t elementCount)
{
    if (submeshes == nullptr)
    {
throw new ArgumentNullException("submeshes");
    }
    if (elementCount < 0)
    {
throw ([&]() {
auto __ctor_arg_000000FB = "elementCount";
auto __ctor_arg_000000FC = "Element count must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000000FB, __ctor_arg_000000FC);
})();
    }
for (int32_t submeshIndex = 0; submeshIndex < submeshes->Length; submeshIndex++) {
::ModelSubmeshAsset *submesh = (*submeshes)[submeshIndex];
    if (submesh == nullptr)
    {
throw new InvalidOperationException("Model submesh collections cannot contain null entries.");
    }
    if (submesh->get_IndexStart() < 0)
    {
throw new InvalidOperationException("Model submesh index starts must be zero or greater.");
    }
    if (submesh->get_IndexCount() <= 0)
    {
throw new InvalidOperationException("Model submesh index counts must be greater than zero.");
    }
    if (submesh->get_IndexStart() + submesh->get_IndexCount() > elementCount)
    {
throw new InvalidOperationException("Model submesh ranges cannot exceed the resolved model element count.");
    }
}
}

