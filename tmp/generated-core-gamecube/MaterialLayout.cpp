#ifdef DrawText
#undef DrawText
#endif
#include "MaterialLayout.hpp"
#include "MaterialLayout.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "MaterialRenderState.hpp"
#include "MaterialLayoutBinding.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::MaterialLayout* MaterialLayout::get_Empty()
{
return EmptyValue;
}

Array<::MaterialLayoutBinding*>* MaterialLayout::get_ConstantBufferBindings()
{
return this->ConstantBufferBindings;
}

std::string MaterialLayout::get_PixelProgram()
{
return this->PixelProgram;
}

::MaterialRenderState* MaterialLayout::get_RenderState()
{
return this->RenderState;
}

Array<::MaterialLayoutBinding*>* MaterialLayout::get_SamplerBindings()
{
return this->SamplerBindings;
}

std::string MaterialLayout::get_ShaderAssetId()
{
return this->ShaderAssetId;
}

Array<::MaterialLayoutBinding*>* MaterialLayout::get_TextureBindings()
{
return this->TextureBindings;
}

std::string MaterialLayout::get_Variant()
{
return this->Variant;
}

std::string MaterialLayout::get_VertexProgram()
{
return this->VertexProgram;
}

int32_t MaterialLayout::FindConstantBufferBindingIndex(std::string bindingName)
{
return this->FindBindingIndex(this->ConstantBufferBindings, bindingName);}

int32_t MaterialLayout::FindSamplerBindingIndex(std::string bindingName)
{
return this->FindBindingIndex(this->SamplerBindings, bindingName);}

int32_t MaterialLayout::FindTextureBindingIndex(std::string bindingName)
{
return this->FindBindingIndex(this->TextureBindings, bindingName);}

MaterialLayout::MaterialLayout(std::string shaderAssetId, std::string vertexProgram, std::string pixelProgram, std::string variant, ::MaterialRenderState* renderState, Array<::MaterialLayoutBinding*>* textureBindings, Array<::MaterialLayoutBinding*>* constantBufferBindings, Array<::MaterialLayoutBinding*>* samplerBindings) : ConstantBufferBindings(), PixelProgram(), RenderState(), SamplerBindings(), ShaderAssetId(), TextureBindings(), Variant(), VertexProgram()
{
this->ShaderAssetId = shaderAssetId;
this->VertexProgram = vertexProgram;
this->PixelProgram = pixelProgram;
this->Variant = variant;
this->RenderState = (renderState != nullptr ? renderState : throw new ArgumentNullException("renderState"));
this->TextureBindings = (textureBindings != nullptr ? textureBindings : throw new ArgumentNullException("textureBindings"));
this->ConstantBufferBindings = (constantBufferBindings != nullptr ? constantBufferBindings : throw new ArgumentNullException("constantBufferBindings"));
this->SamplerBindings = (samplerBindings != nullptr ? samplerBindings : throw new ArgumentNullException("samplerBindings"));
}

::MaterialLayout* MaterialLayout::EmptyValue = CreateEmptyValue();

::MaterialLayout* MaterialLayout::CreateEmptyValue()
{
return ([&]() {
auto __ctor_arg_000000C5 = String::Empty;
auto __ctor_arg_000000C6 = String::Empty;
auto __ctor_arg_000000C7 = String::Empty;
auto __ctor_arg_000000C8 = String::Empty;
auto __ctor_arg_000000C9 = new ::MaterialRenderState();
auto __ctor_arg_000000CA = Array<MaterialLayoutBinding*>::Empty();
auto __ctor_arg_000000CB = Array<MaterialLayoutBinding*>::Empty();
auto __ctor_arg_000000CC = Array<MaterialLayoutBinding*>::Empty();
return new ::MaterialLayout(__ctor_arg_000000C5, __ctor_arg_000000C6, __ctor_arg_000000C7, __ctor_arg_000000C8, __ctor_arg_000000C9, __ctor_arg_000000CA, __ctor_arg_000000CB, __ctor_arg_000000CC);
})();}

int32_t MaterialLayout::FindBindingIndex(Array<::MaterialLayoutBinding*>* bindings, std::string bindingName)
{
    if (bindings == nullptr)
    {
throw new ArgumentNullException("bindings");
    }
    if (String::IsNullOrWhiteSpace(bindingName))
    {
throw ([&]() {
auto __ctor_arg_000000CD = "Binding name must be provided.";
auto __ctor_arg_000000CE = "bindingName";
return new ArgumentException(__ctor_arg_000000CD, __ctor_arg_000000CE);
})();
    }
for (int32_t bindingIndex = 0; bindingIndex < bindings->Length; bindingIndex++) {
::MaterialLayoutBinding *binding = (*bindings)[bindingIndex];
    if (binding == nullptr)
    {
continue;
    }
    if (String::Equals(binding->get_Name(), bindingName, StringComparison::Ordinal))
    {
return bindingIndex;    }
}
return -1;}

