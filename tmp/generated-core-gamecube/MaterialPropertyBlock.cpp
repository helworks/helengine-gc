#ifdef DrawText
#undef DrawText
#endif
#include "MaterialPropertyBlock.hpp"
#include "MaterialLayout.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeTexture.hpp"
#include "MaterialLayoutBinding.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::MaterialLayout* MaterialPropertyBlock::get_Layout()
{
return this->LayoutValue;
}

void MaterialPropertyBlock::CopyMatchingValuesFrom(::MaterialPropertyBlock* source)
{
    if (source == nullptr)
    {
throw new ArgumentNullException("source");
    }
this->CopyMatchingTextureValuesFrom(source);
this->CopyMatchingConstantBufferValuesFrom(source);
}

Array<uint8_t>* MaterialPropertyBlock::GetConstantBufferData(int32_t bindingIndex)
{
this->ValidateConstantBufferBindingIndex(bindingIndex);
Array<uint8_t> *data = (*this->ConstantBufferValues)[bindingIndex];
    if (data == nullptr)
    {
return nullptr;    }
Array<uint8_t> *copiedData = new Array<uint8_t>(data->Length);
Array<uint8_t>::Copy(data, copiedData, data->Length);
return copiedData;}

::RuntimeTexture* MaterialPropertyBlock::GetTexture(int32_t bindingIndex)
{
this->ValidateTextureBindingIndex(bindingIndex);
return (*this->TextureValues)[bindingIndex];}

MaterialPropertyBlock::MaterialPropertyBlock(::MaterialLayout* layout) : ConstantBufferValues(), LayoutValue(), TextureValues()
{
this->LayoutValue = (layout != nullptr ? layout : throw new ArgumentNullException("layout"));
this->TextureValues = new Array<RuntimeTexture*>(layout->get_TextureBindings()->Length);
this->ConstantBufferValues = new Array<Array<uint8_t>*>(layout->get_ConstantBufferBindings()->Length);
}

void MaterialPropertyBlock::SetConstantBufferData(std::string bindingName, Array<uint8_t>* data)
{
const int32_t bindingIndex = this->LayoutValue->FindConstantBufferBindingIndex(bindingName);
    if (bindingIndex < 0)
    {
throw new InvalidOperationException(std::string("Constant buffer binding '") + bindingName + std::string("' was not found on the material layout."));
    }
this->SetConstantBufferData(bindingIndex, data);
}

void MaterialPropertyBlock::SetConstantBufferData(int32_t bindingIndex, Array<uint8_t>* data)
{
this->ValidateConstantBufferBindingIndex(bindingIndex);
    if (data == nullptr)
    {
(*this->ConstantBufferValues)[bindingIndex] = nullptr;
return;    }
::MaterialLayoutBinding *binding = (*this->LayoutValue->get_ConstantBufferBindings())[bindingIndex];
    if (data->Length != binding->get_Size())
    {
throw new InvalidOperationException(std::string("Constant buffer binding '") + binding->get_Name() + std::string("' expects ") + std::to_string(binding->get_Size()) + std::string(" bytes but received ") + std::to_string(data->Length) + std::string("."));
    }
Array<uint8_t> *copiedData = new Array<uint8_t>(data->Length);
Array<uint8_t>::Copy(data, copiedData, data->Length);
(*this->ConstantBufferValues)[bindingIndex] = copiedData;
}

void MaterialPropertyBlock::SetTexture(std::string bindingName, ::RuntimeTexture* texture)
{
const int32_t bindingIndex = this->LayoutValue->FindTextureBindingIndex(bindingName);
    if (bindingIndex < 0)
    {
throw new InvalidOperationException(std::string("Texture binding '") + bindingName + std::string("' was not found on the material layout."));
    }
this->SetTexture(bindingIndex, texture);
}

void MaterialPropertyBlock::SetTexture(int32_t bindingIndex, ::RuntimeTexture* texture)
{
this->ValidateTextureBindingIndex(bindingIndex);
(*this->TextureValues)[bindingIndex] = texture;
}

bool MaterialPropertyBlock::TryGetFirstTexture(::RuntimeTexture*& texture)
{
for (int32_t bindingIndex = 0; bindingIndex < this->TextureValues->Length; bindingIndex++) {
::RuntimeTexture *value = (*this->TextureValues)[bindingIndex];
    if (value == nullptr)
    {
continue;
    }
texture = value;
return true;}
texture = nullptr;
return false;}

void MaterialPropertyBlock::CopyMatchingConstantBufferValuesFrom(::MaterialPropertyBlock* source)
{
for (int32_t constantBufferIndex = 0; constantBufferIndex < this->ConstantBufferValues->Length; constantBufferIndex++) {
::MaterialLayoutBinding *binding = (*this->LayoutValue->get_ConstantBufferBindings())[constantBufferIndex];
const int32_t sourceBindingIndex = source->get_Layout()->FindConstantBufferBindingIndex(binding->get_Name());
    if (sourceBindingIndex < 0)
    {
continue;
    }
::MaterialLayoutBinding *sourceBinding = (*source->get_Layout()->get_ConstantBufferBindings())[sourceBindingIndex];
    if (sourceBinding->get_Size() != binding->get_Size())
    {
continue;
    }
Array<uint8_t> *sourceData = source->GetConstantBufferData(sourceBindingIndex);
    if (sourceData == nullptr)
    {
continue;
    }
(*this->ConstantBufferValues)[constantBufferIndex] = sourceData;
}
}

void MaterialPropertyBlock::CopyMatchingTextureValuesFrom(::MaterialPropertyBlock* source)
{
for (int32_t textureIndex = 0; textureIndex < this->TextureValues->Length; textureIndex++) {
::MaterialLayoutBinding *binding = (*this->LayoutValue->get_TextureBindings())[textureIndex];
const int32_t sourceBindingIndex = source->get_Layout()->FindTextureBindingIndex(binding->get_Name());
    if (sourceBindingIndex < 0)
    {
continue;
    }
::RuntimeTexture *sourceTexture = source->GetTexture(sourceBindingIndex);
    if (sourceTexture == nullptr)
    {
continue;
    }
(*this->TextureValues)[textureIndex] = sourceTexture;
}
}

void MaterialPropertyBlock::ValidateConstantBufferBindingIndex(int32_t bindingIndex)
{
    if (bindingIndex < 0 || bindingIndex >= this->ConstantBufferValues->Length)
    {
throw ([&]() {
auto __ctor_arg_000000DF = "bindingIndex";
auto __ctor_arg_000000E0 = "Constant-buffer binding index is outside the material layout.";
return new ArgumentOutOfRangeException(__ctor_arg_000000DF, __ctor_arg_000000E0);
})();
    }
}

void MaterialPropertyBlock::ValidateTextureBindingIndex(int32_t bindingIndex)
{
    if (bindingIndex < 0 || bindingIndex >= this->TextureValues->Length)
    {
throw ([&]() {
auto __ctor_arg_000000E1 = "bindingIndex";
auto __ctor_arg_000000E2 = "Texture binding index is outside the material layout.";
return new ArgumentOutOfRangeException(__ctor_arg_000000E1, __ctor_arg_000000E2);
})();
    }
}

