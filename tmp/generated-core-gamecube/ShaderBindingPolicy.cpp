#ifdef DrawText
#undef DrawText
#endif
#include "ShaderBindingPolicy.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderResourceType.hpp"
#include "runtime/native_exceptions.hpp"

int32_t ShaderBindingPolicy::get_ConstantBufferShift()
{
return this->ConstantBufferShift;
}

int32_t ShaderBindingPolicy::get_DefaultSpace()
{
return this->DefaultSpace;
}

int32_t ShaderBindingPolicy::get_SamplerShift()
{
return this->SamplerShift;
}

int32_t ShaderBindingPolicy::get_StorageShift()
{
return this->StorageShift;
}

int32_t ShaderBindingPolicy::get_TextureShift()
{
return this->TextureShift;
}

int32_t ShaderBindingPolicy::GetSlot(::ShaderResourceType type, int32_t registerIndex)
{
    if (registerIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_0000016A = "registerIndex";
auto __ctor_arg_0000016B = "Register index cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000016A, __ctor_arg_0000016B);
})();
    }
const int32_t shift = this->GetShift(type);
return shift + registerIndex;}

ShaderBindingPolicy::ShaderBindingPolicy(int32_t defaultSpace, int32_t constantBufferShift, int32_t textureShift, int32_t samplerShift, int32_t storageShift) : ConstantBufferShift(0), DefaultSpace(0), SamplerShift(0), StorageShift(0), TextureShift(0)
{
    if (defaultSpace < 0)
    {
throw ([&]() {
auto __ctor_arg_0000016C = "defaultSpace";
auto __ctor_arg_0000016D = "Default space cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000016C, __ctor_arg_0000016D);
})();
    }
    if (constantBufferShift < 0)
    {
throw ([&]() {
auto __ctor_arg_0000016E = "constantBufferShift";
auto __ctor_arg_0000016F = "Constant buffer shift cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000016E, __ctor_arg_0000016F);
})();
    }
    if (textureShift < 0)
    {
throw ([&]() {
auto __ctor_arg_00000170 = "textureShift";
auto __ctor_arg_00000171 = "Texture shift cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000170, __ctor_arg_00000171);
})();
    }
    if (samplerShift < 0)
    {
throw ([&]() {
auto __ctor_arg_00000172 = "samplerShift";
auto __ctor_arg_00000173 = "Sampler shift cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000172, __ctor_arg_00000173);
})();
    }
    if (storageShift < 0)
    {
throw ([&]() {
auto __ctor_arg_00000174 = "storageShift";
auto __ctor_arg_00000175 = "Storage shift cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000174, __ctor_arg_00000175);
})();
    }
this->DefaultSpace = defaultSpace;
this->ConstantBufferShift = constantBufferShift;
this->TextureShift = textureShift;
this->SamplerShift = samplerShift;
this->StorageShift = storageShift;
}

int32_t ShaderBindingPolicy::GetShift(::ShaderResourceType type)
{
switch (type) {
case ShaderResourceType::ConstantBuffer: {
return this->ConstantBufferShift;}
case ShaderResourceType::Texture2D:case ShaderResourceType::TextureCube: {
return this->TextureShift;}
case ShaderResourceType::Sampler: {
return this->SamplerShift;}
case ShaderResourceType::Buffer:case ShaderResourceType::StorageBuffer:case ShaderResourceType::StorageTexture2D: {
return this->StorageShift;}
default:  {
throw ([&]() {
auto __ctor_arg_00000176 = "type";
auto __ctor_arg_00000177 = "Unsupported resource type.";
return new ArgumentOutOfRangeException(__ctor_arg_00000176, __ctor_arg_00000177);
})();
}
}

}

